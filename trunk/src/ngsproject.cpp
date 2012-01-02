/*
 * ngsproject.cpp
 *
 *  Created on: Jan 2, 2012
 *      Author: lindenb
 */
#include <string>
#include <vector>
#include <map>
#include <set>
#include <memory>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#include "cgi.h"
#include "auto_free.h"
#include "segments.h"
#include "ttview.h"

using namespace std;


extern std::auto_ptr<std::vector<ChromStartEnd> > parseSegments(const char* s);
#define FOR_EACH_BEGIN(root,var) for (xmlNode* var = (root==0?0:root->children); var!=0; var = var->next) { if (var->type != XML_ELEMENT_NODE) continue;
#define FOR_EACH_END }

/** reference genome indexed with faidx */
class Reference
    {
    public:
	std::string id;
	std::string path;
	IndexedFasta* faidx;
    };


/** Indexed BAM file */
class IndexedBam
    {
    public:
	std::string id;
	std::string sample;
	std::string path;
	BamFile* bam;
    };
/** content of a project */
class Project
    {
    public:
	std::string id;
	std::string name;
	std::string description;
	Reference* reference;
	std::vector<IndexedBam*> bams;
    };

/** content of a project */
class ProjectList
    {
    public:
	ProjectList()
	    {
	    }
	~ProjectList()
	    {

	    }
    };

class NGSProject
    {
    public:
	CGI cgi;
	streambuf* stdbuf;;
	cgistreambuf* cgibuff;
	NGSProject():stdbuf(cout.rdbuf()),cgibuff(0)
	    {
	    cgibuff=new cgistreambuf(stdbuf);
	    cout.rdbuf(cgibuff);
	    }

	~NGSProject()
	    {
	    cout.flush();
	    cout.rdbuf(stdbuf);
	    delete cgibuff;
	    }

	void header()
	    {
	    cgibuff->setContentType("text/html");
	    cout << "<html><body>";
	    }

	void footer()
	    {
	    cout << "</body></html>";
	    }

	void quit(const char* mime,int status,const char* message)
	    {
	    cgibuff->setContentType(mime==0?"text/plain":mime);
	    cgibuff->setStatus(status==0?SC_BAD_REQUEST:status);
	    cout << (message==NULL?"error":message) << "\n";
	    cout.flush();
	    exit(EXIT_SUCCESS);
	    }

	xmlDocPtr load_project_file()
	    {
	    char* project_xml=getenv("NGS_PROJECT_PATH");
	    if(project_xml==NULL)
		{
		quit(0,0,"Cannot get $NGS_PROJECT_PATH.");
		}
	    xmlDocPtr doc=::xmlParseFile(project_xml);
	    if(doc==0)
		{
		quit(0,0,"Cannot load xml NGS_PROJECT_PATH.");
		}
	    return doc;
	    }


	auto_ptr<string> apply_stylesheet(
		char* xslt_string,
		unsigned long   xslt_len,
		const char ** params
		)
	    {
	    auto_ptr<string> result_as_string(0);
	    auto_free<xmlDoc> doc(load_project_file(),(auto_free<xmlDoc>::fun)xmlFreeDoc);
	    auto_free<xmlDoc> xsl(xmlParseMemory(xslt_string,xslt_len),(auto_free<xmlDoc>::fun)xmlFreeDoc);
	    if(xsl.get())
		{
		quit(0,SC_INTERNAL_SERVER_ERROR,"XML Error in XSLT file.");
		 return result_as_string;
		}
	    auto_free<xsltStylesheet> stylesheet(xsltParseStylesheetDoc(xsl.get()),(auto_free<xsltStylesheet>::fun)xsltFreeStylesheet);
	    if(stylesheet.get()==0)
		{
		quit(0,SC_INTERNAL_SERVER_ERROR,"XSL Error in XML/XSLT file.");
		 return result_as_string;
		}

	    auto_free<xmlDoc> res(::xsltApplyStylesheet(stylesheet.get(), doc.get(), params),(auto_free<xmlDoc>::fun)::xmlFreeDoc);
	    if(res.get()==0)
		{
		quit(0,SC_INTERNAL_SERVER_ERROR,"Cannot transform XML 2 HTML.");
		return result_as_string;
		}

	    xmlChar* doc_txt_ptr=NULL;
	    int doc_txt_length;
	    if(::xsltSaveResultToString(
		    &doc_txt_ptr,
		    &doc_txt_length,
		    res.get(),
		    stylesheet.get()
		    )!=0)
		    {
		    quit(0,SC_INTERNAL_SERVER_ERROR,"Cannot save to string.");
		    return result_as_string;
		    }
	    result_as_string.reset(new string((const char*)doc_txt_ptr,doc_txt_length));
	    ::xmlFree(doc_txt_ptr);

	    return result_as_string;
	    }


	void print_main()
	    {
	    const char ** params={NULL};
	    auto_ptr<string> html=apply_stylesheet("",1,params);
	    if(html.get()==0) quit(0,0,"Error");
	    }

	void print_project()
	    {
	    const char ** params={NULL};
	    auto_ptr<string> html=apply_stylesheet("",1,params);
	    if(html.get()==0) quit(0,0,"Error");
	    }

	void show_bam()
	    {
	    auto_ptr<Project> project(0);
	    const char* positions=cgi.getParameter("q");
	    if(positions==0)
		{
		quit(0,SC_BAD_REQUEST,"query missing");
		}
	    const char* project_id=cgi.getParameter("project.id");
	    if(project_id==0)
		{
		quit(0,SC_BAD_REQUEST,"project.id missing");
		}

	    std::auto_ptr<std::vector<ChromStartEnd> > segments;
	    try
		{
		segments=::parseSegments(positions);
		}
	    catch(exception& err)
		{
		quit(0,SC_BAD_REQUEST,err.what());
		}


	    auto_ptr<Reference> reference(0);
	    auto_free<xmlDoc> doc(load_project_file(),(auto_free<xmlDoc>::fun)xmlFreeDoc);

	    //loop over each project
	    FOR_EACH_BEGIN(::xmlDocGetRootElement(doc.get()),proj)
		if(strcmp((const char*)proj->name,"project")!=0) continue;
		auto_free<xmlChar> curr_id(::xmlGetProp(proj,BAD_CAST "id"),(auto_free<xmlChar>::fun)::xmlFree);
		if(curr_id.nil()) continue;
		if(::strcmp((const char*)curr_id.get(),project_id)==0)
		    {
		    project.reset(new Project);
		    project->id.assign(project_id);
		    FOR_EACH_BEGIN(proj,C1)
			if(strcmp((const char*)C1->name,"name")==0)
			    {
			    auto_free<xmlChar> xs(::xmlNodeGetContent(C1),(auto_free<xmlChar>::fun)::xmlFree);
			    if(!xs.nil()) project->name.assign((const char*)xs.get());
			    }
			else if(strcmp((const char*)C1->name,"description")==0)
			    {
			    auto_free<xmlChar> xs(::xmlNodeGetContent(C1),(auto_free<xmlChar>::fun)::xmlFree);
			    if(!xs.nil()) project->description.assign((const char*)xs.get());
			    }
			else if(strcmp((const char*)C1->name,"bam")==0)
			    {
			    auto_free<xmlChar> bam_ref(::xmlGetProp(proj,BAD_CAST "ref"),(auto_free<xmlChar>::fun)::xmlFree);
			    if(!bam_ref.nil())
				{
				IndexedBam* bam=new IndexedBam;
				bam->id.assign((const char*)bam_ref.get());
				project->bams.push_back(bam);
				}
			    }
			else if(strcmp((const char*)C1->name,"reference")==0)
			    {
			    auto_free<xmlChar> ref_ref(::xmlGetProp(proj,BAD_CAST "ref"),(auto_free<xmlChar>::fun)::xmlFree);
			    if(!ref_ref.nil())
				{
				reference.reset(new Reference);
				reference->id.assign((const char*)ref_ref.get());
				}
			    }
		    FOR_EACH_END
		    break;
		    }
	    FOR_EACH_END

	    if(project.get())
		{
		quit(0,SC_BAD_REQUEST,"Unknown project");
		}

	    //search for the BAM
	    FOR_EACH_BEGIN(::xmlDocGetRootElement(doc.get()),sam)
	    	if(strcmp((const char*)sam->name,"bam")!=0) continue;
		auto_free<xmlChar> curr_id(::xmlGetProp(sam,BAD_CAST "id"),(auto_free<xmlChar>::fun)::xmlFree);
	    	if(curr_id.nil()) continue;
	    	for(size_t i=0;i< project->bams.size();++i)
	    	    {
	    	    IndexedBam* newsam=project->bams[i];
	    	    if(newsam->id.compare((const char*)curr_id.get())!=0) continue;
		    FOR_EACH_BEGIN(sam,C1)
	    		if(strcmp((const char*)C1->name,"sample")==0)
			    {
			    auto_free<xmlChar> xs(xmlNodeGetContent(C1),(auto_free<xmlChar>::fun)::xmlFree);
			    if(!xs.nil()) newsam->sample.assign((const char*)xs.get());
			    }
	    		else if(strcmp((const char*)C1->name,"path")==0)
			    {
			    auto_free<xmlChar> xs(xmlNodeGetContent(C1),(auto_free<xmlChar>::fun)::xmlFree);
			    if(!xs.nil()) newsam->path.assign((const char*)xs.get());
			    }
		    FOR_EACH_END
	    	    }
	    FOR_EACH_END

	    //search for the reference
	    FOR_EACH_BEGIN(::xmlDocGetRootElement(doc.get()),ref)
	    	if(strcmp((const char*)ref->name,"reference")!=0) continue;
		auto_free<xmlChar> curr_id(::xmlGetProp(ref,BAD_CAST "id"),(auto_free<xmlChar>::fun)::xmlFree);
	    	if(curr_id.nil()) continue;
	    	if(reference.get()!=0 && reference->id.compare((const char*)curr_id.get())==0)
	    	    {
	    	    FOR_EACH_BEGIN(ref,C1)
			if(strcmp((const char*)C1->name,"path")==0)
			    {
			    auto_free<xmlChar> xs(::xmlNodeGetContent(C1),(auto_free<xmlChar>::fun)::xmlFree);
			    if(!xs.nil()) reference->path.assign((const char*)xs.get());
			    }
		    FOR_EACH_END
	    	    }
	    FOR_EACH_END

	    if(reference.get()==0)
		{
		quit(0,0,"Cannot find reference");
		}

	    reference->faidx=new IndexedFasta(reference->path.c_str());

	    header();
	    cout << "<div>";
	    for(vector<ChromStartEnd>::iterator r=segments->begin();r!=segments->end();++r)
		{
		cout << "<div>";
		cout << "<h3>" << (*r).chrom<<":"<< (*r).start << "</h3>";

		for(size_t i=0;i< project->bams.size();++i)
		    {
		    cout << "<h4>" <<  ""<< "</h4>";
		    cout << "<pre>";
		    TTView ttview;
		    ttview.print(cout,
			    r->chrom.c_str(),
			    r->start-1,
			    project->bams[i]->bam,
			    reference->faidx
			    );
		    cout << "</pre>";
		    }
		cout << "</div>";
		}
	    cout << "</div>";
	    footer();

	    }

	int main(int argc,char** argv)
	    {
	    try
		{
		if(!cgi.parse() || !cgi.contains("action") || cgi.contains("action","projects.list"))
		    {
		    print_main();
		    }
		else if(cgi.contains("action","project.show"))
		    {
		    print_project();
		    }
		else if(cgi.contains("action","bam.show"))
		    {
		    show_bam();
		    }
		else
		    {
		    print_main();
		    }
		}
	    catch(exception& err)
		{
		quit(0,0,err.what());
		}
	    return EXIT_SUCCESS;
	    }
    };


int main(int argc,char** argv)
    {
    NGSProject app;
    return app.main(argc,argv);
    }


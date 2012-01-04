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
#include "auto_vector.h"
#include "xmlescape.h"
#include "where.h"
using namespace std;


extern std::auto_ptr<std::vector<ChromStartEnd> > parseSegments(const char* s);
#define FOR_EACH_BEGIN(root,var) for (xmlNode* var = (root==0?0:root->children); var!=0; var = var->next) { if (var->type != XML_ELEMENT_NODE) continue;
#define FOR_EACH_END }


/** content of a project */
class Project
    {
    public:

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
		IndexedBam():bam(0)
		    {

		    }
		~IndexedBam()
		    {
		    if(bam!=0) delete bam;
		    }
		void open()
		    {
		    if(bam==0)
			{
			WHERE("open " << path);
			bam=new BamFile(path.c_str());						
			}
		    }
	    };


	std::string id;
	std::string name;
	std::string description;
	auto_ptr<Reference> reference;
	auto_vector<IndexedBam> bams;

	Project()
	    {

	    }


    };



class NGSProject
    {
    private:
#ifdef STANDALONE_VERSION
	std::string xml_path;
#endif
    public:
	CGI cgi;
	streambuf* stdbuf;;
	cgistreambuf* cgibuff;
	std::string title;
	NGSProject():stdbuf(cout.rdbuf()),cgibuff(0),title("NGS Project")
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
	    cgibuff->setContentType("application/xhtml+xml");
	    cout << "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\" \"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\">\n";
	    cout << "<html xmlns='http://www.w3.org/1999/xhtml' xml:lang='en'><head>"
		<< "<title>" << xmlEscape(title) << "</title>"
		<< "<style type='text/css'>"
		<< "</style>"
		<< "</head><body>";
	    }

	void footer()
	    {
	    cout << "<hr/><div style='font-size:50%;'>";
	    cout << "<a href='http://plindenbaum.blogspot.com'>Pierre Lindenbaum PhD</a><br/>";
	    cout << "Last compilation " << __DATE__ << " at " << __TIME__ << ".<br/>";
	    //cout << "<pre>"; cgi.dump(cout); cout << "</pre>";
	    cout << "</div></body></html>";
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
#ifdef STANDALONE_VERSION
	const char* project_xml=xml_path.c_str();
#else
	char* project_xml=getenv("NGS_PROJECT_PATH");
#endif

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
	    xmlDocPtr doc=load_project_file();
	    xmlDocPtr xsl=::xmlParseMemory(xslt_string,xslt_len);
	    if(xsl==0)
		{
		quit(0,SC_INTERNAL_SERVER_ERROR,"XML Error in XSLT file.");
		}

	    xsltStylesheetPtr stylesheet=::xsltParseStylesheetDoc(xsl);
	    if(stylesheet==0)
		{
		quit(0,SC_INTERNAL_SERVER_ERROR,"XSL Error in XML/XSLT file.");
		}

	    xmlDocPtr res=::xsltApplyStylesheet(stylesheet, doc, params);
	    if(res==0)
		{
		quit(0,SC_INTERNAL_SERVER_ERROR,"Cannot transform XML 2 HTML.");
		}
	    xmlChar* doc_txt_ptr=NULL;
	    int doc_txt_length;
	    if(::xsltSaveResultToString(
		    &doc_txt_ptr,
		    &doc_txt_length,
		    res,
		    stylesheet
		    )!=0)
		    {
		    quit(0,SC_INTERNAL_SERVER_ERROR,"Cannot save to string.");
		    return result_as_string;
		    }
	    result_as_string.reset(new string((const char*)doc_txt_ptr,doc_txt_length));
	    ::xmlFree(doc_txt_ptr);


	    ::xmlFreeDoc(res);
	    ::xsltFreeStylesheet(stylesheet);
	    //::xmlFreeDoc(xsl); //NON "the doc is automatically freed when the stylesheet is closed."
	    ::xmlFreeDoc(doc);
	    return result_as_string;
	    }


	void print_main()
	    {
	    const char * params[3]={"scriptname",NULL,NULL};
	    extern char* ngsproject2html;
	    extern unsigned long ngsproject2html_length;

	    std::string param2;
	    param2.append("\"");
#ifdef STANDALONE_VERSION
 	    param2.append("SCRIPT_NAME");
#else
	    param2.append(getenv("SCRIPT_NAME"));
#endif
	    param2.append("\"");
	    params[1]=param2.c_str();

	    auto_ptr<string> html=apply_stylesheet(ngsproject2html,ngsproject2html_length,params);
	    if(html.get()==0) quit(0,0,"Error");
	    header();
	    cout << *html;
            footer();
	    }

	void print_project()
	    {
	    title.assign("Project ID.");
	    if(!cgi.contains("project.id"))
		{
		quit(0,0,"Error in POST parameters");
		}
            //title.append(cgi.getParameter("project.id"));
	    const char *params[5]={"projectid",NULL,"scriptname",NULL,NULL};
	    std::string param;
	    param.append("\"");
	    param.append(cgi.getParameter("project.id"));
	    param.append("\"");
	    params[1]=param.c_str();

	    std::string param2;
	    param2.append("\"");
#ifdef STANDALONE_VERSION
 	    param2.append("SCRIPT_NAME");
#else
	    param2.append(getenv("SCRIPT_NAME"));
#endif
	    param2.append("\"");
	    params[3]=param2.c_str();

	    extern char* ngsproject2html;
	    extern unsigned long ngsproject2html_length;

	    auto_ptr<string> html=apply_stylesheet(ngsproject2html,ngsproject2html_length,params);
	   
	    if(html.get()==0) quit(0,0,"Error");
	    header();
            cout << *html;
            footer();
	    }

	void show_bam()
	    {
	    title.assign("Project ID.");
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
	    title.append(project_id);
	    std::auto_ptr<std::vector<ChromStartEnd> > segments;
	    try
		{
		segments=::parseSegments(positions);
		if(!segments->empty())
			{
			ostringstream os;
			os << segments->front();
			title.append(" Region:").append(os.str());
			}
		}
	    catch(exception& err)
		{
		quit(0,SC_BAD_REQUEST,err.what());
		}
	    


	    auto_free<xmlDoc> doc(load_project_file(),(auto_free<xmlDoc>::fun)xmlFreeDoc);

	    //loop over each project
	    FOR_EACH_BEGIN(::xmlDocGetRootElement(doc.get()),proj)
		if(strcmp((const char*)proj->name,"project")!=0) continue;
		WHERE("");
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
			    auto_free<xmlChar> bam_ref(::xmlGetProp(C1,BAD_CAST "ref"),(auto_free<xmlChar>::fun)::xmlFree);
			    if(!bam_ref.nil())
				{
				Project::IndexedBam* bam=new Project::IndexedBam;
				bam->id.assign((const char*)bam_ref.get());
				project->bams.push_back(bam);
				}
			    else
				{
				WHERE("missing bam ref");
				}
			    }
			else if(strcmp((const char*)C1->name,"reference")==0)
			    {
			    auto_free<xmlChar> ref_ref(::xmlGetProp(C1,BAD_CAST "ref"),(auto_free<xmlChar>::fun)::xmlFree);
			    if(!ref_ref.nil())
				{
				project->reference.reset(new Project::Reference);
				project->reference->id.assign((const char*)ref_ref.get());
				}
			    else
				{
				WHERE("no @ref in reference");
				}
			    }
		    FOR_EACH_END
		    break;
		    }
	    FOR_EACH_END

	    if(project.get()==0)
		{
		quit(0,SC_BAD_REQUEST,"Unknown project");
		}
	    if(project->bams.empty())
		{
		quit(0,0,"EMpty project");
		}
	    //search for the BAM
	    FOR_EACH_BEGIN(::xmlDocGetRootElement(doc.get()),sam)
	    	if(strcmp((const char*)sam->name,"bam")!=0) continue;
		auto_free<xmlChar> curr_id(::xmlGetProp(sam,BAD_CAST "id"),(auto_free<xmlChar>::fun)::xmlFree);
	    	if(curr_id.nil()) continue;
	    	for(size_t i=0;i< project->bams.size();++i)
	    	    {
	    	    Project::IndexedBam* newsam=project->bams[i];
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


	    if(project->reference.get()==0)
		{
		WHERE("");
		quit(0,0,"Cannot find reference");
		}

	    //search for the reference
	    FOR_EACH_BEGIN(::xmlDocGetRootElement(doc.get()),ref)
	    	if(strcmp((const char*)ref->name,"reference")!=0) continue;
		WHERE("");
		auto_free<xmlChar> curr_id(::xmlGetProp(ref,BAD_CAST "id"),(auto_free<xmlChar>::fun)::xmlFree);
	    	if(curr_id.nil()) continue;
	    	WHERE("");
	    	if(project->reference.get()!=0 && project->reference->id.compare((const char*)curr_id.get())==0)
	    	    {
	    	    WHERE("");
	    	    FOR_EACH_BEGIN(ref,C1)
			if(strcmp((const char*)C1->name,"path")==0)
			    {
			    auto_free<xmlChar> xs(::xmlNodeGetContent(C1),(auto_free<xmlChar>::fun)::xmlFree);
			    if(!xs.nil()) project->reference->path.assign((const char*)xs.get());
			    }
		    FOR_EACH_END
	    	    }
	    FOR_EACH_END



	    project->reference->faidx=new IndexedFasta(project->reference->path.c_str());

	    header();
	    cout << "<div>";
	    cout << "<h2>" << xmlEscape(project->name) << "</h2>";
	    cout << "<p>" << xmlEscape(project->description) << "</p>";
	    for(vector<ChromStartEnd>::iterator r=segments->begin();r!=segments->end();++r)
		{
		cout << "<div>";
		cout << "<h3>" << xmlEscape((*r).chrom)<<":"<< (*r).start << "</h3>";

		for(size_t i=0;i< project->bams.size();++i)
		    {
		    Project::IndexedBam* bam=project->bams[i];
		    bam->open();
		    cout << "<h4>" << xmlEscape(bam->sample) << " ID. " << xmlEscape(bam->id) << "</h4>";
		    cout << "<h5>" << xmlEscape(r->chrom) << ":" << r->start << "</h5>";
		    cout << "<pre style=\"font-size:14pt;color:white;background-color:black;max-width:100%;max-height:400px;overflow:auto;padding:20px;\">";
		    TTView ttview;
		    ttview.print(cout,
			    r->chrom.c_str(),
			    max(0,r->start-1),
			    bam->bam,
			    project->reference->faidx
			    );
		    cout << "</pre>";
		    }
		cout << "</div>";
		}
	    cout << "</div>";
	    footer();
	    }


#ifdef STANDALONE_VERSION
	void usage(std::ostream& out,int argc,char** argv)
	    {
	    out << argv[0] << " Pierre Lindenbaum PHD. 2012.\n";
	    out << "Options:\n";
	    out << "  -D <key> <value> . Add CGI parameter.\n";
	    out << "  -f <xml> path to project file.\n";
	    out << endl;
	    }
#endif
	int main(int argc,char** argv)
	    {
#ifdef STANDALONE_VERSION

	    int optind=1;

	    while(optind < argc)
		{
		if(std::strcmp(argv[optind],"-h")==0)
		    {
		    this->usage(cerr,argc,argv);
		    return (EXIT_FAILURE);
		    }
		else if(std::strcmp(argv[optind],"-D")==0 && optind+2<argc)
		    {
		    WHERE(argv[optind+1]<<"="<< argv[optind+2]);
		    cgi.setParameter(argv[optind+1],argv[optind+2]);
		    optind+=2;
		    }
		else if(std::strcmp(argv[optind],"-f")==0 && optind+1<argc)
		    {
		    xml_path.assign(argv[++optind]);
		    }
		else if(argv[optind][0]=='-')
		    {
		    cerr << "unknown option '"<< argv[optind]<<"'\n";
		    this->usage(cerr,argc,argv);
		    return (EXIT_FAILURE);
		    }
		else
		    {
		    break;
		    }
		++optind;
		}


	if(xml_path.empty())
	    {
	    cerr << "xml path undefined.\n";
	    return EXIT_FAILURE;
	    }
	if(argc!=optind)
	    {
	    cerr << "Illegal number of arguments.\n";
	    return EXIT_FAILURE;
	    }
	cgi.dump(cerr);
#endif

	    try
		{
		if(
#ifndef STANDALONE_VERSION
		   !cgi.parse() ||
#endif
		   !cgi.contains("action") ||
		   cgi.contains("action","projects.list"))
		    {
WHERE("");
		    print_main();
		    }
		else if(
			cgi.contains("action","project.show") &&
			cgi.contains("project.id")

			)
		    {
			WHERE("");
		    print_project();
		    }
		else if(cgi.contains("action","bam.show") &&
			cgi.contains("project.id") &&
			cgi.contains("q")
			)
		    {
		    WHERE("");
		    show_bam();
		    }
		else
		    {
			WHERE("");
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


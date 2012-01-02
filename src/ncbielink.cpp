/*
 * ncbiesearch.cpp
 *
 *  Created on: Oct 12, 2011
 *      Author: lindenb
 */
#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <set>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <curl/curl.h>
#include "netstreambuf.h"
#include "application.h"
#include "throw.h"
#include "zstreambuf.h"
#include "where.h"
#include "httpescape.h"
#include "numeric_cast.h"
using namespace std;

class NcbiELink:public AbstractApplication
    {
    public:
	string db_from;
	string db_to;
	netstreambuf readurl;
	int32_t column;

	NcbiELink():column(-1)
	    {
	    LIBXML_TEST_VERSION
	    }
	virtual ~NcbiELink()
	    {
	    ::xmlCleanupParser();
	    ::xmlMemoryDump();
	    }


	xmlDocPtr parse(const char* url)
	    {
	    netstreambuf net;
	    net.open(url);
	    std::string xml=net.content();
	    net.close();
	    return ::xmlReadMemory(xml.c_str(),xml.size(),url,0,0);
	    }

	void run(std::istream& in)
	    {
	    vector<string> tokens;
	    string line;

	    ostringstream baseos;
		baseos << "http://eutils.ncbi.nlm.nih.gov/entrez/eutils/elink.fcgi?dbfrom=";
		baseos << httpEscape(db_from) << "&db=" << httpEscape(db_to)<< "&retmode=xml"
	    	  << "&id=";
	    string baseurl(baseos.str());

	    while(getline(in,line,'\n'))
		{
		if(line.empty()) continue;
		if(line.at(0)=='#')
		    {
		    if(line.size()>1 && line[1]=='#') continue;
		    cout << line << tokenizer.delim
			    << db_from << ":"<< db_to<< ".linkName"
			    << tokenizer.delim
			    << db_from << ":"<< db_to<< ".id"
			    <<endl;
		    continue;
		    }



		bool found=false;
		tokenizer.split(line,tokens);
		if(column>= (int32_t)tokens.size())
		    {
		    cerr << "Not enough column in "<< line << endl;
		    continue;
		    }


		string url(baseurl);
		url.append(httpEscape(tokens[column]).str());

		xmlDocPtr doc=parse(url.c_str());
		if(doc!=NULL)
		    {
		    xmlNode *root_element = xmlDocGetRootElement(doc);
#define FOR_EACH(root,var) for (xmlNode* var = (root==0?0:root->children); var!=0; var = var->next)

		    FOR_EACH(root_element,LinkSet)
			{
		        if (LinkSet->type != XML_ELEMENT_NODE) continue;
		        FOR_EACH(LinkSet,LinkSetDb)
		            {
		            if (LinkSetDb->type != XML_ELEMENT_NODE) continue;
		            std::string linkName;
		            set<std::string> ids;
		            FOR_EACH(LinkSetDb,C1)
		        	{
		        	if (C1->type != XML_ELEMENT_NODE) continue;
		        	if(strcmp((const char*)C1->name,"LinkName")==0)
		        	    {
		        	    xmlChar *lnk=xmlNodeGetContent(C1);
				    if(lnk!=0 && strlen((const char*)lnk)>0)
					{
					linkName.assign((const char*)lnk);
					xmlFree(lnk);
					}
		        	    }
		        	else if(strcmp((const char*)C1->name,"Link")==0)
				    {
		        	    FOR_EACH(C1,Id)
					{
		        		if (Id->type != XML_ELEMENT_NODE) continue;
		        		xmlChar *s_id=xmlNodeGetContent(Id);
					if(s_id!=0 && strlen((const char*)s_id)>0)
					    {
					    ids.insert((const char*)s_id);
					    xmlFree(s_id);
					    }
					}
				    }
		        	}
		            for(set<string>::iterator r=ids.begin();r!=ids.end();++r)
		        	{
		        	found=true;
		        	cout << line << tokenizer.delim <<  linkName << tokenizer.delim << (*r) << endl;
		        	}
		            }
			}
#undef FOR_EACH
		    xmlFreeDoc(doc);
		    }


		if(!found)
			{
			cout << line << tokenizer.delim << "." << tokenizer.delim << "." << endl;
			}
		}
	    }
	virtual void usage(ostream& out,int argc,char** argv)
	    {
	    out << argv[0] << " Pierre Lindenbaum PHD. 2012.\n";
	    out << VARKIT_REVISION << endl;
	    out << "Retrieves related NCBI records using NCBI-ELink.\n";
	    out << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
	    out << "Usage" << endl
		    << "   "<< argv[0]<< " [options] (file|stdin)"<< endl;
	    out << "Options:\n";
	    out << "  -f <ncbi-database-from> REQUIRED." << endl;
	    out << "  -t <ncbi-database-to> REQUIRED." << endl;
	    out << "  -d <delimiter> (default:tab)" << endl;
	    out << "  -c (column) index of column containing gi identifier for database-from." << endl;
	    cerr << endl;
	    }


	virtual int main(int argc,char** argv)
	    {
	    int optind=1;
	    while(optind < argc)
		{
		if(std::strcmp(argv[optind],"-h")==0)
		    {
		    usage(cout,argc,argv);
		    return (EXIT_FAILURE);
		    }
		else if(std::strcmp(argv[optind],"-f")==0 && optind+1<argc)
		    {
		    this->db_from.assign(argv[++optind]);
		    }
		else if(std::strcmp(argv[optind],"-t")==0 && optind+1<argc)
		    {
		    this->db_to.assign(argv[++optind]);
		    }
		else if(std::strcmp(argv[optind],"-c")==0 && optind+1<argc)
		    {
		    if(!numeric_cast<int32_t>(argv[++optind],&column) || column<1)
			{
			cerr << "Bad column for option -c " << argv[optind] << endl;
			usage(cerr,argc,argv);
			return EXIT_FAILURE;
			}
		    this->column--;
		    }
		else if(std::strcmp(argv[optind],"-d")==0 && optind+1< argc)
		    {
		    char* p=argv[++optind];
		    if(strlen(p)!=1)
			{
			cerr << "Bad delimiter \""<< p << "\"\n";
			usage(cerr,argc,argv);
			return(EXIT_FAILURE);
			}
		    this->tokenizer.delim=p[0];
		    }
		else if(argv[optind][0]=='-')
		    {
		    cerr << "unknown option '"<< argv[optind]<< "'"<< endl;
		    usage(cerr,argc,argv);
		    return (EXIT_FAILURE);
		    }
		else
		    {
		    break;
		    }
		++optind;
		}
	    if(db_from.empty())
		{
		cerr << "Undefined db-from"<< endl;
		usage(cerr,argc,argv);
		return (EXIT_FAILURE);
		}
	    if(db_to.empty())
		{
		cerr << "Undefined db-to"<< endl;
		usage(cerr,argc,argv);
		return (EXIT_FAILURE);
		}
	    if(column<0)
		{
		cerr << "Undefined column"<< endl;
		usage(cerr,argc,argv);
		return (EXIT_FAILURE);
		}

	    if(optind==argc)
		{
		igzstreambuf buf;
		istream in(&buf);
		run(in);
		}
	    else
		{
		while(optind< argc)
		    {
		    char* filename=argv[optind++];
		    igzstreambuf buf(filename);
		    istream in(&buf);
		    run(in);
		    buf.close();
		    }
		}
	    return EXIT_SUCCESS;
	    }
    };

int main(int argc,char** argv)
    {
    NcbiELink app;
    return app.main(argc,argv);
    }

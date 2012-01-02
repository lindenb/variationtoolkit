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

class NcbiCited:public AbstractApplication
    {
    public:
	bool show_pmids;
	netstreambuf readurl;
	int32_t column;

	NcbiCited():show_pmids(false),column(-1)
	    {
	    LIBXML_TEST_VERSION
	    }
	virtual ~NcbiCited()
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
		baseos << "http://eutils.ncbi.nlm.nih.gov/entrez/eutils/elink.fcgi?dbfrom=pubmed"
			<< "&linkname=pubmed_pubmed_citedin"
			<< "&tool="<< httpEscape(__FILE__)
			<< "&cmd=neighbor"
		        << "&retmode=xml"
	    	        << "&id=";
	    string baseurl(baseos.str());

	    while(getline(in,line,'\n'))
		{
		if(line.empty()) continue;
		if(line.at(0)=='#')
		    {
		    if(line.size()>1 && line[1]=='#') continue;
		    cout << line << tokenizer.delim << "count.citing";
		    if(show_pmids)
			{
			cout << tokenizer.delim << "citing.pmid";
			}
		    cout <<endl;
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
		            set<std::string> ids;
		            FOR_EACH(LinkSetDb,C1)
		        	{
		        	if (C1->type != XML_ELEMENT_NODE) continue;
		        	if(strcmp((const char*)C1->name,"Link")==0)
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
		            if(!ids.empty())
		        	{
				if(show_pmids )
				    {
				    for(set<string>::iterator r=ids.begin();r!=ids.end();++r)
					{
					found=true;
					cout << line << tokenizer.delim <<  ids.size() << tokenizer.delim << (*r) << endl;
					}
				    }
				else
				    {
				    found=true;
				    cout << line << tokenizer.delim <<  ids.size()  << endl;
				    }
		        	}
		            }
			}
#undef FOR_EACH
		    xmlFreeDoc(doc);
		    }


		if(!found)
			{

			cout << line << tokenizer.delim << ".";
			if(show_pmids) cout << tokenizer.delim << ".";
			cout << endl;
			}
		}
	    }
	virtual void usage(ostream& out,int argc,char** argv)
	    {
	    out << argv[0] << " Pierre Lindenbaum PHD. 2012.\n";
	    out << VARKIT_REVISION << endl;
	    out << "Retrieves  NCBI/Pubmed records citing a pmid using NCBI-ELink.\n";
	    out << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
	    out << "Usage" << endl
		    << "   "<< argv[0]<< " [options] (file|stdin)"<< endl;
	    out << "Options:\n";
	    out << "  -p show pmids." << endl;
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
		else if(std::strcmp(argv[optind],"-p")==0 )
		    {
		    this->show_pmids=true;
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
    NcbiCited app;
    return app.main(argc,argv);
    }

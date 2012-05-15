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
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#include <curl/curl.h>
#include "netstreambuf.h"
#include "tokenizer.h"
#include "throw.h"
#include "zstreambuf.h"
#include "where.h"
#include "httpescape.h"
using namespace std;

#define XSDLODC(XSLT) virtual std::string xsldoc()\
	{\
	extern char* XSLT;\
	string s=string(XSLT);\
	return s;\
	}

class NcbiESummary
    {
    private:
	xmlDocPtr doc;
	xsltStylesheetPtr stylesheet;

    public:
	Tokenizer tokenizer;
	int column;
	char* database;


	NcbiESummary():doc(0),stylesheet(0),column(-1),database(0)
	    {
	    LIBXML_TEST_VERSION
	    //TODO
	    extern char* summary2json;
	    extern unsigned long summary2json_length;
	    doc=xmlParseMemory(summary2json,summary2json_length);
	    if(doc==NULL) THROW("Cannot parse XSLT/XML stylesheet");
	    stylesheet=xsltParseStylesheetDoc(doc);
	    if(stylesheet==NULL) THROW("Cannot parse XSLT stylesheet");
	    }
	~NcbiESummary()
	    {
	    if(stylesheet!=NULL) ::xsltFreeStylesheet(stylesheet);
	    ::xmlCleanupParser();
	    ::xmlMemoryDump();
	    }


	xmlDocPtr fetch(const char* url)
	    {
	    netstreambuf readurl;
	    readurl.open(url);
	    string xml=readurl.content();
	    xmlDocPtr res=xmlParseMemory(xml.c_str(),xml.size());
	    if(res==NULL) THROW("Cannot parse URL:"<<url);
	    return res;
	    }

	void run(std::istream& in)
	    {
	    vector<string> tokens;
	    vector<string> header;
	    string line;
	    const char ** params={NULL};

	    ostringstream baseos;
		baseos << "http://eutils.ncbi.nlm.nih.gov/entrez/eutils/esummary.fcgi?db=";
		baseos << httpEscape(database) << "&id=";
		string urlbase(baseos.str());

	    while(getline(in,line,'\n'))
			{
			if(line.empty()) continue;
			if(line.at(0)=='#')
				{
				if(line.size()>1 && line[1]=='#') continue;
				cout << line ;
					cout << tokenizer.delim << "esummary";
				cout <<endl;
				continue;
				}

			tokenizer.split(line,tokens);
			if(column>= (int)tokens.size())
				{
				cerr << "column out of range in " << line << endl;
				continue;
				}
			char* p2;
			long gi=strtol(tokens[column].c_str(),&p2,10);
			if(gi<0 || *p2!=0)
				{
				cerr << "bad GI " << tokens[column] << " in " << line << endl;
				continue;
				}

			ostringstream os;
			os << urlbase << gi;
			string url(os.str());

			xmlDocPtr doc=fetch(url.c_str());
			xmlDocPtr res = xsltApplyStylesheet(this->stylesheet, doc, params);
			if(res==NULL) THROW("Cannot apply stylesheet");
			xmlChar* doc_txt_ptr=NULL;
			int doc_txt_len;
			if(::xsltSaveResultToString(
				&doc_txt_ptr,
				&doc_txt_len,
				res,
				this->stylesheet
				)!=0)
				{
				THROW("Cannot save to string");
				}
			cout << line << tokenizer.delim;
			cout.write((const char*)doc_txt_ptr,doc_txt_len);
			cout << endl;

			::xmlFree(doc_txt_ptr);
			::xmlFreeDoc(res);
			::xmlFreeDoc(doc);
			}
	    }

	void usage(int argc,char** argv)
	    {
	    cerr << argv[0] << " Pierre Lindenbaum PHD. 2011.\n";
	    cerr << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
	    cerr << "Usage" << endl
		    << "   "<< argv[0]<< " [options] -c column (file|stdin)"<< endl;
	    cerr << "Options:\n";
	    cerr << "  -D <database> required" << endl;
	    cerr << "  -d <delimiter> (default:tab)" << endl;
	    cerr << "  -c <column=int> " << endl;
	    cerr << endl;
	    }
    };


int main(int argc,char** argv)
    {
    NcbiESummary app;
    int optind=1;
    while(optind < argc)
		{
		if(std::strcmp(argv[optind],"-h")==0)
			{
			app.usage(argc,argv);
			return (EXIT_FAILURE);
			}
		else if(std::strcmp(argv[optind],"-D")==0 && optind+1<argc)
			{
			app.database=argv[++optind];
			}
		else if(std::strcmp(argv[optind],"-c")==0 && optind+1<argc)
			{
			char* p2;
			app.column=(int)strtol(argv[++optind],&p2,10);
			if(*p2!=0 || app.column<1)
				{
				cerr << "Illegal number for column" << endl;
				return EXIT_FAILURE;
				}
			app.column--;
			}
		else if(std::strcmp(argv[optind],"-d")==0 && optind+1< argc)
			{
			char* p=argv[++optind];
			if(strlen(p)!=1)
				{
				cerr << "Bad delimiter \""<< p << "\"\n";
				app.usage(argc,argv);
				return(EXIT_FAILURE);
				}
			app.tokenizer.delim=p[0];
			}
		else if(argv[optind][0]=='-')
			{
			cerr << "unknown option '"<< argv[optind]<< "'"<< endl;
			app.usage(argc,argv);
			return (EXIT_FAILURE);
			}
		else
			{
			break;
			}
		++optind;
		}


    if(app.database==NULL)
    	{
    	cerr << "No database defined"<< endl;
    	app.usage(argc,argv);
    	return (EXIT_FAILURE);
    	}

    if(app.column==-1)
    	{
    	cerr << "Column undefined"<< endl;
		app.usage(argc,argv);
		return (EXIT_FAILURE);
    	}

    if(optind==argc)
    	    {
    	    igzstreambuf buf;
    	    istream in(&buf);
    	    app.run(in);
    	    }
        else
    	    {
    	    while(optind< argc)
				{
				char* filename=argv[optind++];
				igzstreambuf buf(filename);
				istream in(&buf);
				app.run(in);
				buf.close();
				}
    	    }
    return EXIT_SUCCESS;
    }

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
#include <libxml/xmlreader.h>
#include <curl/curl.h>
#include "netstreambuf.h"
#include "tokenizer.h"
#include "throw.h"
#include "zstreambuf.h"
#include "where.h"
#include "httpescape.h"
using namespace std;

class NcbiEsearch
    {
    public:
	Tokenizer tokenizer;
	int limit;
	string database;
	string query;
	netstreambuf readurl;

	NcbiEsearch():limit(10),database("pubmed")
	    {
	    LIBXML_TEST_VERSION
	    }
	~NcbiEsearch()
	    {

	    ::xmlCleanupParser();
	    ::xmlMemoryDump();
	    }

	static int _xmlInputReadCallback(
		 void * context,
		 char * buffer,
		 int len)
	    {
	    return (int)((NcbiEsearch*)context)->readurl.read(buffer,len);
	    }
	static int _xmlInputCloseCallback(void * context)
	    {
	    ((NcbiEsearch*)context)->readurl.close();
	    return 0;
	    }

	xmlTextReaderPtr parse(const char* url)
	    {
	    int options=0;
	    readurl.open(url);
	    xmlTextReaderPtr reader=::xmlReaderForIO(
		 NcbiEsearch::_xmlInputReadCallback,
		 NcbiEsearch::_xmlInputCloseCallback,
		 this,
		 url,
		 "UTF-8",
		 options);
	    if(reader==NULL)
		{
		readurl.close();
		THROW("Cannot open "<< url);
		}
	    return reader;
	    }
	void run(std::istream& in)
	    {
	    vector<string> tokens;
	    string line;

	    ostringstream baseos;
		baseos << "http://eutils.ncbi.nlm.nih.gov/entrez/eutils/esearch.fcgi?db=";
		baseos << httpEscape(database) << "&retmode=xml&retmax="<< limit
	    	  << "&term=";
	    string baseurl(baseos.str());

	    while(getline(in,line,'\n'))
			{
			if(line.empty()) continue;
			if(line.at(0)=='#')
				{
				if(line.size()>1 && line[1]=='#') continue;
				cout << line << tokenizer.delim << database<< ".id"<<endl;
				continue;
				}
			bool found=false;
			tokenizer.split(line,tokens);
			ostringstream os;
			bool ok=true;
			char* p=(char*)query.c_str();
			char* prev=p;
			while((p=strchr(prev,'$'))!=NULL)
				{
				if(!isdigit(*(p+1)))
					{
					os.write(prev,p-prev);
					prev=p+1;
					continue;
					}
				os.write(prev,p-prev);
				++p;
				char *p2;
				int col=(int)strtol(p,&p2,10);
				if(col<1)
					{
					THROW("Bad query near \""<< p<< "\"");
					}
				col--;//0-based
				if(col>=(int)tokens.size())
					{
					ok=false;
					cerr << "Index out of bound for "<< query << " and "<< line << endl;
					break;
					}
				os << tokens[col];
				prev=p2;
				}
			os << prev;
			if(!ok) continue;
			string url(baseurl);
			bool phraseNotFound=false;
			url.append(httpEscape(os.str()).str());
			vector<xmlChar*> identifiers;
			xmlTextReaderPtr reader=parse(url.c_str());
			for(;;)
				{
				int ret = ::xmlTextReaderRead(reader);
				if(ret==-1) THROW("I/O XML error");
				if(ret!=1) break;
				switch(xmlTextReaderNodeType(reader))
					{
					case XML_READER_TYPE_ELEMENT:
						{
						const xmlChar* tag=xmlTextReaderConstName(reader);
						if(tag!=NULL &&
							!::xmlTextReaderIsEmptyElement(reader) &&
							::xmlStrEqual(tag,BAD_CAST"Id")
							)
							{
							xmlChar *content=xmlTextReaderReadString(reader);
							identifiers.push_back(content);
							found=true;
							}
						else  if(tag!=NULL &&
							(
							::xmlStrEqual(tag,BAD_CAST"PhraseNotFound") ||
							::xmlStrEqual(tag,BAD_CAST"FieldNotFound") ||
							::xmlStrEqual(tag,BAD_CAST"PhraseIgnored") ||
							::xmlStrEqual(tag,BAD_CAST"QuotedPhraseNotFound") 
							)
							)
							{
							phraseNotFound=true;
							found=false;
							}
						break;
						}
					default:
						{
						break;
						}
					}
				}
			for(size_t i=0;i< identifiers.size();++i)
				{
				xmlChar *content=identifiers[i];
				if(!phraseNotFound)
					{
					cout << line << tokenizer.delim << content << endl;//xmlTextReaderConstValue(reader)<< endl;
					}
				xmlFree(content);
				}
			
			::xmlFreeTextReader(reader);
			if(!found || phraseNotFound)
				{
				cout << line << tokenizer.delim << "!N/A" << endl;
				}
			}
	    }
	void usage(int argc,char** argv)
	    {
	    cerr << argv[0] << " Pierre Lindenbaum PHD. 2011.\n";
	    cerr << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
	    cerr << "Usage" << endl
		    << "   "<< argv[0]<< " [options] -q query (file|stdin)"<< endl;
	    cerr << "Options:\n";
	    cerr << "  -D <database> (default "<<  database << ")" << endl;
	    cerr << "  -q <query> [required]" << endl;
	    cerr << "  -d <delimiter> (default:tab)" << endl;
	    cerr << "  -L <limit=int> (default:"<< limit<<")" << endl;
	    cerr << endl;
	    }
    };


int main(int argc,char** argv)
    {
    NcbiEsearch app;
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
	else if(std::strcmp(argv[optind],"-q")==0 && optind+1<argc)
	    {
	    app.query=argv[++optind];
	    }
	else if(std::strcmp(argv[optind],"-L")==0 && optind+1<argc)
	    {
	    char* p2;
	    app.limit=(int)strtol(argv[++optind],&p2,10);
	    if(*p2!=0 || app.limit<1)
			{
			cerr << "Illegal number for limit" << endl;
			return EXIT_FAILURE;
			}
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
    if(app.query.empty())
	{
	cerr << "No query defined"<< endl;
	app.usage(argc,argv);
	return (EXIT_FAILURE);
	}
    if(app.database.empty())
    	{
    	cerr << "No database defined"<< endl;
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

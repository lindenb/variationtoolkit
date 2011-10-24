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

class NcbiEFetch
    {
    public:

	class DatabaseHandler
		{
		private:
			xmlDocPtr doc;
			xsltStylesheetPtr stylesheet;
		public:
			DatabaseHandler():doc(NULL),stylesheet(NULL) {}
			virtual ~DatabaseHandler()
				{
				if(stylesheet!=NULL) ::xsltFreeStylesheet(stylesheet);
				//NO ??!! (exception) if(doc!=NULL) ::xmlFreeDoc(doc);
				}
			virtual std::string urlparams()
				{
				return "";
				}
			virtual void fillHeader(vector<string>& header)=0;
			virtual std::string xsldoc()=0;
			virtual std::string database()=0;
			virtual xsltStylesheetPtr sheet()
				{
				if(stylesheet==NULL)
					{
					std::string xsl=xsldoc();
					doc=xmlParseMemory(xsl.c_str(),xsl.size());
					if(doc==NULL) THROW("Cannot parse XSLT/XML stylesheet");
					stylesheet=xsltParseStylesheetDoc(doc);
					if(stylesheet==NULL) THROW("Cannot parse XSLT stylesheet");
					}
				return stylesheet;
				}
		};
	class PubmedHandler:public DatabaseHandler
			{
			public:
				PubmedHandler() {}
				virtual ~PubmedHandler() {}
				XSDLODC(pubmedxsl)
				virtual std::string database()
					{
					return string("pubmed");
					}
				virtual void fillHeader(vector<string>& header)
					{
					header.clear();
					header.push_back("pubmed.year");
					header.push_back("pubmed.title");
					header.push_back("pubmed.journal");
					header.push_back("pubmed.abstract");
					}
			};

	class SeqHandler:public DatabaseHandler
				{
				public:
					SeqHandler() {}
					virtual ~SeqHandler() {}
					XSDLODC(nuccorexsl)
					virtual std::string urlparams()
						{
						return "&rettype=fasta";
						}
					virtual std::string database()=0;
					virtual void fillHeader(vector<string>& header)
						{
						header.clear();
						header.push_back(database()+".type");
						header.push_back(database()+".accver");
						header.push_back(database()+".taxid");
						header.push_back(database()+".orgname");
						header.push_back(database()+".defline");
						header.push_back(database()+".length");
						header.push_back(database()+".sequence");
						}
				};
		class ProteinHandler:public SeqHandler
				{
				public:
					ProteinHandler() {}
					virtual ~ProteinHandler() {}
					virtual std::string database()
						{
						return string("protein");
						}
				};
		class NucleotideHandler:public SeqHandler
				{
				public:
					NucleotideHandler() {}
					virtual ~NucleotideHandler() {}
					virtual std::string database()
						{
						return string("nucleotide");
						}
				};
		class DbSNPHandler:public DatabaseHandler
			{
			public:
				DbSNPHandler() {}
				virtual ~DbSNPHandler() {}
				XSDLODC(dbsnpxsl)
				virtual std::string database()
					{
					return string("snp");
					}
				virtual void fillHeader(vector<string>& header)
					{
					header.clear();
					header.push_back("snp.het");
					header.push_back("snp.bitField");
					header.push_back("snp.seq5");
					header.push_back("snp.obs");
					header.push_back("snp.seq3");
					header.push_back("snp.map");
					}
			};

		class GeneHandler:public DatabaseHandler
				{
				public:
					GeneHandler() {}
					virtual ~GeneHandler() {}
					XSDLODC(genexsl)
					virtual std::string database()
						{
						return string("gene");
						}
					virtual void fillHeader(vector<string>& header)
						{
						header.clear();
						header.push_back("gene.locus");
						header.push_back("gene.desc");
						header.push_back("gene.maploc");
						header.push_back("gene.ids");
						header.push_back("gene.summary");
						}
				};

		class TaxonomyHandler:public DatabaseHandler
			{
			public:
				TaxonomyHandler() {}
				virtual ~TaxonomyHandler() {}
				XSDLODC(taxonomyxsl)
				virtual std::string database()
					{
					return string("taxonomy");
					}
				virtual void fillHeader(vector<string>& header)
					{
					header.clear();
					header.push_back("taxon.name");
					header.push_back("taxon.lineage");
					}
			};


	Tokenizer tokenizer;
	int column;
	vector<DatabaseHandler*> handlers;
	DatabaseHandler* handler;


	NcbiEFetch():column(-1)
	    {
	    LIBXML_TEST_VERSION
	    handlers.push_back(new PubmedHandler);
	    handler=handlers.back();
	    handlers.push_back(new NucleotideHandler);
	    handlers.push_back(new ProteinHandler);
	    handlers.push_back(new DbSNPHandler);
	    handlers.push_back(new GeneHandler);
	    handlers.push_back(new TaxonomyHandler);
	    }
	~NcbiEFetch()
	    {
		while(!handlers.empty())
			{
			delete handlers.back();
			handlers.pop_back();
			}
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
		const char ** params={NULL};
	    vector<string> tokens;
	    vector<string> header;
	    string line;

	    handler->fillHeader(header);

	    ostringstream baseos;
		baseos << "http://eutils.ncbi.nlm.nih.gov/entrez/eutils/efetch.fcgi?db=";
		baseos << httpEscape(handler->database()) <<
				handler->urlparams()<< "&retmode=xml&id=";
		string urlbase(baseos.str());

	    while(getline(in,line,'\n'))
			{
			if(line.empty()) continue;
			if(line.at(0)=='#')
				{
				if(line.size()>1 && line[1]=='#') continue;
				cout << line ;
				for(size_t i=0;i< header.size();++i)
					{
					cout << tokenizer.delim << header[i];
					}
				cout <<endl;
				continue;
				}

			tokenizer.split(line,tokens);
			if(column>= (int)tokens.size())
				{
				cerr << "column out of range in " << line << endl;
				continue;
				}
			if(tokens[column].length()>2 &&
					handler->database().compare("snp")==0 &&
					tolower(tokens[column][0])=='r' &&
					tolower(tokens[column][1])=='s'
					)
					{
					tokens[column]=tokens[column].substr(2);
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
			xmlDocPtr res = xsltApplyStylesheet(handler->sheet(), doc, params);
			if(res==NULL) THROW("Cannot apply stylesheet");
			xmlChar* doc_txt_ptr=NULL;
			int doc_txt_len;
			if(::xsltSaveResultToString(
				&doc_txt_ptr,
				&doc_txt_len,
				res,
				handler->sheet()
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
	    cerr << "  -D <database> (default "<<  handler->database() << ")" << endl;
	    cerr << "  -d <delimiter> (default:tab)" << endl;
	    cerr << "  -c <column=int> " << endl;
	    cerr << endl;
	    }
    };


int main(int argc,char** argv)
    {
	NcbiEFetch app;
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
			char* db=argv[++optind];
			app.handler=NULL;
			for(size_t i=0;i< app.handlers.size();++i)
				{
				if(app.handlers.at(i)->database().compare(db)==0)
					{
					app.handler=app.handlers.at(i);
					break;
					}
				}
			if(app.handler==NULL)
				{
				THROW("unsupported database :"<< db);
				}
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


    if(app.handler==NULL)
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

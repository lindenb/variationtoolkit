/**
 * Author:
 *	Pierre Lindenbaum PhD
 * Contact:
 *	plindenbaum@yahoo.fr
 * Date:
 *	Oct 2011
 * WWW:
 *	http://plindenbaum.blogspot.com
 * Motivation:
 *
 */
#include <cstdlib>
#include <vector>
#include <map>
#include <set>
#include <cerrno>
#include <string>
#include <cstring>
#include <stdexcept>
#include <climits>
#include <cmath>
#include <cfloat>
#include <cstdio>
#include <iostream>
#include <zlib.h>
#include <sstream>
#include <algorithm>
#include <cassert>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <stdint.h>
#include "application.h"
#include "zstreambuf.h"
#include "tokenizer.h"
#include "smartcmp.h"
#include "netstreambuf.h"

using namespace std;


typedef int column_t;





class UniProt:public AbstractApplication
    {
    public:
		int32_t column_aa_pos;
		int32_t column_spId;
		xmlDocPtr record;
		string current_recordid;
		UniProt():column_aa_pos(-1),column_spId(-1),record(NULL)
			{
			 LIBXML_TEST_VERSION
			xmlInitParser();
			}

		virtual ~UniProt()
			{
			if(record!=NULL) ::xmlFreeDoc(record);
			record=NULL;
			xmlCleanupParser();
			xmlMemoryDump();
			}


	static xmlNodePtr first(xmlNodePtr root,const char* name)
		{
		if(root==NULL) return NULL;
		for(xmlNodePtr n = root->children; n!=NULL; n = n->next)
			{
			if (n->type != XML_ELEMENT_NODE) continue;
			if(::xmlStrEqual(n->name,BAD_CAST name))
				{
				return n;
				}
			}
		return NULL;
		}

	static int parseAttInt(xmlNodePtr root,const char* att)
		{
		xmlChar* s=::xmlGetProp(root,BAD_CAST att);
		if(s==NULL) return -1;
		char* p2;
		int n=(int)strtol((const char*)s,&p2,10);
		if(*p2!=0) return -1;
		xmlFree(s);
		return n;
		}
	static string parseAttStr(xmlNodePtr root,const char* att)
		{
		string s(".");
		xmlChar* x=::xmlGetProp(root,BAD_CAST att);
		if(x==NULL) return s;
		s.assign((const char*)x);
		xmlFree(x);
		return s;
		}

	void run(std::istream& in)
	    {
	    vector<string> tokens;
	    string line;


	    while(getline(in,line,'\n'))
		{
		if(AbstractApplication::stopping()) break;
		if(line.empty()) continue;
		if(line[0]=='#')
		    {
		    if(line.size()>1 && line[1]=='#')
			{
			cout << line << endl;
			continue;
			}
		    cout << line << tokenizer.delim
			    << "uniprot.beg"  << tokenizer.delim
			    << "uniprot.end"  << tokenizer.delim
			    << "uniprot.type"  << tokenizer.delim
			    << "uniprot.status"  << tokenizer.delim
			    << "uniprot.desc"  << tokenizer.delim
			    << "uniprot.evidence"  << tokenizer.delim
			    << "uniprot.ref"
			    << endl;
		    continue;
		    }

		tokenizer.split(line,tokens);
		if(column_aa_pos>=(int)tokens.size())
		    {
		    cerr << "Out of range for COLUMN_AA in " << line << endl;
		    continue;
		    }
		if(column_spId>=(int)tokens.size())
		    {
		    cerr << "Out of range for SWISSPROT-ID in " << line << endl;
		    continue;
		    }


		string swissprotId=tokens[column_spId];
		if(swissprotId.empty() || swissprotId.compare(".")==0)
		    {
		    cout << line;
		    for(int i=0;i< 7;++i) cout << tokenizer.delim << ".";
		    cout << endl;
		    continue;
		    }
		char* p2;
		int posAA= (int)strtol(tokens[column_aa_pos].c_str(),&p2,10);
		if(*p2!=0 || posAA<1)
		    {
		    cerr << "Bad Column-aa in " << line << endl;
		    continue;
		    }
		if(!(current_recordid.compare(swissprotId)==0 && record!=NULL))
		    {
		    if(record!=NULL) ::xmlFreeDoc(record);
		    record=NULL;

		    ostringstream urlos;
		    urlos << "http://www.uniprot.org/uniprot/" << swissprotId << ".xml";
		    string url(urlos.str());
		    netstreambuf in;
		    in.open(url.c_str());
		    string xml=in.content();
		    in.close();
		    int options=XML_PARSE_NOERROR|XML_PARSE_NONET;
		    record=xmlReadMemory(xml.c_str(),xml.size(),
			    url.c_str(),
			    NULL,
			    options);
		    if(record==NULL)
			{
			cerr << "#warning: Cannot find record for "<< swissprotId << endl;
			cout << line ;
			for(int i=0;i< 7;++i) cout << tokenizer.delim << ".";
			cout << endl;
			continue;
			}
		    current_recordid.assign(swissprotId);
		    }
		bool found=false;
		xmlNodePtr uniprot=xmlDocGetRootElement(record);
		xmlNodePtr entry=first(uniprot,"entry");
		if(entry!=NULL)
		    {
		    for(xmlNodePtr feature = entry->children; feature!=NULL; feature = feature->next)
			{
			if (feature->type != XML_ELEMENT_NODE) continue;
			if(!::xmlStrEqual(feature->name,BAD_CAST "feature"))
			    {
			    continue;
			    }
			bool match=false;
			xmlNodePtr location=first(feature,"location");
			if(location==NULL) continue;

			int featBeg;
			int featEnd;
			xmlNodePtr locpos=first(location,"position");
			if(locpos!=NULL)
			    {
			    featBeg=parseAttInt(locpos,"position");
			    featEnd=featBeg;
			    if(featBeg==posAA) match=true;
			    }
			else
			    {
			    xmlNodePtr locbegin=first(location,"begin");
			    xmlNodePtr locend=first(location,"end");
			    if(locbegin!=NULL && locend!=NULL)
				{
				featBeg=parseAttInt(locbegin,"position");
				featEnd=parseAttInt(locend,"position");
				if(featBeg<=featEnd && featBeg<=posAA && posAA<=featEnd)
				    {
				    match=true;
				    }
				}
			    }
			if(!match) continue;
			cout << line << tokenizer.delim
				<< featBeg  << tokenizer.delim
				<< featEnd  << tokenizer.delim
				<< parseAttStr(feature,"type")  << tokenizer.delim
				<< parseAttStr(feature,"status")  << tokenizer.delim
				<< parseAttStr(feature,"description")  << tokenizer.delim
				<< parseAttStr(feature,"evidence")  << tokenizer.delim
				<< parseAttStr(feature,"ref")
				<< endl
				;

			found=true;
			}
		    }
		if(!found)
		    {
		    cout << line;
		    for(int i=0;i< 7;++i) cout << tokenizer.delim << ".";
		    cout << endl;
		    }
		}
	    }

    virtual void usage(ostream& out,int argc,char** argv)
		{
    	out << argv[0] << " Pierre Lindenbaum PHD. 2011.\n";
		out << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
		out << "Options:\n";
		out << "  -d (char) delimiter default:tab\n";
		out << "  -p <column-index> column containing the amino acid index.\n";
		out << "  -s <spId-index> column containing the swissprot-acn (e.g.: Q04721 or  NOTC2_HUMAN).\n";
		out << "(stdin|vcf|vcf.gz)\n\n";
		}


    };




int main(int argc,char** argv)
    {
    UniProt app;
    int optind=1;
    while(optind < argc)
   		{
   		if(std::strcmp(argv[optind],"-h")==0)
   			{
   			app.usage(cerr,argc,argv);
   			return (EXIT_FAILURE);
   			}
   		else if(strcmp(argv[optind],"-p")==0)
   			{
   			char* p2;
   			app.column_aa_pos=strtol(argv[++optind],&p2,10);
   			if(*p2!=0 || app.column_aa_pos<1)
   				{
   				cerr << "bad value for option -p" << endl;
   				app.usage(cerr,argc,argv);
   				 return (EXIT_FAILURE);
   				}
   			app.column_aa_pos--;
   			}
   		else if(strcmp(argv[optind],"-s")==0)
			{
			char* p2;
			app.column_spId=strtol(argv[++optind],&p2,10);
			if(*p2!=0 || app.column_spId<1)
				{
				cerr << "bad value for option -s" << endl;
				app.usage(cerr,argc,argv);
				return (EXIT_FAILURE);
				}
			app.column_spId--;
			}
   		else if(std::strcmp(argv[optind],"-d")==0 && optind+1< argc)
			{
			char* p=argv[++optind];
			if(strlen(p)!=1)
			    {
			    cerr << "Bad delimiter \""<< p << "\"\n";
			    app.usage(cerr,argc,argv);
			    return(EXIT_FAILURE);
			    }
			app.tokenizer.delim=p[0];
			}
   		else if(argv[optind][0]=='-')
   			{
   			fprintf(stderr,"unknown option '%s'\n",argv[optind]);
   			app.usage(cerr,argc,argv);
   			return (EXIT_FAILURE);
   			}
   		else
   			{
   			break;
   			}
   		++optind;
       }
    if(app.column_aa_pos==-1)
		{
		cerr << "Undefined amino-acid column."<< endl;
		app.usage(cerr,argc,argv);
		return (EXIT_FAILURE);
		}
    if(app.column_spId==-1)
		{
		cerr << "Undefined spId column."<< endl;
		app.usage(cerr,argc,argv);
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
		if(AbstractApplication::stopping()) break;
		char* filename=argv[optind++];
		igzstreambuf buf(filename);
		istream in(&buf);
		app.run(in);
		buf.close();
		}
	    }
    return EXIT_SUCCESS;
    }

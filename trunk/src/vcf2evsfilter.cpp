/*
 * vcf2sqlite.cpp
 *
 *  Created on: Dec 25, 2011
 *      Author: lindenb
 */
#include <iostream>
#include <cstdlib>

#ifndef NOSQLITE

#include <string>
#include <vector>
#include <cstring>
#include <set>
#include <libxml/parser.h>
#include <libxml/tree.h>

#include "xsqlite.h"
#include "application.h"
#include "throw.h"
#include "zstreambuf.h"
#include "numeric_cast.h"

#include "where.h"

using namespace std;

#define TABLE "evsData"


#define ECHO_EMPTY_LINE	cout << line ;\
for(size_t i=0;i< cols.size();i++)\
    {\
    if(!limitCols.empty() && limitCols.find(cols[i])==limitCols.end())\
	{\
	continue;\
	}\
    cout << "\t.";\
    }\
if(print_xml)\
	{\
	cout << "\t.";\
	}\
cout << endl;

class VcfEvsFilter:public AbstractApplication
    {
    public:
	char* databasename;
	auto_ptr<Connection> connection;
	auto_ptr<Statement> select_variation;
	vector<string> cols;
	set<string> limitCols;
	bool print_xml;
	int32_t altColumn;

	VcfEvsFilter():databasename(NULL),print_xml(false),altColumn(-1)
	    {
	    cols.push_back("positionString");
	    cols.push_back("chrPosition");
	    cols.push_back("alleles");
	    cols.push_back("uaAlleleCounts");
	    cols.push_back("aaAlleleCounts");
	    cols.push_back("totalAlleleCounts");
	    cols.push_back("uaMAF");
	    cols.push_back("aaMAF");
	    cols.push_back("totalMAF");
	    cols.push_back("avgSampleReadDepth");
	    cols.push_back("geneList");
	    cols.push_back("conservationScore");
	    cols.push_back("conservationScoreGERP");
	    cols.push_back("refAllele");
	    cols.push_back("altAlleles");
	    cols.push_back("ancestralAllele");
	    cols.push_back("chromosome");
	    cols.push_back("hasAtLeastOneAccession");
	    cols.push_back("rsIds");
	    cols.push_back("filters");
	    cols.push_back("clinicalLink");
	    cols.push_back("dbsnpVersion");
	    cols.push_back("uaGenotypeCounts");
	    cols.push_back("aaGenotypeCounts");
	    cols.push_back("totalGenotypeCounts");
	    cols.push_back("onExomeChip");
	    cols.push_back("gwasPubmedIds");
	    }

	void close()
	    {
	    }

	void open()
	    {
	    if(databasename==0) THROW("DB name undefined.");
	    ConnectionFactory cf;
	    cf.set_allow_create(false);
	    cf.set_read_only(true);
	    cf.set_filename(databasename);
	    this->connection=cf.create();
	    select_variation = this->connection->prepare(
	       "select xml from " TABLE " where chrom=? and pos=?"
		);
	    }





	void run(std::istream& in)
	    {
	    Tokenizer tab('\t');
	    Tokenizer comma(',');
	    vector<string> tokens;
	    string line;
	    while(getline(in,line,'\n'))
		{
		if(line.empty()) continue;
		if(line[0]=='#')
		    {
		    cout << line;
		    for(size_t i=0;i< cols.size();i++)
			{
			if(!limitCols.empty() && limitCols.find(cols[i])==limitCols.end())
			    {
			    continue;
			    }
			cout << "\t";
			cout << "evs." << cols[i];
			}
		    if(print_xml)
			{
			cout << "\tevs.xml";
			}
		    cout << endl;
		    continue;
		    }




		tab.split(line,tokens);
		if(tokens.size()<2)
		    {
		    THROW("Expected 2 columns in "<< line);
		    }

		if(altColumn!=-1 && (int)tokens.size()<=altColumn)
		    {
		    THROW("cannot get the ALT column in "<< line);
		    }

		int32_t position;
		if(!numeric_cast<int32_t>(tokens[1].c_str(),&position))
		    {
		    THROW("Not a position in " << line);
		    }
		bool found=false;
		auto_ptr<string> ret(0);
		select_variation->reset();
		select_variation->bind_string(1,tokens[0].c_str());
		select_variation->bind_int(2,position);

		for(;;)
		    {

		    if(select_variation->step()==Statement::DONE)
			{

			break;
			}

		    const char* xml=select_variation->get_string(1);
		    if(xml==0) continue;
		    found=true;

		    xmlDocPtr dom=::xmlParseMemory(xml,strlen(xml));
		    if(dom==0)
			{
			THROW("Cannot parse "<<xml);
			}
		    xmlNodePtr root=xmlDocGetRootElement(dom);
		    if(root==0)
			{
			THROW("Cannot get root: "<<xml);
			}
		    if(altColumn!=-1)
			{
			bool ok=false;
			for(xmlNodePtr c2 = (root==0?0:root->children);
				c2!=0;
				c2 = c2->next)
			     {
			     if(c2->type != XML_ELEMENT_NODE) continue;
			     if(!xmlStrEqual(c2->name,BAD_CAST "altAlleles")) continue;
			     xmlChar* xAlt=xmlNodeGetContent(c2);

			     vector<string> tokens2;
			     comma.split((const char*)xAlt,tokens2);
			     xmlFree(xAlt);
			     for(size_t k=0;k< tokens2.size();++k)
				 {
				 if(strcasecmp(tokens2[k].c_str(),tokens[altColumn].c_str())==0)
				    {
				    ok=true;
				    break;
				    }
				 }
			      if(ok) break;
			      }
			if(!ok)
			    {
			    ECHO_EMPTY_LINE
			    continue;
			    }
			}

		    cout << line ;



		    for(size_t i=0;i< cols.size();i++)
			{
			if(!limitCols.empty() && limitCols.find(cols[i])==limitCols.end())
			    {
			    continue;
			    }

			xmlChar* content=0;
			for(xmlNodePtr c2 = (root==0?0:root->children);
				c2!=0;
				c2 = c2->next)
			     {
			     if(c2->type != XML_ELEMENT_NODE) continue;
			     if(!xmlStrEqual(c2->name,BAD_CAST cols[i].c_str()))
				{
				continue;
				}
			     content=xmlNodeGetContent(c2);
			     break;
			     }
			cout << "\t";
			if(content!=NULL)
			    {
			    cout << content;
			    xmlFree(content);
			    }
			else
			    {
			    cout << ".";
			    }
			}

		    if(print_xml)
			{
			cout << "\t" << xml;
			}
		    cout << endl;
		    xmlFree(dom);
		    }


		if(!found)
		    {
		    ECHO_EMPTY_LINE
		    }

		}



	    }


	virtual void usage(ostream& out,int argc,char** argv)
	    {
	    out << argv[0] << " Pierre Lindenbaum PHD. 2012.\n";
	    out << VARKIT_REVISION << endl;
	    out << "Append EVS data to VCF like file from a sqlite database.\n";
	    out << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
	    out << "Options:\n";
	    out << "  -f (file) sqlite filename. (REQUIRED).\n";
	    out << "  -x print xml .\n";
	    out << "  -a (column index) ALTernate allele column (optional) .\n";
	    out << "  -c (column-name) limit header to this column name (can be used multiple times).\n";
	    out << "(stdin|vcf|vcf.gz)\n\n";
	    out << endl;
	    out << "Building the SQLITE table:"<< endl;
	    out << " sqlite> create table evsData(chrom TEXT NOT NULL,pos INT NOT NULL,xml TEXT NOT NULL);\n";
	    out << " sqlite> create index chrompos on evsData(chrom,pos);\n";
	    out << " sqlite> .separator \"\\t\";\n";
	    out << " sqlite> .import \"input.evs.tsv\" evsData;\n";

	    out << endl;
	    }

	int main(int argc,char** argv)
		{
		int optind=1;

		while(optind < argc)
		    {
		    if(std::strcmp(argv[optind],"-h")==0)
			{
			this->usage(cerr,argc,argv);
			return (EXIT_FAILURE);
			}
		    else if(strcmp(argv[optind],"-x")==0 )
			{
			print_xml=true;
			}
		    else if(strcmp(argv[optind],"-f")==0 && optind+1<argc)
			{
			databasename=argv[++optind];
			}
		    else if(strcmp(argv[optind],"-a")==0 && optind+1<argc)
			{
			if(!numeric_cast(argv[++optind],&altColumn) || altColumn<1)
			    {
			    cerr << "Bad ALT column " << argv[optind] << endl;
			    return EXIT_FAILURE;
			    }
			altColumn--;
			}
		    else if(strcmp(argv[optind],"-c")==0 && optind+1<argc)
			{
			limitCols.insert((argv[++optind]));
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
		if(databasename==NULL)
		    {
		    cerr << "Undefined sqlite database."<< endl;
		    this->usage(cerr,argc,argv);
		    return (EXIT_FAILURE);
		    }
		open();
		if(optind==argc)
		    {
		    igzstreambuf buf;
		    istream in(&buf);
		    this->run(in);
		    }
		else
		    {
		    while(optind< argc)
			{
			if(AbstractApplication::stopping()) break;
			char* input=argv[optind++];
			igzstreambuf buf(input);
			istream in(&buf);
			this->run(in);
			buf.close();
			}
		    }
		close();
		return EXIT_SUCCESS;
		}
    };


int main(int argc,char** argv)
    {
    VcfEvsFilter app;
    return app.main(argc,argv);
    }

#else

int main(int argc,char** argv)
    {
    std::cerr << argv[0] << " was compiled without sqlite3 ( $SQLITE_LIB undefined) \n";
    return EXIT_FAILURE;
    }

#endif

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
#include "xsqlite.h"
#include "application.h"
#include "throw.h"
#include "zstreambuf.h"
#include "numeric_cast.h"
#define NOWHERE
#include "where.h"
#include "xstream.h"

using namespace std;

#define RDF_NS "http://www.w3.org/1999/02/22-rdf-syntax-ns#"

class GoDatabaseManager:public AbstractApplication
    {
    public:
	char* dataasename;
	auto_ptr<Connection> connection;


	class RDFHandler:public XmlStream
	       {
		public:
		   RDFHandler(std::istream& in):XmlStream(in) {}
		   virtual ~RDFHandler() {}

		   virtual bool isPivotNode(const xmlNodePtr element,int depth1) const
		       {
		       return depth1==3 && xmlStrEqual(element->name,"go:term");
		       }

	       };

	GoDatabaseManager():dataasename(NULL)
	    {

	    }

	void close()
	    {

	    }

	void open(bool read_only)
	    {
	    if(dataasename==0) THROW("DB name undefined.");
	    ConnectionFactory cf;
	    cf.set_allow_create(!read_only);
	    cf.set_read_only(read_only);
	    cf.set_filename(dataasename);
	    this->connection=cf.create();
	    if(!read_only)
		{
		this->connection->execute(
		       "create table if not exists TERM("
		       "acn varchar(50) NOT NULL UNIQUE, "
		       "xml TEXT NOT NULL "
		       ")");
		this->connection->execute(
		   "create table if not exists TERM2REL("
		   "acn varchar(50) NOT NULL, "
		   "rel varchar(50) NOT NULL, "
		    "target varchar(50) NOT NULL"
		   ")");
		this->connection->execute(
		   "create index if not exists TERM2REL_ACN on TERM2REL.acn"
		   );
		this->connection->execute(
		   "create index if not exists TERM2REL_TARGET on TERM2REL.target"
		   );

		}

	    }

	static  xmlNodePtr first( xmlNodePtr root,const xmlChar* name)
	    {
	    if(root==0) return 0;
	    for(xmlNodePtr cur_node = root; cur_node; cur_node = xmlNextElementSibling(cur_node))
	            {
	            if(cur_node->type != XML_ELEMENT_NODE) continue;
	            if( xmlStrEqual(cur_node->name,name)) return cur_node;
	            }
	    return 0;
	    }

	static int _xmlOutputWriteCallback(void * context,
						 const char * buffer,
						 int len)
	    {
	    ((std::string*)context)->append(buffer,len);
	    return len;
	    }
	static int _xmlOutputCloseCallback(void * context)
	    {
	    return 0;
	    }

	void insert_go_rdf(std::istream& in)
	    {
	    auto_ptr<RDFHandler> handler=auto_ptr<RDFHandler>(new RDFHandler(in));
	    open(false);
	    this->connection->begin();

	    auto_ptr<Statement> insert_goterm = this->connection->prepare(
	    		   "insert or REPLACE into TERM(acn,xml) values (?,?)"
	    		    );

	    auto_ptr<Statement> delete_term2rel= this->connection->prepare(
		   "delete from TERM2REL where acn=?"
		    );

	    auto_ptr<Statement> insert_term2rel= this->connection->prepare(
	   		   "insert or REPLACE into TERM2REL(ac,rel,target)"
	   		    );

	    for(;;)
		{
		xmlDocPtr doc= handler->next();
	 	if(doc==0) break;
	 	xmlNodePtr term =handler->getCurrentPivot();
	 	xmlNodePtr acn=first(term,BAD_CAST"go:accession");
	 	if(acn==0) continue;
	 	xmlChar *acnstr=xmlNodeGetContent(acn);
	 	if(acnstr==0) continue;

	 	std::string xml;
	 	xmlOutputBufferPtr outbuf=::xmlOutputBufferCreateIO(
	 		_xmlOutputWriteCallback,
	 		_xmlOutputCloseCallback,
			 (void)&xml,
			 0);
	 	if(outbuf==0) THROW("Cannot xmlOutputBufferCreateIO");
		xmlNodeDumpOutput(outbuf,doc,term,0,0,0);
		xmlOutputBufferClose(outbuf);
		insert_goterm->reset();
		insert_goterm->bind_string(1,(const char*)acnstr);
		insert_goterm->bind_string(2,(const char*)xml.c_str());
		insert_variation->execute();

		delete_term2rel->reset();
		delete_term2rel->bind_string(1,(const char*)acnstr);
		delete_term2rel->execute();

		for(xmlNodePtr cur_node = root; cur_node; cur_node = xmlNextElementSibling(cur_node))
		    {
		    if(cur_node->type != XML_ELEMENT_NODE) continue;

		    xmlChar* rsrc=xmlGetNsProp(cur_node,BAD_CAST "rdf:resource",BAD_CAST RDF_NS);
		    if(rsrc==0) continue;
		    const char* hash=strchr((const char*)rsrc,'#');
		    if(hash!=0 &&
			(
			xmlStrEqual(cur_node->name,BAD_CAST "rdf:is_a") ||
			xmlStrEqual(cur_node->name,BAD_CAST "rdf:part_of")
			))
			{
			const char* colon=strchr((const char*)cur_node->name,':');
			insert_term2rel->reset();
			insert_term2rel->bind_string(1,(const char*)acnstr);
			insert_term2rel->bind_string(2,(const char*)(colon+1));
			insert_term2rel->bind_string(3,(const char*)(hash+1));
			insert_term2rel->execute();
			}
		    xmlFree(rsrc);
		    }

		xmlFree(acnstr);
		}
	    this->connection->commit();
	    }


	virtual void usage(ostream& out,int argc,char** argv)
	    {
	    out << argv[0] << " Pierre Lindenbaum PHD. 2012.\n";
	    out << VARKIT_REVISION << endl;
	    out << "sqlite3 database manager for GeneOntology.\n";
	    out << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
	    out << "Options:\n";
	    out << "  -f (file) sqlite filename. (REQUIRED).\n";
	    out << "(stdin|vcf|vcf.gz)\n\n";
	    }

	int main_insertrdf(int argc,char** argv,int optind)
		{
		while(optind < argc)
		    {
		    if(std::strcmp(argv[optind],"-h")==0)
			{
			this->usage(cerr,argc,argv);
			return (EXIT_FAILURE);
			}
		    else if(strcmp(argv[optind],"-f")==0 && optind+1<argc)
			{
			dataasename=argv[++optind];
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
		if(dataasename==NULL)
		    {
		    cerr << "Undefined sqlite database."<< endl;
		    this->usage(cerr,argc,argv);
		    return (EXIT_FAILURE);
		    }
		open(false);
		if(optind==argc)
		    {
		    igzstreambuf buf;
		    istream in(&buf);
		    this->insert_go_rdf(in);
		    }
		else
		    {
		    while(optind< argc)
			{
			if(AbstractApplication::stopping()) break;
			char* input=argv[optind++];
			igzstreambuf buf(input);
			istream in(&buf);
			this->insert_go_rdf(input);
			buf.close();
			}
		    }
		close();
		return EXIT_SUCCESS;
		}

	int main(int argc,char** argv)
	    {
	    char *program=0;
	    int optind=1;
	    while(optind < argc)
		{
		if(std::strcmp(argv[optind],"-h")==0)
		    {
		    this->usage(cerr,argc,argv);
		    return (EXIT_FAILURE);
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

	    if(optind==argc)
		{
		cerr << "program name missing\n";
		this->usage(cerr,argc,argv);
		return (EXIT_FAILURE);
		}
	    if(strcmp(argv[optind],"loadrdf")==0)
		{
		return main_insertrdf(argc,argv,optind);
		}
	    else
		{
		cerr << "unknown program " << argv[optind] << "\n";
		this->usage(cerr,argc,argv);
		return (EXIT_FAILURE);
		}
	    return EXIT_SUCCESS;
	    }

    };


int main(int argc,char** argv)
    {
    GoDatabaseManager app;
    return app.main(argc,argv);
    }

#else

int main(int argc,char** argv)
    {
    std::cerr << argv[0] << " was compiled without sqlite3 ( $SQLITE_LIB undefined) \n";
    return EXIT_FAILURE;
    }

#endif

/*
 * GoDatabaseManager
 */
#include <iostream>
#include <cstdlib>
#include <cassert>
#ifndef NOSQLITE

#include <string>
#include <vector>
#include <set>
#include <cstring>
#include "xsqlite.h"
#include "application.h"
#include "throw.h"
#include "zstreambuf.h"
#include "numeric_cast.h"
//#define NOWHERE
#include "where.h"
#include "xxml.h"
#include "xstream.h"
#include "cescape.h"

using namespace std;

#define RDF_NS "http://www.w3.org/1999/02/22-rdf-syntax-ns#"

class GoDatabaseManager:public AbstractApplication
    {
    private:
	set<string> relation_ontology;
	int32_t goa_keyword_column;
	int32_t go_acn_column;
	bool inverse_result;
	bool is_relation_ontology(const char* s) const
	    {
	    if(s==0) return false;
	    std::string s2(s);
	    std::string::size_type colon=s2.find(':');
	    if(colon!=string::npos) s2.erase(0,colon+1);
	    return relation_ontology.find(s2)!=relation_ontology.end();
	    }

    public:
	char* dataasename;
	auto_ptr<Connection> connection;
	auto_ptr<Statement> select_term_stmt;


	class RDFHandler:public XmlStream
	       {
		public:
		   RDFHandler(std::istream& in):XmlStream(in) {}
		   virtual ~RDFHandler() {}

		   virtual bool isPivotNode(const xmlNodePtr element,int depth1) const
		       {
		       return depth1==3 && ::xmlStrEqual(element->name,BAD_CAST "go:term");
		       }

	       };

	GoDatabaseManager():goa_keyword_column(-1),
		go_acn_column(-1),
		inverse_result(false),
		dataasename(NULL)
	    {
		relation_ontology.insert("is_a");
		relation_ontology.insert("part_of");
		relation_ontology.insert("has_part");
		relation_ontology.insert("integral_part_of");
		relation_ontology.insert("has_integral_part");
		relation_ontology.insert("proper_part_of");
		relation_ontology.insert("has_proper_part");
		relation_ontology.insert("located_in");
		relation_ontology.insert("location_of");
		relation_ontology.insert("contained_in");
		relation_ontology.insert("contains");
		relation_ontology.insert("adjacent_to");
		relation_ontology.insert("transformation_of");
		relation_ontology.insert("transformed_into");
		relation_ontology.insert("derives_from");
		relation_ontology.insert("derived_into");
		relation_ontology.insert("preceded_by");
		relation_ontology.insert("precedes");
		relation_ontology.insert("has_participant");
		relation_ontology.insert("participates_in");
		relation_ontology.insert("has_agent");
		relation_ontology.insert("agent_in");
		relation_ontology.insert("instance_of");
		relation_ontology.insert("has_improper_part");
		relation_ontology.insert("improper_part_of");
		relation_ontology.insert("relationship");
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
		   "create index if not exists TERM2REL_ACN on TERM2REL(acn)"
		   );
		this->connection->execute(
		   "create index if not exists TERM2REL_TARGET on TERM2REL(target)"
		   );


		this->connection->execute(
		   "create table if not exists GOA("
		   "DB varchar(50) NOT NULL, "
		   "DB_Object_ID varchar(50) NOT NULL, "
		   "DB_Object_Symbol varchar(50) NOT NULL, "
	           //"Qualifier varchar(50) NOT NULL, "
		   "term varchar(50) NOT NULL, "
		   "DB_Object_Name TEXT NOT NULL, "
	           "Synonym TEXT, "
		   " DB_Object_Type varchar(10) NOT NULL "
		   ")");
		this->connection->execute(
		   "create index if not exists GOA_DB_Object_ID on GOA(DB_Object_ID)"
		   );
		this->connection->execute(
		   "create index if not exists GOA_term on GOA(term)"
		   );
		this->connection->execute(
		   "create index if not exists GOA_symbol on GOA(DB_Object_Symbol)"
		   );
		}
	    select_term_stmt=this->connection->prepare("select xml from TERM where acn=?");
	    }

	auto_ptr<string> getTermXmlByAcn(const char* term)
		{
	       if(term==0) return auto_ptr<string>(0);
		select_term_stmt->reset();
		select_term_stmt->bind_string(1,term);
		while(select_term_stmt->step()!=Statement::DONE)
		    {
		    return auto_ptr<string>(new string(select_term_stmt->get_string(1)));
		    }
		return auto_ptr<string>(0);
		}


	xmlDocPtr getTermDomByAcn(const char* term)
	    {
	    auto_ptr<string> ret=getTermXmlByAcn(term);
	    if(ret.get()==0) return 0;
	    xmlDocPtr dom=xmlParseMemory(ret->data(),(int)ret->size());
	    if(dom==0)
		{
		THROW("CANNOT PARSE XML "<< ret->c_str());
		}
	    return dom;
	    }


	 void _find_descendants(
		 const char* term,
		 set<string>* terms,
		 Statement* select_descendants
		 )
		{
		if(terms->find(term)!=terms->end()) return;
		terms->insert(term);
		select_descendants->reset();
		select_descendants->bind_string(1,term);
		set<string> to_process;
		while(select_descendants->step()!=Statement::DONE)
		    {

		    const char* child=select_descendants->get_string(1);
		    assert(child!=0);
		    if(terms->find(child)!=terms->end()) continue;
		    to_process.insert(child);
		    }

		for(set<string>::iterator r=to_process.begin();
			r!=to_process.end();
			++r)
		    {
		    _find_descendants(r->c_str(),terms,select_descendants);
		    }
		}

	std::auto_ptr<set<string> > find_descendants(const char* term,const set<string>& relations)
	    {
	    ostringstream os;
	    os << "select acn from TERM2REL where target=?";
	    if(!relations.empty())
		{
		os << " and (";
		for(set<string>::const_iterator r=relations.begin();r!=relations.end();++r)
		    {
		    if(r!=relations.begin()) os << " or ";
		    os << " rel=\""<< CEscape(*r) << "\"";
		    }
		os << ")";
		}
	    string sql(os.str());
	    std::auto_ptr<set<string> > ret(new set<string>());
	    auto_ptr<Statement> select_descendants=this->connection->prepare(sql.c_str());
	    _find_descendants(term,ret.get(),select_descendants.get());
	    return ret;
	    }

	void _find_ascendants(
			 const char* term,
			 set<string>* terms,
			 Statement* select_ascendants
			 )
	    {
	    if(terms->find(term)!=terms->end()) return;
	    terms->insert(term);
	    select_ascendants->reset();
	    select_ascendants->bind_string(1,term);
	    set<string> to_process;
	    while(select_ascendants->step()!=Statement::DONE)
		{

		const char* child=select_ascendants->get_string(1);
		assert(child!=0);
		if(terms->find(child)!=terms->end()) continue;
		to_process.insert(child);
		}

	    for(set<string>::iterator r=to_process.begin();
		    r!=to_process.end();
		    ++r)
		{
		_find_ascendants(r->c_str(),terms,select_ascendants);
		}
	    }

	std::auto_ptr<set<string> > find_ascendants(const char* term,const set<string>& relations)
	    {
	    ostringstream os;
	    os << "select target from TERM2REL where acn=?";
	    if(!relations.empty())
		{
		os << " and (";
		for(set<string>::const_iterator r=relations.begin();r!=relations.end();++r)
		    {
		    if(r!=relations.begin()) os << " or ";
		    os << " rel=\""<< CEscape(*r) << "\"";
		    }
		os << ")";
		}
	    string sql(os.str());
	    std::auto_ptr<set<string> > ret(new set<string>());
	    auto_ptr<Statement> select_ascendants=this->connection->prepare(sql.c_str());
	    _find_ascendants(term,ret.get(),select_ascendants.get());
	    return ret;
	    }

	static  xmlNodePtr first( xmlNodePtr root,const xmlChar* name)
	    {
	    if(root==0) return 0;
	    for(xmlNodePtr cur_node = xmlFirstElementChild(root); cur_node; cur_node = xmlNextElementSibling(cur_node))
	            {
	            if(cur_node->type != XML_ELEMENT_NODE) continue;
	            if( xmlStrEqual(cur_node->name,name)) return cur_node;
	            }
	    return 0;
	    }



	void insert_go_rdf(std::istream& in)
	    {
	    auto_ptr<RDFHandler> handler=auto_ptr<RDFHandler>(new RDFHandler(in));
	    this->connection->begin();

	    auto_ptr<Statement> insert_goterm = this->connection->prepare(
	    		   "insert or REPLACE into TERM(acn,xml) values (?,?)"
	    		    );

	    auto_ptr<Statement> delete_term2rel= this->connection->prepare(
		   "delete from TERM2REL where acn=?"
		    );

	    auto_ptr<Statement> insert_term2rel= this->connection->prepare(
	   		   "insert or REPLACE into TERM2REL(acn,rel,target) values (?,?,?)"
	   		    );

	    for(;;)
		{
		xmlDocPtr doc= handler->next();
	 	if(doc==0) break;

	 	xmlNodePtr term =handler->getCurrentPivot();
	 	xmlNodePtr acn=first(term,BAD_CAST"go:accession");
	 	if(acn==0)
	 	    {
	 	   cerr << "acn ?"<< endl;
	 	    continue;
	 	    }
	 	xmlChar *acnstr=xmlNodeGetContent(acn);
	 	if(acnstr==0) continue;
	 	std::string xml;
	 	xmlOutputBufferPtr outbuf=::xmlOutputBufferCreateString(&xml,0);
	 	xmlNodeDumpOutput(outbuf,doc,term,0,0,0);
		xmlOutputBufferClose(outbuf);
		insert_goterm->reset();
		insert_goterm->bind_string(1,(const char*)acnstr);
		insert_goterm->bind_string(2,(const char*)xml.c_str());
		insert_goterm->execute();

		delete_term2rel->reset();
		delete_term2rel->bind_string(1,(const char*)acnstr);
		delete_term2rel->execute();

		for(xmlNodePtr cur_node = xmlFirstElementChild(term); cur_node;
		    cur_node = xmlNextElementSibling(cur_node))
		    {
		    if(cur_node->type != XML_ELEMENT_NODE) continue;

		    xmlChar* rsrc=xmlGetNsProp(cur_node,BAD_CAST "resource",BAD_CAST RDF_NS);
		    if(rsrc==0)
			{

			continue;
			}

		    const char* hash=strchr((const char*)rsrc,'#');

		    if(hash!=0 && is_relation_ontology((const char*)cur_node->name))
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
	    this->connection->compact();
	    }


	void insert_goa(std::istream& in)
	    {
	    Tokenizer tab('\t');
	    vector<string> tokens;
	    string line;
	    auto_ptr<Statement> insert_goa = this->connection->prepare(
		   "insert into GOA(DB,DB_Object_ID,DB_Object_Symbol,term,DB_Object_Name,Synonym, DB_Object_Type) "
		   "         values(?,?,?,?,?,?,?)"
		    );
	    this->connection->begin();
	    for(;;)
		{

		if(!getline(in,line,'\n')) break;

		if(line.empty() || line[0]=='!') continue;
		tab.split(line,tokens);
		if(tokens.size()<12) continue;
		if(tokens[5].find("NOT")!=string::npos) continue;
		insert_goa->reset();
		insert_goa->bind_string(1,tokens[0].c_str());//DB
		insert_goa->bind_string(2,tokens[1].c_str());//DB_Object_ID
		insert_goa->bind_string(3,tokens[2].c_str());//DB_Object_Symbol
		insert_goa->bind_string(4,tokens[4].c_str());//term
		insert_goa->bind_string(5,tokens[9].c_str());//DB_Object_Name
		insert_goa->bind_string(6,tokens[10].c_str());//Synonym
		insert_goa->bind_string(7,tokens[11].c_str());//DB_Object_Type

		insert_goa->execute();
		}

	    this->connection->commit();
	    this->connection->compact();
	    }

	void run_goa(std::istream& in)
	    {
	    Tokenizer tab('\t');
	    vector<string> tokens;
	    string line;
	    auto_ptr<Statement> select_goa = this->connection->prepare(
		   "select DB,DB_Object_ID,DB_Object_Symbol,term,DB_Object_Name,Synonym, DB_Object_Type "
		   " from GOA where (DB_Object_ID=? or DB_Object_Symbol=? or term=? )"
		    " "
		    );

	    for(;;)
		{
		if(!getline(in,line,'\n')) break;
		if(line.empty()) continue;
		if(line[0]=='#')
		    {
		    cout << line << "\t";
		    cout << "DB\tDB_Object_ID\tDB_Object_Symbol\tterm\tDB_Object_Name,Synonym\tDB_Object_Type";
		    cout << endl;
		    continue;
		    }
		tab.split(line,tokens);
		if((int32_t)tokens.size()<=goa_keyword_column)
		    {
		    cerr << "column "<< (goa_keyword_column+1)<< " missing in "<< line << endl;
		    continue;
		    }
		bool found=false;
		if(!tokens[goa_keyword_column].empty())
		    {
		    select_goa->reset();
		    select_goa->bind_string(1,tokens[goa_keyword_column].c_str());
		    select_goa->bind_string(2,tokens[goa_keyword_column].c_str());
		    select_goa->bind_string(3,tokens[goa_keyword_column].c_str());
		    while(select_goa->step()!=Statement::DONE)
			{
			found=true;
			cout << line;
			for(int c=0;c < select_goa->column_count();++c)
			    {
			    cout << "\t";
			    cout << select_goa->get_string(c+1);
			    }
			cout << endl;
			}
		    }
		if(!found)
		    {
		    cout << line;
		    for(int c=0;c < 7;++c)
			{
			cout << "\t";
			cout << ".";
			}
		    cout << endl;
		    }
		}


	    }


	void run_grep(std::istream& in,const set<string>& all_descendants)
	    {
	    Tokenizer tab('\t');
	    vector<string> tokens;
	    string line;

	    for(;;)
		{
		if(!getline(in,line,'\n')) break;
		if(line.empty()) continue;
		if(line[0]=='#')
		    {
		    cout << line << endl;
		    continue;
		    }
		tab.split(line,tokens);
		if((int32_t)tokens.size()<=go_acn_column)
		    {
		    cerr << "column "<< (go_acn_column+1)<< " missing in "<< line << endl;
		    continue;
		    }
		bool found=false;
		if( (all_descendants.find(tokens[go_acn_column])!=all_descendants.end())!=inverse_result)
		    {
		    cout << line << endl;
		    }
		}


	    }

	virtual void usage(ostream& out,int argc,char** argv)
	    {
	    out << argv[0] << " Pierre Lindenbaum PHD. 2012.\n";
	    out << VARKIT_REVISION << endl;
	    out << "sqlite3 database manager for GeneOntology.\n";
	    out << "Usage:\n\t"<< argv[0]<< " progam_name\n";
	    out << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
	    out << "\nProgram: loadrdf\n\n";
	    out << " insert the RDF/XML database for GO into a sqlite database.\n";
	    out << "  Usage\n";
	    out << " "<< argv[0] << " loadrdf -f db.sqlite (stdin|rdf-files)\n";
	    out << " Options for :\n";
	    out << "   -f (file) sqlite filename. (REQUIRED).\n";
	    out << "\nProgram: loadgoa\n\n";
	    out << " insert the database for GOA into a sqlite database (e.g: ftp://ftp.ebi.ac.uk/pub/databases/GO/goa/HUMAN/gene_association.goa_human.gz) .\n";
	    out << "  Usage\n";
	    out << " "<< argv[0] << " loadgoa -f db.sqlite (stdin|rdf-files)\n";
	    out << " Options for :\n";
	    out << "   -f (file) sqlite filename. (REQUIRED).\n";

	    out << "\nProgram: desc\n\n";
	    out << " print the descendants (children) of a given node.\n";
	    out << "  Usage\n";
	    out << " "<< argv[0] << " desc -f db.sqlite term1 term2 ... termn\n";
	    out << " Options for :\n";
	    out << "   -f (file) sqlite filename. (REQUIRED).\n";
	    out << "   -r (rel) add a go relationship (OPTIONAL, default: it adds \"is_a\").\n";
	    out << "   -t output: xml, goa ,tsv ,acn\n";

	    out << "\nProgram: asc\n\n";
	    out << " print the ascendants (parents) of a given node.\n";
	    out << "  Usage\n";
	    out << " "<< argv[0] << " desc -f db.sqlite term1 term2 ... termn\n";
	    out << " Options for :\n";
	    out << "   -f (file) sqlite filename. (REQUIRED).\n";
	    out << "   -r (rel) add a go relationship (OPTIONAL, default: it adds \"is_a\").\n";
	    out << "   -t output: xml, goa, tsv ,acn\n";


	    out << "\nProgram: goa\n\n";
	    out << " appends the GOA annotation\n";
	    out << "  Usage\n";
	    out << " "<< argv[0] << " goa [options] (file1 file2 .. | stdin)\n";
	    out << " Options for :\n";
	    out << "   -f (file) sqlite filename. (REQUIRED).\n";
	    out << "   -c (column index) REQUIRED. The observed column\n";
	    out << "\nProgram: grep\n\n";
	    out << " filters line having a GO acn descendant of one or more go term.\n";
	    out << "  Usage\n";
	    out << " "<< argv[0] << " grep [options] (file1 file2 .. | stdin)\n";
	    out << " Options for grep:\n";
	    out << "   -f (file) sqlite filename. (REQUIRED).\n";
	    out << "   -c (column index) REQUIRED. The column containing a GO:acn\n";
	    out << "   -v inverse selection\n";
	    out << "   -t (GO:acn) add a GO term in the filter (One is REQUIRED).\n";
	    out << "   -r (rel) add a go relationship (OPTIONAL, default: it adds \"is_a\").\n";
	    out << endl;
	    }

	int main_insertrdf(int argc,char** argv,int optind)
		{
		int output_fmt=0;
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


		 this->connection->execute("delete from GOA");

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
			this->insert_go_rdf(in);
			buf.close();
			}
		    }
		close();
		return EXIT_SUCCESS;
		}

	int main_insertgoa(int argc,char** argv,int optind)
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
		this->insert_goa(in);
		}
	    else
		{
		while(optind< argc)
		    {
		    if(AbstractApplication::stopping()) break;
		    char* input=argv[optind++];
		    igzstreambuf buf(input);
		    istream in(&buf);
		    this->insert_goa(in);
		    buf.close();
		    }
		}
	    close();
	    return EXIT_SUCCESS;
	    }




	int main_descendants_or_ascendant(int argc,char** argv,int optind,bool is_descendant)
	    {
	    int output_format=0;
	    set<string> relationships;
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
		else if(strcmp(argv[optind],"-r")==0 && optind+1<argc)
		    {
		    const char* s=(argv[++optind]);
		    if(!is_relation_ontology(s))
			{
			cerr << "Not in relation ontology: "<< s << endl;
			this->usage(cerr,argc,argv);
			return (EXIT_FAILURE);
			}
		    relationships.insert(s);
		    }
		else if(strcmp(argv[optind],"-t")==0 && optind+1<argc)
		    {
		    char* fmt=argv[++optind];
		    if(strcmp(argv[optind],"xml")==0)
		    	{
		    	output_format=1;
		    	}
		    else if(strcmp(argv[optind],"goa")==0)
		    	{
		    	output_format=2;
		    	}
		    else if(strcmp(argv[optind],"tsv")==0)
			{
			output_format=3;
			}
		    else
			{
			output_format=0;
			}
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
	    if(relationships.empty())
		{
		relationships.insert("is_a");
		}

	    set<string> terms;
	    open(true);




	    if(optind==argc)
		{
		cerr << "No term given."<< endl;
		return EXIT_SUCCESS;
		}
	    while(optind< argc)
		{
		terms.insert(argv[optind++]);
		}



	    set<string> result;
	    for(set<string>::iterator r=terms.begin();r!=terms.end();++r)
		{
		auto_ptr<set<string> > ret(0);
		if(is_descendant)
		    {
		    ret=this->find_descendants(r->c_str(),relationships);
		    }
		else
		    {
		    ret=this->find_ascendants(r->c_str(),relationships);
		    }
		result.insert(ret->begin(),ret->end());
		}
	    /* XML format */
	    if(output_format==1)
		{
		cout << "<go:go xmlns:go='http://www.geneontology.org/dtds/go.dtd#' xmlns:rdf='http://www.w3.org/1999/02/22-rdf-syntax-ns#'>\n"
			 " <rdf:RDF>\n"
			;
		for(set<string>::iterator r=result.begin();r!=result.end();++r)
		    {
		    xmlDocPtr dom=getTermDomByAcn(r->c_str());
		    if(dom==0)
			{
			cout << "<!-- CANNOT FIND "<< (*r) << " in DOM database ?? -->\n";
			cerr << "Cannot find "<< (*r) << " in DOM database ?"<< endl;
			continue;
			}
		    xmlOutputBufferPtr outbuf=::xmlOutputBufferCreateIOStream(&cout,0);
		    if(outbuf==0) THROW("Cannot xmlOutputBufferCreateIO");
		    xmlNodeDumpOutput(outbuf,dom,xmlDocGetRootElement(dom),0,0,0);
		    xmlOutputBufferClose(outbuf);
		    xmlFreeDoc(dom);
		    cout  << endl;
		    }
		cout << " </rdf:RDF>\n</go:go>";
		}
	    else if(output_format==2) /* GOA format */
	    	{
	    	auto_ptr<Statement> select_goa=this->connection->prepare(
	    		"select * from 	GOA where term=?"
	    		);
	    	
	    	for(set<string>::iterator r=result.begin();
	    		r!=result.end();++r)
		    {
		    select_goa->reset();
		    select_goa->bind_string(1,(*r).c_str());
		    while(select_goa->step()!=Statement::DONE)
		    	{
		    	for(int j=0;j< select_goa->column_count();++j)
		    		{
		    		if(j!=0) cout << "\t";
		    		cout << select_goa->get_string(j+1);
		    		}
		    	cout << endl;
		    	}
		    }
	    	}
	    else if(output_format==3)
		{
		cout << "#go:accession\tgo.name\tgo.def"<< endl;
		for(set<string>::iterator r=result.begin();r!=result.end();++r)
		    {
		    xmlDocPtr dom=getTermDomByAcn(r->c_str());
		    if(dom==0)
			{
			cout << (*r)<< "\t.\t."<< endl;
			continue;
			}
		    cout << (*r);
		    for(int k=0;k<2;++k)
			    {
			    cout << "\t";
			    xmlNodePtr term=xmlDocGetRootElement(dom);
			    xmlNodePtr elmt=first(term,BAD_CAST (k==0?"name":"definition"));
			    if(elmt==0)
				{
				cout << ".";
				continue;
				}
			    xmlChar *content=xmlNodeGetContent(elmt);
			    if(content==0)
				{
				cout << ".";
				}
			    else
				{
				cout << CEscape((const char*) content);
				xmlFree(content);
				}
			}
		    cout << endl;
		    xmlFreeDoc(dom);
		    }
		}
	    else
		{
		for(set<string>::iterator r=result.begin();r!=result.end();++r)
		    {
		    cout << (*r) << endl;
		    }
		}
	    close();
	    return EXIT_SUCCESS;
	    }




	int main_goa(int argc,char** argv,int optind)
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
		else if(strcmp(argv[optind],"-c")==0 && optind+1<argc)
		    {
		    if(!numeric_cast<int32_t>(argv[++optind],&goa_keyword_column) || goa_keyword_column<1)
			{
			cerr << "Bad column index "<< argv[optind]<< endl;
			this->usage(cerr,argc,argv);
			return (EXIT_FAILURE);
			}
		    goa_keyword_column--;
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
	    if(goa_keyword_column<0)
		{
		cerr << "Undefined column."<< endl;
		this->usage(cerr,argc,argv);
		return (EXIT_FAILURE);
		}
	    open(true);
	    if(optind==argc)
		{
		igzstreambuf buf;
		istream in(&buf);
		this->run_goa(in);
		}
	    else
		{
		while(optind< argc)
		    {
		    if(AbstractApplication::stopping()) break;
		    char* input=argv[optind++];
		    igzstreambuf buf(input);
		    istream in(&buf);
		    this->run_goa(in);
		    buf.close();
		    }
		}
	    close();
	    return EXIT_SUCCESS;
	    }


	int main_grep(int argc,char** argv,int optind)
	    {
	    set<string> relationships;
	    set<string> terms;
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
		else if(strcmp(argv[optind],"-t")==0 && optind+1<argc)
		    {
		    terms.insert(argv[++optind]);
		    }
		else if(strcmp(argv[optind],"-v")==0)
		    {
		    this->inverse_result=true;
		    }
		else if(strcmp(argv[optind],"-r")==0 && optind+1<argc)
		    {
		    const char* s=(argv[++optind]);
		    if(!is_relation_ontology(s))
			{
			cerr << "Not in relation ontology: "<< s << endl;
			this->usage(cerr,argc,argv);
			return (EXIT_FAILURE);
			}
		    relationships.insert(s);
		    }
		else if(strcmp(argv[optind],"-c")==0 && optind+1<argc)
		    {
		    if(!numeric_cast<int32_t>(argv[++optind],&go_acn_column) || go_acn_column<1)
			{
			cerr << "Bad column index "<< argv[optind]<< endl;
			this->usage(cerr,argc,argv);
			return (EXIT_FAILURE);
			}
		    go_acn_column--;
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
	    if(go_acn_column<0)
		{
		cerr << "Undefined column."<< endl;
		this->usage(cerr,argc,argv);
		return (EXIT_FAILURE);
		}
	    if(terms.empty())
		{
		cerr << "Undefined go-terms."<< endl;
		this->usage(cerr,argc,argv);
		return (EXIT_FAILURE);
		}
	    if(relationships.empty())
		{
		relationships.insert("is_a");
		}



	    open(true);

	    set<string> result;
	    for(set<string>::iterator r=terms.begin();r!=terms.end();++r)
		{
		auto_ptr<set<string> > ret= this->find_descendants(r->c_str(),relationships);
		result.insert(ret->begin(),ret->end());
		}

	    if(optind==argc)
		{
		igzstreambuf buf;
		istream in(&buf);
		this->run_grep(in,result);
		}
	    else
		{
		while(optind< argc)
		    {
		    if(AbstractApplication::stopping()) break;
		    char* input=argv[optind++];
		    igzstreambuf buf(input);
		    istream in(&buf);
		    this->run_grep(in,result);
		    buf.close();
		    }
		}
	    close();
	    return EXIT_SUCCESS;
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
		return main_insertrdf(argc,argv,optind+1);
		}
	    else if(strcmp(argv[optind],"loadgoa")==0)
		{
		return main_insertgoa(argc,argv,optind+1);
		}
	    else if(strcmp(argv[optind],"desc")==0)
		{
		return main_descendants_or_ascendant(argc,argv,optind+1,true);
		}
	    else if(strcmp(argv[optind],"goa")==0)
		{
		return main_goa(argc,argv,optind+1);
		}
	    else if(strcmp(argv[optind],"grep")==0)
		{
		return main_grep(argc,argv,optind+1);
		}
	    else if(strcmp(argv[optind],"asc")==0)
		{
		return main_descendants_or_ascendant(argc,argv,optind+1,false);
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

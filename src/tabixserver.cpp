/*
 * faidxserver.cpp
 *
 *  Created on: Jan 24, 2012
 *      Author: lindenb
 */
#include <string>
#include <vector>
#include <map>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include "xtabix.h"
#include "auto_vector.h"
#include "segments.h"

using namespace std;
class Instance;

class Renderer
    {
    public:
	Instance* instance;
	virtual void begin(Instance* instance,const char* chrom,int32_t chromStart,int32_t chromEnd)=0;
	virtual void handle(const std::vector<std::string>& tokens,uint64_t nLine)=0;
	virtual void end()=0;
    };

class Named
    {
    public:
	std::string label;
	std::string description;
    };

class Column:public Named
    {
    public:
	std::string name;
	bool ignore;
	Column():ignore(false)
	    {
	    }
    };

class Table:public Named
    {
    public:
	std::string id;
	auto_vector<Column> columns;
	Tokenizer tokenizer;
	Column* chromColumn;
	Column* chromStartColumn;
	Column* chromEndColumn;

	Table():tokenizer('\t'),
		chromColumn(0),
		chromStartColumn(0),
		chromEndColumn(0)
	    {

	    }

	void scan(Tabix::Cursor* cursor,Renderer* renderer)
	    {
	    std::vector<std::string> tokens;
	    int len=0;
	    uint64_t nLine=0;
	    for(;;)
		{
		const char* line=cursor->next(&len);
		if(line==0) break;
		++nLine;
		tokenizer.split(line);
		renderer->handle(tokens,nLine);
		}
	    }
    };

class Instance:public Named
    {
    public:
	std::string id;
	Table* table;
	std::string path;
	void scan(const char* chrom,int32_t chromStart,int32_t chromEnd,Renderer* renderer)
	    {
	    if(table==0 || chrom==0 || chromStart>=chromEnd) return;
	    Tabix tabix(path.c_str(),true);
	    std::auto_ptr<Tabix::Cursor> c=tabix.cursor(chrom,chromStart,chromEnd);
	    renderer->begin(this,chrom,chromStart,chromEnd);
	    table->scan(c.get(),renderer);
	    renderer->end();
	    }
	void scan(const ChromStartEnd* position,Renderer* renderer)
	    {
	    scan(position->chrom.c_str(),position->start,position->end,renderer);
	    }
    };

class Model
    {
    private:
	void _label_and_desc(xmlNodePtr root,Named* named)
	    {
	    xmlChar* att=::xmlGetProp(root,BAD_CAST "label");
	    if(att==0) att=::xmlGetProp(root,BAD_CAST "title");
	    if(att==0) att=::xmlGetProp(root,BAD_CAST "name");
	    if(att==0) att=::xmlGetProp(root,BAD_CAST "id");
	    if(att!=0)
		 {
		 named->label.assign((const char*)att);
		 named->description.assign(table->label);
		 xmlFree(att);
		 }
	    att=::xmlGetProp(root,BAD_CAST "desc");
	    if(att==0) att=::xmlGetProp(root,BAD_CAST "description");
	    if(att==0) att=::xmlGetProp(root,BAD_CAST "comment");
	    if(att!=0)
		 {
		 named->description.assign(table->label);
		 xmlFree(att);
		 }
	    }
    public:
	auto_vector<Table> tables;
	auto_vector<Instance> instances;
	map<string,Instance*> id2instance;
	Model()
	    {

	    }


	void read(xmlDocPtr dom)
	    {
	    int32_t id_generator=0;
	    map<string,Table*> id2table;
	    xmlNodePtr root= xmlDocGetRootElement(dom);
	    //get the TABLES
	    for(xmlNodePtr c1 = xmlFirstElementChild(root); c1!=0; c1 = xmlNextElementSibling(c1))
		{
		if (c1->type != XML_ELEMENT_NODE) continue;
		if(!::xmlStrEqual(c1->name,BAD_CAST "table")) continue;
		xmlChar* att=::xmlGetProp(c1,BAD_CAST "id");
		if(att==0) THROW("no @id in <table>");
		Table* table=new Table;
		table->id.assign((const char*)att);
		xmlFree(att);

		_label_and_desc(c1,table);

		tables.push_back(table);
		id2table.insert(make_pair<string,Table*>(table->id,table));

		 for(xmlNodePtr c2 = xmlFirstElementChild(c1); c2!=0; c2 = xmlNextElementSibling(c2))
		    {
		    if (c2->type != XML_ELEMENT_NODE) continue;
		    if(!::xmlStrEqual(c2->name,BAD_CAST "column")) continue;
		    Column* column=new Column;
		    att=::xmlGetProp(c2,BAD_CAST "name");
		    if(att==0) THROW("no @name in table/column");
		    column->name.assign((const char*)att);
		    xmlFree(att);
		    table->columns.push_back(column);

		    att=::xmlGetProp(c2,BAD_CAST "ignore");
		    if(att!=0)
			{
			column->ignore=::xmlStrEqual(att,BAD_CAST "true");
			xmlFree(att);
			}

		    _label_and_desc(c2,column);
		    }
		}
	    //get the INSTANCE OF TABLES
	    for(xmlNodePtr c1 = xmlFirstElementChild(root); c1!=0; c1 = xmlNextElementSibling(c1))
		{
		if (c1->type != XML_ELEMENT_NODE) continue;
		if(!::xmlStrEqual(c1->name,BAD_CAST "instance")) continue;
		xmlChar* att=::xmlGetProp(c1,BAD_CAST "table-ref");
		if(att==0) att=::xmlGetProp(c1,BAD_CAST "table");
		if(att==0) THROW("no @table-ref in table/column");
		string ref((const char*)att);
		xmlFree(att);
		map<string,Table*>::iterator titer=id2table.find(ref);

		if(titer==id2table.end())
		    {
		    THROW("Cannot find table @id=" << ref);
		    }
		Instance* instance=new Instance;
		instance->table=(titer->second);

		xmlChar* att=::xmlGetProp(c1,BAD_CAST "id");
		if(att==0) att=::xmlGetProp(c1,BAD_CAST "name");
		if(att==0) THROW("no @id in instance");
		instance->id.assign((const char*)att);
		xmlFree(att);
		if(this->id2instance.find(instance->id)!=this->id2instance.end())
		    {
		    THROW("instance id "<< instance->id << " defined twice.");
		    }
		this->id2instance.insert(make_pair<string,Instance*>(instance->id,instance));
		this->instances.push_back(instance);
		 _label_and_desc(c1,instance);

		att=::xmlGetProp(c1,BAD_CAST "path");
		if(att==0)
		    {
		    THROW("Cannot find @path for instance");
		    }
		instance->path.assign((const char*)att);
		xmlFree(att);
		}
	    }

	void read(const char* configfile)
	    {
	    xmlDocPtr doc=::xmlParseFile(configfile);
	    if(doc==0)
		{
		  THROW("Cannot read config file=" << configfile);
		return;
		}
	    read(doc);
	    ::xmlFreeDoc(doc);
	    }
    };

class TabixServer
    {
    public:
	Model model;

	virtual void usage(ostream& out,int argc,char** argv)
	    {
	    out << argv[0] << " Pierre Lindenbaum PHD. 2012.\n";
	    out << VARKIT_REVISION << endl;
	    out << "Tabix files server.\n";
	    out << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
	    out << "Usage" << endl
		    << "   "<< argv[0]<< " [options] (file|stdin)"<< endl;
	    out << "Options:\n";
	    out << "  -c (xml-filename) path to XML config file." << endl;
	    out << "  -i (instance-id) ignore this instance-id (optional)." << endl;
	    out << "  -u (instance-id) always use this instance-id (optional)." << endl;
	    cerr << endl;
	    }


	virtual int main(int argc,char** argv)
	    {
	    set<string> ignore_instance_id;
	    set<string> only_instance_id;
	    char* xmlConfig=0;
	    int optind=1;
	    while(optind < argc)
		{
		if(std::strcmp(argv[optind],"-h")==0)
		    {
		    usage(cout,argc,argv);
		    return (EXIT_FAILURE);
		    }
		else if(std::strcmp(argv[optind],"-c")==0 && optind+1<argc)
		    {
		    xmlConfig=argv[++optind];
		    }
		else if(
			(std::strcmp(argv[optind],"-i")==0 ||
			 std::strcmp(argv[optind],"-u")==0
			) && optind+1<argc)
		    {
		    Tokenizer comma(',');
		    vector<string> tokens;
		    char which=argv[optind][1];
		    string line(argv[++optind]);
		    comma.split(line,tokens);
		    for(size_t i=0;i< tokens.size();++i)
			{
			if(tokens[i].empty()) continue;
			if(which=='i')
			    {
			    ignore_instance_id.insert(tokens[i]);
			    }
			else
			    {
			    only_instance_id.insert(tokens[i]);
			    }
			}
		    }
		else if(std::strcmp(argv[optind],"--")==0)
		    {
		    ++optind;
		    break;
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
	    if(!ignore_instance_id.empty() && !only_instance_id)
		{
		cerr << "Both 'ignore' and 'always' use instance id have been defined._n";
		usage(cerr,argc,argv);
		return (EXIT_FAILURE);
		}
	    if(xmlConfig==0)
		{
		cerr << "XML config file has not been defined."<< endl;
		usage(cerr,argc,argv);
		return (EXIT_FAILURE);
		}
	    model.read(xmlConfig);
	    if(optind==argc)
		{
		//just print the instances of tables
		//TODO
		}
	    else
		{

		}
	    return EXIT_SUCCESS;
	    }
    };


int main(int argc,char** argv)
    {
    TabixServer app;
    return app.main(argc,argv);
    }

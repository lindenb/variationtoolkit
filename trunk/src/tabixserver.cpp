/*
 * faidxserver.cpp
 *
 *  Created on: Jan 24, 2012
 *      Author: lindenb
 */
#include <string>
#include <vector>
#include <map>
#include <set>
#include <cstring>
#include <cstdlib>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include "xtabix.h"
#include "auto_vector.h"
#include "segments.h"
#include "xxml.h"
#include "throw.h"
#include "tokenizer.h"

using namespace std;
class Instance;

enum  RenderStatus
    {
    CURSOR_OK=0,
    CURSOR_BREAK=1
    };

class Renderer
    {
    public:
	std::ostream* out;
	Instance* instance;
	Renderer():out(&cout),instance(0)
	    {
	    }
	virtual ~Renderer()
	    {
	    if(out!=0) out->flush();
	    }
	virtual void startDocument()=0;
	virtual void endDocument()=0;
	virtual void startQuery(const char* chrom,int32_t chromStart,int32_t chromEnd)=0;
	virtual void endQuery()=0;
	virtual void startInstance(Instance* instance,const char* chrom,int32_t chromStart,int32_t chromEnd)=0;
	virtual RenderStatus handle(const std::vector<std::string>& tokens,uint64_t nLine)=0;
	virtual void endInstance()=0;
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
		//string s(line);
		tokenizer.split(line,tokens);
		if(renderer->handle(tokens,nLine)!=CURSOR_OK) break;
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
	    renderer->startInstance(this,chrom,chromStart,chromEnd);
	    table->scan(c.get(),renderer);
	    renderer->endInstance();
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
		 named->description.assign(named->label);
		 xmlFree(att);
		 }
	    att=::xmlGetProp(root,BAD_CAST "desc");
	    if(att==0) att=::xmlGetProp(root,BAD_CAST "description");
	    if(att==0) att=::xmlGetProp(root,BAD_CAST "comment");
	    if(att!=0)
		 {
		 named->description.assign(named->label);
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

		att=::xmlGetProp(c1,BAD_CAST "id");
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

class AbstractXMlRenderer:public Renderer
    {
    protected:
	xmlOutputBufferPtr buffer;
	xmlTextWriterPtr writer;
    public:
	AbstractXMlRenderer()
	    {
	    buffer= ::xmlOutputBufferCreateIOStream(&cout,0);
	    writer = ::xmlNewTextWriter(buffer);
	    }
	virtual ~AbstractXMlRenderer()
	    {
	    if(writer!=0) ::xmlFreeTextWriter(writer);
	    if(buffer!=0) ::xmlOutputBufferClose(buffer);
	    }
	virtual void startDocument()=0;
	virtual void endDocument()=0;
	virtual void startQuery(const char* chrom,int32_t chromStart,int32_t chromEnd)=0;
	virtual void endQuery()=0;
	virtual void startInstance(Instance* instance,const char* chrom,int32_t chromStart,int32_t chromEnd)=0;
	virtual RenderStatus handle(const std::vector<std::string>& tokens,uint64_t nLine)=0;
	virtual void endInstance()=0;
    };

class XMlRenderer:public AbstractXMlRenderer
    {
    public:
	XMlRenderer()
	    {
	    }

	virtual ~XMlRenderer()
	    {

	    }

	virtual void startDocument()
	    {
	    ::xmlTextWriterStartDocument(writer,0,0,0);
	    ::xmlTextWriterStartElement(writer,BAD_CAST "tabix");
	    }
	virtual void endDocument()
	    {
	    ::xmlTextWriterEndElement(writer);
	    ::xmlTextWriterFlush(writer);
	    out->flush();
	    }

	virtual void startQuery(const char* chrom,int32_t chromStart,int32_t chromEnd)
	    {
	    ::xmlTextWriterStartElement(writer,BAD_CAST "query");
	    ::xmlTextWriterWriteAttribute(writer,BAD_CAST "chrom",BAD_CAST chrom);
	    ::xmlTextWriterWriteAttr<int32_t>(writer,BAD_CAST "chromStart",chromStart);
	    ::xmlTextWriterWriteAttr<int32_t>(writer,BAD_CAST "chromEnd",chromEnd);
	    }
	virtual void endQuery()
	    {
	    ::xmlTextWriterEndElement(writer);
	    }
	virtual void startInstance(Instance* instance,const char* chrom,int32_t chromStart,int32_t chromEnd)
	    {
	    this->instance=instance;
	    ::xmlTextWriterStartElement(writer,BAD_CAST "table");
	    ::xmlTextWriterWriteAttribute(writer,BAD_CAST "type",BAD_CAST instance->id.c_str());
	    ::xmlTextWriterWriteAttribute(writer,BAD_CAST "label",BAD_CAST instance->label.c_str());
	    ::xmlTextWriterWriteAttribute(writer,BAD_CAST "description",BAD_CAST instance->description.c_str());
	    ::xmlTextWriterStartElement(writer,BAD_CAST "head");
	    for(size_t i=0;i< instance->table->columns.size();++i)
		{
		Column* col= instance->table->columns.at(i);
		if(col->ignore) continue;
		::xmlTextWriterStartElement(writer,BAD_CAST "column");
		 ::xmlTextWriterWriteAttr<string>(writer,BAD_CAST "id",col->name);
		 ::xmlTextWriterWriteAttr<string>(writer,BAD_CAST "label",col->label);
		 ::xmlTextWriterWriteText<string>(writer,col->description);
		::xmlTextWriterEndElement(writer);//head
		}
	    ::xmlTextWriterEndElement(writer);//head
	    ::xmlTextWriterStartElement(writer,BAD_CAST "body");
	    }
	virtual RenderStatus handle(const std::vector<std::string>& tokens,uint64_t nLine)
	    {
	    ::xmlTextWriterStartElement(writer,BAD_CAST instance->id.c_str());
	    ::xmlTextWriterWriteAttr<uint64_t>(writer,BAD_CAST "index",nLine);
	    for(size_t i=0;i< this->instance->table->columns.size();++i)
		{
		Column* col= instance->table->columns.at(i);
		if(col->ignore) continue;
		::xmlTextWriterStartElement(writer,BAD_CAST col->name.c_str());
		if(i<tokens.size())
		    {
		    ::xmlTextWriterWriteText<string>(writer,tokens[i]);
		    }
		::xmlTextWriterEndElement(writer);
		}
	    ::xmlTextWriterEndElement(writer);//tr
	    return CURSOR_OK;
	    }
	virtual void endInstance()
	    {
	    ::xmlTextWriterEndElement(writer);//body
	    ::xmlTextWriterEndElement(writer);
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


	auto_ptr<Renderer> createRenderer()
	    {
	    auto_ptr<Renderer> render(new XMlRenderer);
	    return render;
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
	    if(!ignore_instance_id.empty() && !only_instance_id.empty())
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
		cout << "#id\tlabel\tdescription\n";
		for(size_t i=0;
		    i< model.instances.size();
		    ++i)
		    {
		    Instance* instance=model.instances.at(i);
		    cout << instance->id
			    << "\t"
			    << instance->label
			    << "\t"
			    << instance->description
			    << endl;
		    }
		return EXIT_SUCCESS;
		}

	    auto_ptr<Renderer> renderer= createRenderer();
	    renderer->startDocument();
	    while(optind<argc)
		{
		extern std::auto_ptr<std::vector<ChromStartEnd> > parseSegments(const char* s);

		char* arg=argv[optind++];
		auto_ptr<vector<ChromStartEnd> > segs=parseSegments(arg);
		if(segs.get()==0)
		    {
		    cerr << "Bad segment:"<< arg << endl;
		    return (EXIT_FAILURE);
		    }
		for(size_t j=0;j< segs->size();++j)
		    {
		    const ChromStartEnd& segment=segs->at(j);
		    renderer->startQuery(segment.chrom.c_str(),segment.start,segment.end);

		    for(size_t i=0;
			i< model.instances.size();
			++i)
			{
			Instance* instance=model.instances.at(i);
			if(!ignore_instance_id.empty() && ignore_instance_id.find(instance->id)!=ignore_instance_id.end())
			    {
			    continue;
			    }
			if(!only_instance_id.empty() && only_instance_id.find(instance->id)==only_instance_id.end())
			    {
			    continue;
			    }

			instance->scan(&segment,renderer.get());
			}
		    }
		renderer->endQuery();
		}
	    renderer->endDocument();
	    return EXIT_SUCCESS;
	    }
    };


int main(int argc,char** argv)
    {
    TabixServer app;
    return app.main(argc,argv);
    }

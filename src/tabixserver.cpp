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
#include "xmlescape.h"
#ifdef CGI_VERSION
#include "cgi.h"
#endif

using namespace std;
class Instance;

#define XSD_PREFIX BAD_CAST "xsd"
#define XSD_NS BAD_CAST "http://www.w3.org/2001/XMLSchema"

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
	virtual void comment(std::string s)
	    {

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

	void schema(xmlTextWriterPtr w)
	    {
	    ::xmlTextWriterStartElementNS(w,XSD_PREFIX, BAD_CAST "element", XSD_NS);
	    ::xmlTextWriterWriteAttribute(w,BAD_CAST "name", BAD_CAST name.c_str());
	    ::xmlTextWriterWriteAttribute(w,BAD_CAST "type", BAD_CAST "xsd:string");
	    ::xmlTextWriterEndElement(w);
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

	string xsdType() const
	    {
	    string tt("TableDef");
	    tt.append(this->id);
	    return tt;
	    }

	void schema(xmlTextWriterPtr w)
	    {
	    string type(xsdType());
	    ::xmlTextWriterStartElementNS(w,XSD_PREFIX, BAD_CAST "complexType", XSD_NS);
	    ::xmlTextWriterWriteAttribute(w,BAD_CAST "name", BAD_CAST type.c_str());
	    ::xmlTextWriterStartElementNS(w,XSD_PREFIX, BAD_CAST "complexContent", XSD_NS);
	    ::xmlTextWriterStartElementNS(w,XSD_PREFIX, BAD_CAST "extension", XSD_NS);
	    ::xmlTextWriterWriteAttribute(w,BAD_CAST "base", BAD_CAST "AbstractChromStartEnd");
	    ::xmlTextWriterStartElementNS(w,XSD_PREFIX, BAD_CAST "sequence", XSD_NS);
	    size_t i=0;
	    for(i=0;i< this->columns.size();++i)
		{
		Column* c=this->columns.at(i);
		if(c->ignore) continue;
		c->schema(w);
		}

	    ::xmlTextWriterEndElement(w);
	    ::xmlTextWriterEndElement(w);
	    ::xmlTextWriterEndElement(w);
	    ::xmlTextWriterEndElement(w);
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
	void schema(xmlTextWriterPtr w)
	    {
	    string t(table->xsdType());
	    ::xmlTextWriterStartElementNS(w,XSD_PREFIX, BAD_CAST "element", XSD_NS);
	    ::xmlTextWriterWriteAttribute(w,BAD_CAST "name", BAD_CAST id.c_str());
	    ::xmlTextWriterWriteAttribute(w,BAD_CAST "type", BAD_CAST t.c_str());
	    ::xmlTextWriterEndElement(w);
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
	    if(root==0 || !::xmlStrEqual(root->name,BAD_CAST "config")) THROW("root!=config");
	    //get the TABLES
	    for(xmlNodePtr c1 = root->children; c1!=0; c1 = c1->next)
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


		for(xmlNodePtr c2 = c1->children; c2!=0; c2 = c2->next)
		    {
		    if (c2->type != XML_ELEMENT_NODE) continue;
		    if(!::xmlStrEqual(c2->name,BAD_CAST "column")) continue;
		    Column* column=new Column;
		    att=::xmlGetProp(c2,BAD_CAST "name");
		    if(att==0) THROW("no @name in table/column");
		    column->name.assign((const char*)att);
		    xmlFree(att);
		    table->columns.push_back(column);

		    if(column->name.compare("chrom")==0 && table->chromColumn==0)
			{
			table->chromColumn=column;
			}
		    else if(column->name.compare("chromStart") && table->chromStartColumn==0)
			{
			table->chromStartColumn=column;
			}
		    else if(column->name.compare("chromEnd") && table->chromEndColumn==0)
			{
			table->chromEndColumn=column;
			}
		    else if(column->name.compare("txStart") && table->chromStartColumn==0)
			{
			table->chromStartColumn=column;
			}
		    else if(column->name.compare("txEnd") && table->chromEndColumn==0)
			{
			table->chromEndColumn=column;
			}


		    att=::xmlGetProp(c2,BAD_CAST "ignore");
		    if(att!=0)
			{
			column->ignore=::xmlStrEqual(att,BAD_CAST "true");
			xmlFree(att);
			}

		    _label_and_desc(c2,column);
		    }
		if(table->chromColumn==0)   THROW("Cannot chrom column for table " << table->id);
		if(table->chromStartColumn==0)   THROW("Cannot chromStart column for table " << table->id);
		if(table->chromEndColumn==0)   THROW("Cannot chromEnd column for table " << table->id);
		}
	    //get the INSTANCE OF TABLES
	    for(xmlNodePtr c1 = root->children; c1!=0; c1 = c1->next)
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

#define DEFINE_ATT(name,type)   ::xmlTextWriterStartElementNS(w,XSD_PREFIX, BAD_CAST "attribute", XSD_NS);\
	::xmlTextWriterWriteAttribute(w,BAD_CAST "name", BAD_CAST name);\
	::xmlTextWriterWriteAttribute(w,BAD_CAST "type", BAD_CAST type);\
	::xmlTextWriterEndElement(w)

	void schema(std::ostream& out)
	    {
	    xmlOutputBufferPtr buffer=::xmlOutputBufferCreateIOStream(&cout,0);
	    xmlTextWriterPtr w=::xmlNewTextWriter(buffer);
	    ::xmlTextWriterStartDocument(w,0,0,0);
	    ::xmlTextWriterStartElementNS(w,XSD_PREFIX, BAD_CAST "schema", XSD_NS);


	    ::xmlTextWriterStartElementNS(w,XSD_PREFIX, BAD_CAST "complexType", XSD_NS);
	    ::xmlTextWriterWriteAttribute(w,BAD_CAST "name", BAD_CAST "AbstractChromStartEnd");
	    DEFINE_ATT("chrom","xsd:string");
	    DEFINE_ATT("start","xsd:int");
	    DEFINE_ATT("end","xsd:int");
	    ::xmlTextWriterEndElement(w);

	    for(size_t i=0;i< tables.size();++i)
		{
		Table* t=tables.at(i);
		t->schema(w);
		}
	    for(size_t i=0;i< instances.size();++i)
		{
		Instance* instance=instances.at(i);
		instance->schema(w);
		}
	    ::xmlTextWriterEndElement(w);
	    ::xmlTextWriterEndDocument(w);
	    ::xmlFreeTextWriter(w);
	    ::xmlOutputBufferClose(buffer);
	    out.flush();
	    }
    };


class TextRenderer:public Renderer
    {
    private:
	int n_printed;
    public:
	TextRenderer():n_printed(0)
	    {

	    }
	virtual ~TextRenderer()
	    {
	    cout.flush();
	    }
	virtual void startDocument()
	    {

	    }
	virtual void endDocument()
	    {

	    }
	virtual void startQuery(const char* chrom,int32_t chromStart,int32_t chromEnd)
	    {

	    }
	virtual void endQuery()
	    {

	    }
	virtual void startInstance(Instance* instance,const char* chrom,int32_t chromStart,int32_t chromEnd)
	    {
	    if(n_printed>0) cout << "\\\\" << endl;
	    cout << "##instance.label="<< instance->label << endl;
	    cout << "##instance.desc="<< instance->description << endl;
	    cout << "##region="<<chrom << ":" << chromStart<<"-" << chromEnd << endl;
	    cout << "#";
	    bool found=false;
	    for(size_t i=0;i< instance->table->columns.size();++i)
	    		{
	    		Column* col= instance->table->columns.at(i);
	    		if(col->ignore) continue;
	    		if(found) cout << "\t";
	    		found=true;
	    		cout << col->label;
	    		}
	    cout << endl;
	    }
	virtual RenderStatus handle(const std::vector<std::string>& tokens,uint64_t nLine)
	    {
	    bool found=false;
	    for(size_t i=0;i< this->instance->table->columns.size();++i)
		{
		Column* col= instance->table->columns.at(i);
		if(col->ignore) continue;
		if(found) cout << "\t";
		if(i>=tokens.size()) continue;
		found=true;
		cout << tokens[i];
		}
	    return CURSOR_OK;
	    }
	virtual void endInstance()
	    {
	    n_printed++;
	    }
	virtual void comment(const std::string s)
	    {

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
	virtual void comment(const std::string s)
	    {
	    ::xmlTextWriterWriteComment(writer,BAD_CAST s.c_str());
	    }
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
	    ::xmlTextWriterEndDocument(writer);
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


class HtmlRenderer:public AbstractXMlRenderer
    {
    public:
	HtmlRenderer()
	    {
	    }

	virtual ~HtmlRenderer()
	    {

	    }

	virtual void startDocument()
	    {
	    //::xmlTextWriterStartDocument(writer,0,0,0);
	    ::xmlTextWriterStartElement(writer,BAD_CAST "html");
	    ::xmlTextWriterStartElement(writer,BAD_CAST "body");
	    }
	virtual void endDocument()
	    {
	    ::xmlTextWriterEndElement(writer);//body
	    ::xmlTextWriterEndElement(writer);//html
	    ::xmlTextWriterFlush(writer);
	    out->flush();
	    }

	virtual void startQuery(const char* chrom,int32_t chromStart,int32_t chromEnd)
	    {
	    ::xmlTextWriterStartElement(writer,BAD_CAST "div");
	    ::xmlTextWriterWriteAttribute(writer,BAD_CAST "class",BAD_CAST "query");


	    ostringstream os;
	    os << chrom << ":" <<chromStart << "-" << chromEnd;
	    ::xmlTextWriterStartElement(writer,BAD_CAST "h2");
	    ::xmlTextWriterWriteText<string>(writer,os.str());
	    ::xmlTextWriterEndElement(writer);//h2
	    }
	virtual void endQuery()
	    {
	    ::xmlTextWriterEndElement(writer);//div
	    ::xmlTextWriterEndElement(writer);
	    }
	virtual void startInstance(Instance* instance,const char* chrom,int32_t chromStart,int32_t chromEnd)
	    {
	    this->instance=instance;
	    ::xmlTextWriterStartElement(writer,BAD_CAST "div");
	    ::xmlTextWriterWriteAttribute(writer,BAD_CAST "class",BAD_CAST "instance");

	    ::xmlTextWriterStartElement(writer,BAD_CAST "h3");
	    ::xmlTextWriterWriteText<string>(writer,instance->label);
	    ::xmlTextWriterEndElement(writer);//h3


	    ::xmlTextWriterStartElement(writer,BAD_CAST "p");
	    ::xmlTextWriterWriteText<string>(writer,instance->description);
	    ::xmlTextWriterEndElement(writer);


	    ::xmlTextWriterStartElement(writer,BAD_CAST "table");
	    ::xmlTextWriterStartElement(writer,BAD_CAST "thead");
	    ::xmlTextWriterStartElement(writer,BAD_CAST "tr");



	    for(size_t i=0;i< instance->table->columns.size();++i)
		{
		Column* col= instance->table->columns.at(i);
		if(col->ignore) continue;
		::xmlTextWriterStartElement(writer,BAD_CAST "th");
		 ::xmlTextWriterWriteText<string>(writer,col->label);
		::xmlTextWriterEndElement(writer);//th
		}
	    ::xmlTextWriterEndElement(writer);//tr
	    ::xmlTextWriterEndElement(writer);//thead
	    ::xmlTextWriterStartElement(writer,BAD_CAST "tbody");
	    }
	virtual RenderStatus handle(const std::vector<std::string>& tokens,uint64_t nLine)
	    {
	    ::xmlTextWriterStartElement(writer,BAD_CAST "tr");
	    for(size_t i=0;i< this->instance->table->columns.size();++i)
		{
		Column* col= instance->table->columns.at(i);
		if(col->ignore) continue;
		::xmlTextWriterStartElement(writer,BAD_CAST "td");
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
	    ::xmlTextWriterEndElement(writer);//tbody
	    ::xmlTextWriterEndElement(writer);//table
	    ::xmlTextWriterEndElement(writer);//div
	    }

    };

class TabixServer
    {
    public:
	Model model;

#ifdef CGI_VERSION
	CGI cgi;
	streambuf* stdbuf;;
	cgistreambuf* cgibuff;
	std::string title;

	TabixServer():stdbuf(cout.rdbuf()),cgibuff(0),title("Tabix server")
	    {
	    cgibuff=new cgistreambuf(stdbuf);
	    cout.rdbuf(cgibuff);
	    }

	~TabixServer()
	    {
	    cout.flush();
	    cout.rdbuf(stdbuf);
	    delete cgibuff;
	    }




#else
	std::string xml_path;
	std::string format;
	TabixServer():format("xml")
	    {

	    }
	~TabixServer()
	    {

	    }
#endif

	xmlDocPtr load_project_file()
		{
#ifndef CGI_VERSION
	    const char* project_xml=xml_path.c_str();
#else
	    char* project_xml=getenv("TABIX_SERVER_PATH");
#endif

		if(project_xml==NULL)
		    {
		    quit(0,0,"Cannot get $TABIX_SERVER_PATH.");
		    }
		xmlDocPtr doc=::xmlParseFile(project_xml);
		if(doc==0)
		    {
		    quit(0,0,"Cannot load xml TABIX_SERVER_PATH.");
		    }
		return doc;
		}

	std::string script_name()
	    {
	    string name;
	    char* s=getenv("SCRIPT_NAME");
	    if(s!=0) name.assign(s);
	    return name;
	    }

#ifndef CGI_VERSION
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
#endif




	auto_ptr<Renderer> createRenderer()
	    {
#ifdef CGI_VERSION
	    string format("html");

	    const char* fmt=cgi.getParameter("format");
	    if(fmt!=0)
		{
		format.assign(fmt);
		//string s(fmt);
		//s.append("_ICI_");
		//cgi.setParameter("format",s.c_str());
		}

#endif
	    auto_ptr<Renderer> render(0);
	    if(format.compare("xml")==0)
		{
		render.reset(new XMlRenderer);
#ifndef CGI_VERSION
		cgibuff->setContentType("text/xml");
#endif

		}
	    else if(format.compare("html")==0 || format.compare("xhtml")==0)
		{
		render.reset(new HtmlRenderer);
#ifndef CGI_VERSION
		cgibuff->setContentType("text/html");
#endif
		}
	    else if(format.compare("txt")==0 || format.compare("text")==0)
		{
		render.reset(new TextRenderer);
#ifndef CGI_VERSION
		cgibuff->setContentType("text/plain");
#endif
		}
	    else
		{
		render.reset(new XMlRenderer);
#ifndef CGI_VERSION
		cgibuff->setContentType("text/xml");
#endif

		}
	    return render;
	    }

#ifdef CGI_VERSION

	void header()
	    {
	    cgibuff->setContentType("text/html");
	    //cout << "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\" \"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\">\n";
	    cout << "<html xmlns='http://www.w3.org/1999/xhtml' xml:lang='en'><head>"
		<< "<title>" << xmlEscape(title) << "</title>"
		<< "<style type='text/css'>"
		<< "label { text-align:right; margin:5px;}\n"
			"dl {padding: 0.5em;}\n"
			"dt {float: left; clear: left; width: 190px; text-align: right; font-weight: bold; color:darkgray;}\n"
			"dt:after { content: \":\"; }\n"
			"dd { margin: 0 0 0 200px; padding: 0 0 0.5em 0; }\n"
			"button {font-size:200%;min-width:100px;border: 1px solid; background-image:-moz-linear-gradient( top, gray, lightgray );margin:5px;}\n"
			"button:hover {background-image:-moz-linear-gradient( top, lightgray, gray );}\n"
			".code {font-family:monospace;font-size:14pt;color:white;background-color:black;max-width:100%;max-height:400px;overflow:auto;padding:20px;}\n"
			"p.desc { border-style:solid; border-width:1px; border-radius: 5px; background-color:lightgray;padding:20px;margin:20px;color:black;}\n"
			".bigtitle {text-align:center;padding:10px;text-shadow: 3px 3px 4px gray; font-size:200%;}\n"
		<< "</style>"
		<< "</head><body>";
	    }

	void footer()
	    {
	    cout << "<hr/><div style='font-size:50%;'>";
	    cout << "<a href='http://plindenbaum.blogspot.com'>Pierre Lindenbaum PhD</a><br/>";
	    cout << "Last compilation " << __DATE__ << " at " << __TIME__ << ".<br/>";
	    //cout << "<pre>"; cgi.dump(cout); cout << "</pre>";
	    cout << "</div></body></html>";
	    cout.flush();
	    }

	void loadModel()
	    {
	    xmlDocPtr dom=load_project_file();
	    model.read(dom);
	    xmlFreeDoc(dom);
	    }

	void print_main()
	    {
	    loadModel();
	    header();



	    cout << "<div>";
	    cout << "<form method='GET' action='" << script_name() << "'>";
	    cout << "<input type='hidden' name='action' value='tabix.show'/>";
	    /* region box */
	    cout << "<div style='text-align:center;font-size:200%; margin:50px;'>";
	    cout << "<label for='query'>Position: </label>";
	    cout << "<input type='text' name='q' value='' id='query' style='padding:10px;font-size:100%;' title='position' placeholder='chrom:start-end' required='true'/>";

	    cout << " <select name='format' style='padding:10px;font-size:100%;'>"
		    "<option>xml</option>"
		    "<option>html</option>"
		    "<option>txt</option>"
		    "</select>";
	    cout << "</div>";
	    /* list instances in a <DL/> list */
	    cout << "<dl>";
	    for(map<string,Instance*>::iterator r=model.id2instance.begin();
		 r!=model.id2instance.end();
		 ++r)
		{
		cout << "<dt>";
		cout << "<input type='checkbox' name='t' value='" <<  xmlEscape(r->first) << "'/>";
		cout << "<label >"<<  xmlEscape(r->second->label) << "</label>";
		cout << "</dt>";

		cout << "<dd>";
		cout <<  xmlEscape(r->second->description);
		cout << "</dd>";
		}
	    cout << "</dl>";
	    cout << "</form>";
	    cout << "</div>";
	    footer();
	    }

	void quit(const char* mime,int status,const char* message)
	    {
	    cgibuff->setContentType(mime==0?"text/plain":mime);
	    cgibuff->setStatus(status==0?SC_BAD_REQUEST:status);
	    cout << (message==NULL?"error":message) << "\n";
	    cout.flush();
	    exit(EXIT_SUCCESS);
	    }

	void run()
	    {
	    loadModel();
	    std::set<std::string> queries= cgi.getParameters("q");
	    auto_ptr<Renderer> renderer= createRenderer();
	    renderer->startDocument();

	    if(cgi.contains("format"))
		{
		string s(cgi.getParameter("format"));
		s.append("XXXX");
		renderer->comment(s);
		}
	    for(
		set<std::string>::iterator r=queries.begin();
		r!=queries.end();
		++r
		)
		{
		extern std::auto_ptr<std::vector<ChromStartEnd> > parseSegments(const char* s);

		if(r->empty()) continue;
		auto_ptr<vector<ChromStartEnd> > segs=parseSegments(r->c_str());
		if(segs.get()==0)
		    {
		    renderer->comment("No segments");
		    continue;
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

			if(!cgi.contains("t",instance->id))
			    {
			    continue;
			    }

			instance->scan(&segment,renderer.get());
			}
		    }
		renderer->endQuery();
		}
	    renderer->endDocument();
	    }

	void schema()
	    {
	    loadModel();
	    cgibuff->setContentType("text/xml");
	    model.schema(cout);
	    }

	int main(int argc,char** argv)
	    {
	    try
		{
		if(
		   !cgi.parse() ||
		   !cgi.contains("action") ||
		   cgi.contains("action","home"))
		    {
		    print_main();
		    }
		else if(cgi.contains("action","tabix.show"))
		    {
		    run();
		    }
		else if(cgi.contains("action","schema"))
		    {
		    schema();
		    }
		else
		    {
		    print_main();
		    }
		}
	    catch(exception& err)
		{
		quit(0,0,err.what());
		}
	    return EXIT_SUCCESS;
	    }
#else

	void quit(const char* mime,int status,const char* message)
		{
		cout << (message==NULL?"error":message) << "\n";
		cout.flush();
		exit(EXIT_FAILURE);
		}

	int main(int argc,char** argv)
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
#endif
    };


int main(int argc,char** argv)
    {
    TabixServer app;
    return app.main(argc,argv);
    }

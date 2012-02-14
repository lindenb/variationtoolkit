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
#include "cescape.h"
#include "cgi.h"
#include "numeric_cast.h"
#define NOWHERE
#include "where.h"

using namespace std;
class Instance;

#define XSD_PREFIX  "xsd"
#define XSD_NS  "http://www.w3.org/2001/XMLSchema"

#define XSI_PREFIX  "xsi"
#define XSI_NS  "http://www.w3.org/2001/XMLSchema-instance"

#define SOAP_PREFIX  "soap"
#define SOAP_NS  "http://schemas.xmlsoap.org/wsdl/soap/"

#define WSDL_PREFIX  "wsdl"
#define WSDL_NS  "http://schemas.xmlsoap.org/wsdl/"


/***************************************************************************************/
/***************************************************************************************/
/***************************************************************************************/


/** primitive type */
enum ColumnDataType
    {
    TYPE_STRING,
    TYPE_BOOL,
    TYPE_INT,
    TYPE_LONG,
    TYPE_DOUBLE
    };

/* after scanning a tabix file, should we break a loop ? */
enum  RenderStatus
    {
    CURSOR_OK=0,
    CURSOR_BREAK=1
    };

/***************************************************************************************/
/***************************************************************************************/
/***************************************************************************************/

/** abstract tabix renderer */
class Renderer
    {
    public:
	/** output stream */
	std::ostream* out;
	/** owner instance */
	Instance* instance;
	/** limit to n_rows, default infinite */
	long limit_rows;
    protected:
	/** current number of rows */
	long count_rows;
	Renderer():out(&cout),instance(0),limit_rows(-1L),count_rows(0L)
	    {
	    }
    public:
	virtual ~Renderer()
	    {
	    if(out!=0) out->flush();
	    }
	/** write a comment */
	virtual void comment(std::string s)
	    {

	    }
	/** start rendering */
	virtual void startDocument()=0;
	/** end rendering */
	virtual void endDocument()=0;
	virtual void startQuery(const char* chrom,int32_t chromStart,int32_t chromEnd)=0;
	virtual void endQuery()=0;
	virtual void startInstance(Instance* instance,const char* chrom,int32_t chromStart,int32_t chromEnd)=0;
	/** handle a new row of a tabix file */
	virtual RenderStatus handle(const std::vector<std::string>& tokens,uint64_t nLine)=0;
	virtual void endInstance()=0;

	/** shortcut, should we stop the loop */
	RenderStatus _hasNext()
		    {
		    ++count_rows;
		    if(limit_rows!=-1L && count_rows>=limit_rows) return CURSOR_BREAK;
		    return CURSOR_OK;
		    }
    };
/***************************************************************************************/
/***************************************************************************************/
/***************************************************************************************/


/** abstract class, something having a label and a description */
class Named
    {
    public:
	std::string label;
	std::string description;

	/** print as xsd:annotation */
	void xsdAnnotation(XmlStreamWriter* w)
		{
		w->writeStartElementNS(XSD_PREFIX, "annotation", XSD_NS);
		w->writeStartElementNS(XSD_PREFIX, "documentation", XSD_NS);
		w->writeString(description);
		w->writeEndElement();
		w->writeEndElement();
		}

    };

/***************************************************************************************/
/***************************************************************************************/
/***************************************************************************************/

/** a column in a tabix file */
class Column:public Named
    {
    public:
	/** column id */
	std::string name;
	/** shall we ignore this column in the output: default false */
	bool ignore;
	/** primitive type: default string */
	ColumnDataType dataType;
	/** index in table */
	int32_t columnIndex;

	/** constructor */
	Column():ignore(false),dataType(TYPE_STRING),columnIndex(-1)
	    {
	    }

	/** write XSD:schema for this column */
	void schema(XmlStreamWriter* w)
	    {
	    std::string xsd_type;
	    switch(dataType)
		{
		case TYPE_BOOL: xsd_type.assign("xsd:boolean"); break;
		case TYPE_LONG: xsd_type.assign("xsd:long"); break;
		case TYPE_INT: xsd_type.assign("xsd:int"); break;
		case TYPE_DOUBLE: xsd_type.assign("xsd:double"); break;
		default: xsd_type.assign("xsd:string"); break;
		}
	    w->writeStartElementNS(XSD_PREFIX,  "element", XSD_NS);
	    w->writeAttribute( "name",  name.c_str());
	    w->writeAttribute( "type",  xsd_type.c_str());

	    if(!description.empty() && description.compare(name)!=0)
		{
		xsdAnnotation(w);
		}
	    w->writeEndElement();//xsd::element
	    }

    };

/***************************************************************************************/
/***************************************************************************************/
/***************************************************************************************/

/** a Table defining a tabix file */
class Table:public Named
    {
    public:
	/** id for this table */
	std::string id;
	/** all the columns for this table */
	auto_vector<Column> columns;
	/** line splitter */
	Tokenizer tokenizer;
	/** chromosome column */
	Column* chromColumn;
	/* chromosome start column */
	Column* chromStartColumn;
	/* chromosome end column */
	Column* chromEndColumn;

	Table():tokenizer('\t'),
		chromColumn(0),
		chromStartColumn(0),
		chromEndColumn(0)
	    {

	    }

	/** number of columns */
	int32_t size() const
	    {
	    return (int32_t)columns.size();
	    }
	/** get column by index */
	Column* at(int32_t index)
	    {
	    return columns.at(index);
	    }

	/* scan this tabix file with this cursor and renderer */
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

	/** returns a unique name for xml Schema */
	string xsdType() const
	    {
	    string tt("TableDef");
	    tt.append(this->id);
	    return tt;
	    }

	/** print this table as a xsd::complexType */
	void schema(XmlStreamWriter* w)
	    {
	    string type(xsdType());
	    w->writeStartElementNS(XSD_PREFIX,  "complexType", XSD_NS);
	    w->writeAttribute( "name",  type.c_str());


	    if(!description.empty() && description.compare(id)!=0)
		{
		xsdAnnotation(w);
		}

	    w->writeStartElementNS(XSD_PREFIX,  "complexContent", XSD_NS);
	    w->writeStartElementNS(XSD_PREFIX,  "extension", XSD_NS);
	    w->writeAttribute( "base",  "AbstractChromStartEndType");
	    w->writeStartElementNS(XSD_PREFIX,  "sequence", XSD_NS);
	    int i=0;
	    for(i=0;i< size();++i)
		{
		Column* c=at(i);
		if(c->ignore) continue;
		c->schema(w);
		}

	    w->writeEndElement();
	    w->writeEndElement();
	    w->writeEndElement();
	    w->writeEndElement();
	    }

    };
/***************************************************************************************/
/***************************************************************************************/
/***************************************************************************************/


/** an instance of table 'Table */
class Instance:public Named
    {
    public:
	/** uniq id for this instance */
	std::string id;
	/** associated table */
	Table* table;
	/** file path to tabix file */
	std::string path;

	/** number of columns */
	int32_t size()
	    {
	    return table->size();
	    }

	/** get column by index */
	Column* at(int32_t i)
	    {
	    return table->at(i);
	    }

	/** print this instance using the given renderer */
	void scan(const char* chrom,int32_t chromStart,int32_t chromEnd,Renderer* renderer)
	    {
	    if(table==0 || chrom==0 || chromStart>=chromEnd) return;
	    Tabix tabix(path.c_str(),true);
	    std::auto_ptr<Tabix::Cursor> c=tabix.cursor(chrom,chromStart,chromEnd);
	    renderer->startInstance(this,chrom,chromStart,chromEnd);
	    table->scan(c.get(),renderer);
	    renderer->endInstance();
	    }
	/** print this instance using the given renderer */
	void scan(const ChromStartEnd* position,Renderer* renderer)
	    {
	    scan(position->chrom.c_str(),position->start,position->end,renderer);
	    }
	/** xsd schema for this instance */
	void schema(XmlStreamWriter* w)
	    {
	    string t(table->xsdType());
	    w->writeStartElementNS(XSD_PREFIX,  "element", XSD_NS);
	    w->writeAttribute( "name",  id.c_str());
	    w->writeAttribute( "type",  t.c_str());
	    w->writeAttribute( "substitutionGroup",  "AbstractChromStartEnd");

	    if(!description.empty() && description.compare(id)!=0)
		{
		xsdAnnotation(w);
	   	}

	    w->writeEndElement();
	    }

	void schemaArrayOf(XmlStreamWriter* w)
	    {
	    string t(table->xsdType());
	    string element("ArrayOf");
	    element.append(this->id);
	    w->writeStartElementNS(XSD_PREFIX, "element", XSD_NS);
	    w->writeAttr("name",element);
	    w->writeStartElementNS(XSD_PREFIX,"complexType",XSD_NS);
	    w->writeStartElementNS(XSD_PREFIX,"sequence",XSD_NS);
	    w->writeStartElementNS(XSD_PREFIX,"element",XSD_NS);
	    w->writeAttr("type",this->id);
	    w->writeAttr("maxOccurs","unbounded");
	    w->writeAttr("minOccurs",0);
	    w->writeEndElement();
	    w->writeEndElement();
	    w->writeEndElement();
	    w->writeEndElement();
	    }
    };

/***************************************************************************************/
/***************************************************************************************/
/***************************************************************************************/


/** model of instance and tables */
class Model
    {
    private:
	/** parse label and description */
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
	/** all tables */
	auto_vector<Table> tables;
	/** all instances */
	auto_vector<Instance> instances;
	/** map instance id to instance */
	map<string,Instance*> id2instance;

	/* constructor */
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

		/* add  this table */
		this->tables.push_back(table);
		/* insert id2tabme */
		id2table.insert(make_pair<string,Table*>(table->id,table));

		/** parse column */
		for(xmlNodePtr c2 = c1->children; c2!=0; c2 = c2->next)
		    {
		    if (c2->type != XML_ELEMENT_NODE) continue;
		    if(!::xmlStrEqual(c2->name,BAD_CAST "column")) continue;
		    /* create new column */
		    Column* column=new Column;

		    /* column name */
		    att=::xmlGetProp(c2,BAD_CAST "name");
		    if(att==0) att=::xmlGetProp(c2,BAD_CAST "id");
		    if(att==0) THROW("no @name in table/column");
		    column->name.assign((const char*)att);
		    xmlFree(att);

		    /* add this column to the current table */
		    column->columnIndex= table->size();
		    table->columns.push_back(column);

		    /* is this column chrom/start/end ? */
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
		    else if(column->name.compare("POS") && table->chromStartColumn==0 && table->chromEndColumn==0)
			{
			table->chromStartColumn=column;
			table->chromEndColumn=column;
			}
		    else if(column->name.compare("CHROM")==0 && table->chromColumn==0)
			{
			table->chromColumn=column;
			}

		    /** ignore flag */
		    att=::xmlGetProp(c2,BAD_CAST "ignore");
		    if(att!=0)
			{
			column->ignore=::xmlStrEqual(att,BAD_CAST "true");
			xmlFree(att);
			}

		    /** data type for this column */
		    att=::xmlGetProp(c2,BAD_CAST "dataType");
		    if(att!=0)
			{
			if(::xmlStrEqual(att,BAD_CAST "xsd:int") || ::xmlStrEqual(att,BAD_CAST "int"))
			    {
			    column->dataType=TYPE_INT;
			    }
			else if(::xmlStrEqual(att,BAD_CAST "xsd:long") || ::xmlStrEqual(att,BAD_CAST "long"))
			    {
			    column->dataType=TYPE_LONG;
			    }
			else if(::xmlStrEqual(att,BAD_CAST "xsd:float") || ::xmlStrEqual(att,BAD_CAST "float") ||
				::xmlStrEqual(att,BAD_CAST "xsd:double") || ::xmlStrEqual(att,BAD_CAST "double"))
			    {
			    column->dataType=TYPE_DOUBLE;
			    }
			else if(::xmlStrEqual(att,BAD_CAST "xsd:bool"))
			    {
			    column->dataType=TYPE_BOOL;
			    }
			else if(::xmlStrEqual(att,BAD_CAST "xsd:string") || ::xmlStrEqual(att,BAD_CAST "string"))
			    {
			    column->dataType=TYPE_STRING;
			    }
			xmlFree(att);
			}
		    /* parse documentation */
		    _label_and_desc(c2,column);
		    }
		/* check we found chrom/start/end */
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
		/*instance id */
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
		/* parse documentation */
		 _label_and_desc(c1,instance);

		att=::xmlGetProp(c1,BAD_CAST "path");
		if(att==0) att=::xmlGetProp(c1,BAD_CAST "file");
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

#define DEFINE_ATT(name,type)   w->writeStartElementNS(XSD_PREFIX, BAD_CAST "attribute", XSD_NS);\
	w->writeAttribute(BAD_CAST "name", BAD_CAST name);\
	w->writeAttribute(BAD_CAST "type", BAD_CAST type);\
	w->writeEndElement()

	/** write XML schema for this model */
	void schema(std::ostream& out)
	    {
	    xmlOutputBufferPtr buffer=::xmlOutputBufferCreateIOStream(&cout,0);
	    xmlTextWriterPtr xtw=::xmlNewTextWriter(buffer);
	    XmlStreamWriter* w=new XmlStreamWriter(xtw);
	    w->writeStartDocument();
	    w->writeStartElementNS(XSD_PREFIX, "schema", XSD_NS);


	    extern void _schema_fragment1(xmlTextWriterPtr w);

	    _schema_fragment1(w->getDelegate());


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
	    w->writeEndElement();
	    w->writeEndDocument();

	    ::xmlFreeTextWriter(xtw);

	    out.flush();
	    delete w;
	    }


	/** write WDSL for the schame */
	void wsdl(std::ostream& out)
	    {
	    xmlOutputBufferPtr buffer=::xmlOutputBufferCreateIOStream(&cout,0);
	    xmlTextWriterPtr xtw=::xmlNewTextWriter(buffer);
	    XmlStreamWriter w(xtw);
	    w.writeStartDocument();

	    w.writeStartElementNS(WSDL_PREFIX, "definitions", WSDL_NS);
	   // w.writeAttr("xmlns:xsd",XSD_NS);
	    w.writeAttr("xmlns:wsdl",WSDL_NS);

	    //types
	    w.writeStartElementNS(WSDL_PREFIX, "types", WSDL_NS);

	    w.writeEndElement();//types

	    w.writeEndElement();

	    w.writeEndDocument();

	    ::xmlFreeTextWriter(xtw);

	    out.flush();
	    }

    };



/***************************************************************************************/
/***************************************************************************************/
/***************************************************************************************/

/* text renderer */
class TextRenderer:public Renderer
    {
    private:
	/** flag: shall we print a delimiter */
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
	/** print header */
	virtual void startInstance(Instance* instance,const char* chrom,int32_t chromStart,int32_t chromEnd)
	    {
	    this->instance=instance;
	    this->count_rows=0L;
	    if(n_printed>0) cout << "\\\\" << endl;
	    ++n_printed;
	    cout << "##instance.id="<< instance->id << endl;
	    cout << "##instance.label="<< instance->label << endl;
	    cout << "##instance.desc="<< instance->description << endl;
	    cout << "##region="<<chrom << ":" << chromStart<<"-" << chromEnd << endl;
	    cout << "#";
	    bool found=false;
	    /** print header */
	    for(int32_t i=0;i< instance->size();++i)
	    		{
	    		Column* col= instance->at(i);
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
	    for(int32_t i=0;i< this->instance->size();++i)
		{
		Column* col= instance->at(i);
		if(col->ignore) continue;
		if(found) cout << "\t";
		if(i>=(int32_t)tokens.size()) continue;
		found=true;
		cout << tokens[i];
		}
	    cout << endl;
	    return _hasNext();
	    }

	virtual void endInstance()
	    {
	    n_printed++;
	    }
	virtual void comment(const std::string s)
	    {

	    }
    };

/***************************************************************************************/
/***************************************************************************************/
/***************************************************************************************/

class TextCountRenderer:public Renderer
    {
    private:
	string query;
    public:
	TextCountRenderer()
	    {

	    }
	virtual ~TextCountRenderer()
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
	    ostringstream os;
	    os << chrom << ":" << chromStart << "-" << chromEnd;
	    query.assign(os.str());
	    }
	virtual void endQuery()
	    {

	    }
	virtual void startInstance(Instance* instance,const char* chrom,int32_t chromStart,int32_t chromEnd)
	    {
	    this->instance=instance;
	    this->count_rows=0L;
	    cout << query << "\t" << instance->id << "\t";
	    }
	virtual RenderStatus handle(const std::vector<std::string>& tokens,uint64_t nLine)
	    {
	    return _hasNext();
	    }

	virtual void endInstance()
	    {
	    cout << this->count_rows << endl;
	    }
    };

/***************************************************************************************/
/***************************************************************************************/
/***************************************************************************************/

class JSONRenderer:public Renderer
    {
    private:
	int n_queries;
	int n_instances;
    public:
	JSONRenderer():n_queries(0),n_instances(0)
	    {

	    }
	virtual ~JSONRenderer()
	    {
	    cout.flush();
	    }
	virtual void startDocument()
	    {
	    cout << "{\"queries\":[";
	    }
	virtual void endDocument()
	    {
	    cout << "]}\n";
	    }
	virtual void startQuery(const char* chrom,int32_t chromStart,int32_t chromEnd)
	    {
	    if(n_queries>0) cout << ",";
	    n_queries++;
	    n_instances=0;
	    this->count_rows=0L;
	    cout << "{\"chrom\":\""<<  CEscape(chrom) << "\","
		      "\"chromStart\":"<< chromStart << ","
		      "\"chromEnd\":"<< chromEnd << ","
		      "\"tables\":["
		      ;
	    }
	virtual void endQuery()
	    {
	    cout << "]}";
	    }
	virtual void startInstance(Instance* instance,const char* chrom,int32_t chromStart,int32_t chromEnd)
	    {
	    this->instance=instance;
	    if(n_instances>0) cout << ",";
	    n_instances++;
	    this->count_rows=0L;
	    cout << "{\"id\":\""<<  CEscape(instance->id) << "\","
	      "\"label\":\""<< CEscape(instance->label) << "\","
	      "\"desc\":\""<< CEscape(instance->description) << "\","
	      "\"columns\":["
	      ;
	    bool found=false;
	    for(size_t i=0;i< this->instance->table->columns.size();++i)
		{
		Column* col= instance->table->columns.at(i);
		if(col->ignore) continue;
		if(found) cout << ",";
		found=true;
		cout <<"{\"name\":\"" << CEscape(col->name) << "\",";
		cout <<"\"label\":\"" << CEscape(col->label) << "\",";
		cout <<"\"desc\":\"" << CEscape(col->description) << "\",";
		cout <<"\"type\":\"";
		switch(col->dataType)
		    {
		    case TYPE_BOOL: cout << "xsd:bool"; break;
		    case TYPE_DOUBLE: cout << "xsd:double"; break;
		    case TYPE_LONG: cout << "xsd:long"; break;
		    case TYPE_INT: cout << "xsd:int"; break;
		    default: cout << "xsd:string"; break;
		    }
		cout << "\"}";
		}

	    cout << "],\"rows\":["
	      ;
	    }


	virtual void endInstance()
	    {
	    cout << "]}";
	    }

	virtual RenderStatus handle(const std::vector<std::string>& tokens,uint64_t nLine)
	    {
	    if(this->count_rows>0) cout << ",";
	    cout << "{";
	    bool found=false;
	    for(size_t i=0;i< this->instance->table->columns.size();++i)
		{
		Column* col= instance->table->columns.at(i);
		if(col->ignore) continue;
		if(i>=tokens.size()) continue;
		if(found) cout << ",";
		found=true;
		cout << "\"";
		cout << CEscape(col->name);
		cout << "\":";
		const char* s_= tokens[i].c_str();
		if(strcasecmp(s_,"NULL")==0 || strcasecmp(s_,"NIL")==0 || strcasecmp(s_,"N/A")==0)
		    {
		    cout << "null";
		    continue;
		    }
		switch(col->dataType)
		    {
		    case TYPE_BOOL:
			{
			if(strcasecmp(s_,"yes")==0 ||
			   strcasecmp(s_,"true")==0 ||
			   strcasecmp(s_,"1")==0 ||
			   strcasecmp(s_,"T")==0 ||
			   strcasecmp(s_,"Y")==0)
			    {
			    cout << "true";
			    }
			else if(strcasecmp(s_,"no")==0 ||
			   strcasecmp(s_,"false")==0 ||
			   strcasecmp(s_,"0")==0 ||
			   strcasecmp(s_,"F")==0 ||
			   strcasecmp(s_,"N")==0)
			    {
			    cout << "false";
			    }
			else
			    {
			    cout << "\""<< CEscape(tokens[i])<<"\"";
			    }
			break;
			}
		    case TYPE_DOUBLE:
			{
			double v;
			if(numeric_cast(s_,&v))
			    {
			    cout << tokens[i];
			    }
			else
			    {
			    cout << "\""<< CEscape(tokens[i])<<"\"";
			    }
			break;
			}
		    case TYPE_INT:
			{
			int v;
			if(numeric_cast(s_,&v))
			    {
			    cout << tokens[i];
			    }
			else
			    {
			    cout << "\""<< CEscape(tokens[i])<<"\"";
			    }
			break;
			}
		    case TYPE_LONG:
			{
			long v;
			if(numeric_cast(s_,&v))
			    {
			    cout << tokens[i];
			    }
			else
			    {
			    cout << "\""<< CEscape(tokens[i])<<"\"";
			    }
			break;
			}
		    default:
			{
			cout << "\""<< CEscape(tokens[i])<<"\"";
			break;
			}
		    }
		}
	    cout << "}";
	    return _hasNext();
	    }

	virtual void comment(const std::string s)
	    {

	    }
    };
/***************************************************************************************/
/***************************************************************************************/
/***************************************************************************************/

class JSONCountRenderer:public Renderer
    {
    private:
	int n_queries;
	int n_instances;
    public:
	JSONCountRenderer():n_queries(0),n_instances(0)
	    {

	    }
	virtual ~JSONCountRenderer()
	    {
	    cout.flush();
	    }
	virtual void startDocument()
	    {
	    cout << "{\"queries\":[";
	    }
	virtual void endDocument()
	    {
	    cout << "]}\n";
	    }
	virtual void startQuery(const char* chrom,int32_t chromStart,int32_t chromEnd)
	    {
	    if(n_queries>0) cout << ",";
	    n_queries++;
	    n_instances=0;
	    this->count_rows=0L;
	    cout << "{\"chrom\":\""<<  CEscape(chrom) << "\","
		      "\"chromStart\":"<< chromStart << ","
		      "\"chromEnd\":"<< chromEnd << ","
		      "\"tables\":["
		      ;
	    }
	virtual void endQuery()
	    {
	    cout << "]}";
	    }
	virtual void startInstance(Instance* instance,const char* chrom,int32_t chromStart,int32_t chromEnd)
	    {
	    this->instance=instance;
	    if(n_instances>0) cout << ",";
	    n_instances++;
	    this->count_rows=0L;
	    cout << "{\"id\":\""<<  CEscape(instance->id) << "\","
	      "\"label\":\""<< CEscape(instance->label) << "\","
	      "\"desc\":\""<< CEscape(instance->description) << "\","
	      "\"count\":"
	      ;
	    }


	virtual void endInstance()
	    {
	    cout << count_rows << "}";
	    }

	virtual RenderStatus handle(const std::vector<std::string>& tokens,uint64_t nLine)
	    {

	    return _hasNext();
	    }

	virtual void comment(const std::string s)
	    {

	    }
    };

/***************************************************************************************/
/***************************************************************************************/
/***************************************************************************************/


class AbstractXMlRenderer:public Renderer
    {
    protected:
	xmlOutputBufferPtr buffer;
	xmlTextWriterPtr writer;
	XmlStreamWriter* sw;
    public:
	AbstractXMlRenderer()
	    {
	    buffer= ::xmlOutputBufferCreateIOStream(&cout,0);
	    writer = ::xmlNewTextWriter(buffer);
	    sw=new XmlStreamWriter(writer);
	    }
	virtual ~AbstractXMlRenderer()
	    {
	    if(sw!=0) delete sw;
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
	    sw->writeComment( s.c_str());
	    }
    };

/***************************************************************************************/
/***************************************************************************************/
/***************************************************************************************/


class XMlRenderer:public AbstractXMlRenderer
    {
    public:
	std::string xsd_url;
	XMlRenderer()
	    {
	    }

	virtual ~XMlRenderer()
	    {

	    }

	virtual void startDocument()
	    {
	    ::xmlTextWriterStartDocument(writer,0,0,0);
	    sw->writeStartElement( "tabix");
	    if(!xsd_url.empty())
		{
		sw->writeAttribute( "xmlns:xsi",XSI_NS);
		sw->writeAttribute( "xsi:schemaLocation", xsd_url.c_str());
		}
	    }

	virtual void endDocument()
	    {
	    sw->writeEndElement();
	    sw->writeEndDocument();
	    ::xmlTextWriterFlush(writer);
	    out->flush();
	    }

	virtual void startQuery(const char* chrom,int32_t chromStart,int32_t chromEnd)
	    {
	    sw->writeStartElement( "query");
	    sw->writeAttr("chrom", chrom);
	    sw->writeAttr("chromStart",chromStart);
	    sw->writeAttr("chromEnd",chromEnd);
	    }
	virtual void endQuery()
	    {
	    sw->writeEndElement();
	    }
	virtual void startInstance(Instance* instance,const char* chrom,int32_t chromStart,int32_t chromEnd)
	    {
	    this->instance=instance;
	    this->count_rows=0L;
	    sw->writeStartElement( "table");
	    sw->writeAttribute( "type", instance->id.c_str());
	    sw->writeAttribute( "label", instance->label.c_str());
	    sw->writeAttribute( "description", instance->description.c_str());
	    sw->writeStartElement( "head");
	    for(size_t i=0;i< instance->table->columns.size();++i)
		{
		Column* col= instance->table->columns.at(i);
		if(col->ignore) continue;
		sw->writeStartElement( "column");
		 sw->writeAttr( "id",col->name);
		 sw->writeAttr( "label",col->label);
		 ::xmlTextWriterWriteText<string>(writer,col->description);
		sw->writeEndElement();//head
		}
	    sw->writeEndElement();//head
	    sw->writeStartElement( "body");
	    }
	virtual RenderStatus handle(const std::vector<std::string>& tokens,uint64_t nLine)
	    {
	    sw->writeStartElement( instance->id.c_str());
	    sw->writeAttr( "index",nLine);
	    sw->writeAttr( "chrom",
		tokens[this->instance->table->chromColumn->columnIndex]
		);
	    sw->writeAttr( "chromStart",
		tokens[this->instance->table->chromStartColumn->columnIndex]
		);
	    sw->writeAttr( "chromEnd",
		tokens[this->instance->table->chromEndColumn->columnIndex]
		);


	    for(int32_t i=0;i< this->instance->size();++i)
		{
		Column* col= instance->at(i);
		if(col->ignore) continue;
		sw->writeStartElement( col->name.c_str());
		if(i< (int32_t)tokens.size())
		    {
		    ::xmlTextWriterWriteText<string>(writer,tokens[i]);
		    }
		sw->writeEndElement();
		}
	    sw->writeEndElement();//tr
	    return _hasNext();
	    }
	virtual void endInstance()
	    {
	    sw->writeEndElement();//body
	    sw->writeEndElement();
	    }

    };


/***************************************************************************************/
/***************************************************************************************/
/***************************************************************************************/

class XMlCountRenderer:public AbstractXMlRenderer
    {
    public:
	std::string xsd_url;
	XMlCountRenderer()
	    {
	    }

	virtual ~XMlCountRenderer()
	    {

	    }

	virtual void startDocument()
	    {
	    ::xmlTextWriterStartDocument(writer,0,0,0);
	    sw->writeStartElement( "tabix");
	    if(!xsd_url.empty())
		{
		sw->writeAttribute( "xmlns:xsi",XSI_NS);
		sw->writeAttribute( "xsi:schemaLocation", xsd_url.c_str());
		}
	    }
	virtual void endDocument()
	    {

	    sw->writeEndElement();
	    sw->writeEndDocument();
	    ::xmlTextWriterFlush(writer);
	    out->flush();
	    }

	virtual void startQuery(const char* chrom,int32_t chromStart,int32_t chromEnd)
	    {
	    sw->writeStartElement( "query");
	    sw->writeAttribute( "chrom", chrom);
	    sw->writeAttr( "chromStart",chromStart);
	    sw->writeAttr( "chromEnd",chromEnd);
	    }
	virtual void endQuery()
	    {
	    sw->writeEndElement();
	    }
	virtual void startInstance(Instance* instance,const char* chrom,int32_t chromStart,int32_t chromEnd)
	    {
	    this->instance=instance;
	    this->count_rows=0L;
	    sw->writeStartElement( "table");
	    sw->writeAttribute( "type", instance->id.c_str());
	    sw->writeAttribute( "label", instance->label.c_str());
	    sw->writeAttribute( "description", instance->description.c_str());

	    }
	virtual RenderStatus handle(const std::vector<std::string>& tokens,uint64_t nLine)
	    {
	    return _hasNext();
	    }
	virtual void endInstance()
	    {
	    sw->writeAttr( "count",this->count_rows);
	    sw->writeEndElement();
	    }

    };
/***************************************************************************************/
/***************************************************************************************/
/***************************************************************************************/


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
	    sw->writeStartElement( "html");
	    sw->writeStartElement( "body");
	    }
	virtual void endDocument()
	    {
	    sw->writeEndElement();//body
	    sw->writeEndElement();//html
	    ::xmlTextWriterFlush(writer);
	    out->flush();
	    }

	virtual void startQuery(const char* chrom,int32_t chromStart,int32_t chromEnd)
	    {
	    sw->writeStartElement( "div");
	    sw->writeAttribute( "class", "query");


	    ostringstream os;
	    os << chrom << ":" <<chromStart << "-" << chromEnd;
	    sw->writeStartElement( "h2");
	    ::xmlTextWriterWriteText<string>(writer,os.str());
	    sw->writeEndElement();//h2
	    }
	virtual void endQuery()
	    {
	    sw->writeEndElement();//div
	    sw->writeEndElement();
	    }
	virtual void startInstance(Instance* instance,const char* chrom,int32_t chromStart,int32_t chromEnd)
	    {
	    this->instance=instance;
	    this->count_rows=0L;
	    sw->writeStartElement( "div");
	    sw->writeAttribute( "class", "instance");

	    sw->writeStartElement( "h3");
	    ::xmlTextWriterWriteText<string>(writer,instance->label);
	    sw->writeEndElement();//h3


	    sw->writeStartElement( "p");
	    ::xmlTextWriterWriteText<string>(writer,instance->description);
	    sw->writeEndElement();


	    sw->writeStartElement( "table");
	    sw->writeStartElement( "thead");
	    sw->writeStartElement( "tr");



	    for(size_t i=0;i< instance->table->columns.size();++i)
		{
		Column* col= instance->table->columns.at(i);
		if(col->ignore) continue;
		sw->writeStartElement( "th");
		 ::xmlTextWriterWriteText<string>(writer,col->label);
		sw->writeEndElement();//th
		}
	    sw->writeEndElement();//tr
	    sw->writeEndElement();//thead
	    sw->writeStartElement( "tbody");
	    }
	virtual RenderStatus handle(const std::vector<std::string>& tokens,uint64_t nLine)
	    {
	    sw->writeStartElement( "tr");
	    for(size_t i=0;i< this->instance->table->columns.size();++i)
		{
		Column* col= instance->table->columns.at(i);
		if(col->ignore) continue;
		sw->writeStartElement( "td");
		if(i<tokens.size())
		    {
		    ::xmlTextWriterWriteText<string>(writer,tokens[i]);
		    }
		sw->writeEndElement();
		}
	    sw->writeEndElement();//tr
	    return _hasNext();
	    }
	virtual void endInstance()
	    {
	    sw->writeEndElement();//tbody
	    sw->writeEndElement();//table
	    sw->writeEndElement();//div
	    }

    };

/***************************************************************************************/
/***************************************************************************************/
/***************************************************************************************/


class HtmlCountRenderer:public AbstractXMlRenderer
    {
    public:
	HtmlCountRenderer()
	    {
	    }

	virtual ~HtmlCountRenderer()
	    {

	    }

	virtual void startDocument()
	    {
	    //::xmlTextWriterStartDocument(writer,0,0,0);
	    sw->writeStartElement( "html");
	    sw->writeStartElement( "body");
	    }
	virtual void endDocument()
	    {
	    sw->writeEndElement();//body
	    sw->writeEndElement();//html
	    ::xmlTextWriterFlush(writer);
	    out->flush();
	    }

	virtual void startQuery(const char* chrom,int32_t chromStart,int32_t chromEnd)
	    {
	    sw->writeStartElement("div");


	    ostringstream os;
	    os << chrom << ":" <<chromStart << "-" << chromEnd;




	    sw->writeStartElement("table");

	    sw->writeStartElement( "caption");
	    sw->writeString(os.str());
	    sw->writeEndElement();

	    sw->writeStartElement( "thead");
	    sw->writeStartElement( "tr");

	    sw->writeStartElement( "th");
	    sw->writeString("Table");
	    sw->writeEndElement();

	    sw->writeStartElement( "th");
	    sw->writeString("Label");
	    sw->writeEndElement();

	    sw->writeStartElement( "th");
	    sw->writeString("Description");
	    sw->writeEndElement();

	    sw->writeStartElement( "th");
	    sw->writeString("Count");
	    sw->writeEndElement();


	    sw->writeEndElement();//tr
	    sw->writeEndElement();//thead

	    sw->writeStartElement( "tbody");

	    }
	virtual void endQuery()
	    {
	    sw->writeEndElement();//tbody
	    sw->writeEndElement();//table
	    sw->writeEndElement();//div
	    }
	virtual void startInstance(Instance* instance,const char* chrom,int32_t chromStart,int32_t chromEnd)
	    {
	    this->instance=instance;
	    this->count_rows=0L;
	    sw->writeStartElement("tr");


	    sw->writeStartElement( "td");
	    sw->writeString(instance->id);
	    sw->writeEndElement();//td

	    sw->writeStartElement( "td");
	    sw->writeString(instance->label);
	    sw->writeEndElement();//td

	    sw->writeStartElement( "td");
	    sw->writeString(instance->description);
	    sw->writeEndElement();//td

	    }
	virtual RenderStatus handle(const std::vector<std::string>& tokens,uint64_t nLine)
	    {
	    return _hasNext();
	    }
	virtual void endInstance()
	    {
	    sw->writeStartElement("td");
	    sw->writeString(this->count_rows);
	    sw->writeEndElement();//td
	    sw->writeEndElement();//tr
	    }

    };

/***************************************************************************************/
/***************************************************************************************/
/***************************************************************************************/

/* 'the' tabix server */
class TabixServer
    {
    public:
	/** associated model*/
	Model model;
	/** cgi parser */
	CGI cgi;
	/** original streambuf */
	streambuf* stdbuf;
	/** cgi streambuf */
	cgistreambuf* cgibuff;
	/** html title */
	std::string title;

	/** constructor */
	TabixServer():stdbuf(cout.rdbuf()),cgibuff(0),title("Tabix server")
	    {
	    cgibuff=new cgistreambuf(stdbuf);
	    cout.rdbuf(cgibuff);
	    }

	/* destructor */
	~TabixServer()
	    {
	    cout.flush();
	    cout.rdbuf(stdbuf);
	    delete cgibuff;
	    }



        /** load XML containing the model */
	xmlDocPtr load_project_file()
	   {
	   char* project_xml=getenv("TABIX_SERVER_PATH");

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

	/** get the script name or empty string */
	std::string script_name()
	    {
	    string name;
	    char* s=getenv("SCRIPT_NAME");
	    if(s!=0) name.assign(s);
	    return name;
	    }



	/* create the renderer for the request parameter 'format' */
	auto_ptr<Renderer> createRenderer()
	    {
	    string format("html");

	    const char* fmt=cgi.getParameter("format");
	    if(fmt!=0)
		{
		format.assign(fmt);
		//string s(fmt);
		//s.append("_ICI_");
		//cgi.setParameter("format",s.c_str());
		}
	    bool count=false;
	    const char* count_str=cgi.getParameter("count");
	    if(count_str!=0)
		{
		count=(strcmp(count_str,"yes")==0);
		}

	    auto_ptr<Renderer> render(0);
	    if(format.compare("xml")==0)
		{
		char* server_name=getenv("SERVER_NAME");
		if(count)
		    {
		    XMlCountRenderer*x=new XMlCountRenderer;
		    if(server_name!=0)  x->xsd_url.append("http://").append(server_name);
		    x->xsd_url.append(script_name()).append("?action=schema");
		    render.reset(x);
		    }
		else
		    {
		    XMlRenderer*x=new XMlRenderer;
		    if(server_name!=0)  x->xsd_url.append("http://").append(server_name);
		    x->xsd_url.append(script_name()).append("?action=schema");
		    render.reset(x);
		    }

		cgibuff->setContentType("text/xml");
		}
	    else if(format.compare("html")==0 || format.compare("xhtml")==0)
		{
		if(count)
		    {
		    render.reset(new HtmlCountRenderer);
		    }
		else
		    {
		    render.reset(new HtmlRenderer);
		    }
		cgibuff->setContentType("text/html");
		}
	    else if(format.compare("txt")==0 || format.compare("text")==0)
		{
		if(count)
		    {
		    render.reset(new TextCountRenderer);
		    }
		else
		    {
		    render.reset(new TextRenderer);
		    }
		cgibuff->setContentType("text/plain");
		}
	    else if(format.compare("json")==0 || format.compare("js")==0)
		{
		if(count)
		    {
		    render.reset(new JSONCountRenderer);
		    }
		else
		    {
		    render.reset(new JSONRenderer);
		    }
		cgibuff->setContentType("application/json");
		}
	    else
		{
		if(count)
		    {
		    render.reset(new XMlCountRenderer);
		    }
		else
		    {
		    render.reset(new XMlRenderer);
		    }
		cgibuff->setContentType("text/xml");
		}
	    return render;
	    }


	/* print HTML header */
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

	/** print HTML FOOTER */
	void footer()
	    {
	    cout << "<hr/><div style='font-size:50%;'>";
	    cout << "<a href='http://plindenbaum.blogspot.com'>Pierre Lindenbaum PhD</a><br/>";
	    cout << "Last compilation " << __DATE__ << " at " << __TIME__ << ".<br/>";
	    //cout << "<pre>"; cgi.dump(cout); cout << "</pre>";
	    cout << "</div></body></html>";
	    cout.flush();
	    }

	/** load and parse the Model */
	void loadModel()
	    {
	    xmlDocPtr dom=load_project_file();
	    model.read(dom);
	    xmlFreeDoc(dom);
	    }

	/** print MAIN HTML PAGE */
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
		    "<option>json</option>"
		    "</select>";
	    cout << "<input type='submit' style='padding:10px;font-size:100%;'>";
	    cout << "</div>";
	    cout << "<div>Options: "
		    "<label for='limitrows'>Limit:</label> "
		    "<input type='number' id='limitrows' name='limit' value='' type='number' min='1' placeholder=\"stop after 'N' rows\"/>"
		    " "
		    "<input type='checkbox' id='justcount' value='yes'  name='count'/> <label for='justcount'>Count</label>"
		    "</div>"
		    ;


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

	/** problem, quit */
	void quit(const char* mime,int status,const char* message)
	    {
	    cgibuff->setContentType(mime==0?"text/plain":mime);
	    cgibuff->setStatus(status==0?SC_BAD_REQUEST:status);
	    cout << (message==NULL?"error":message) << "\n";
	    cout.flush();
	    exit(EXIT_SUCCESS);
	    }

	/** run the tabix query */
	void run()
	    {
	    loadModel();
	    std::set<std::string> queries= cgi.getParameters("q");
	    auto_ptr<Renderer> renderer= createRenderer();
	    renderer->startDocument();
	    if(cgi.contains("limit"))
		{
		long limit;
		if(numeric_cast(cgi.getParameter("limit"),&limit) && limit>0)
		    {
		    renderer->limit_rows=limit;
		    }
		}

	    /* loop over the queries */
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
			/* this table is not used */
			if(!cgi.contains("t",instance->id))
			    {
			    //skip
			    continue;
			    }

			instance->scan(&segment,renderer.get());
			}
		    }
		renderer->endQuery();
		}
	    renderer->endDocument();
	    }

	/* print the schema */
	void schema()
	    {
	    loadModel();
	    cgibuff->setContentType("text/xml");
	    model.schema(cout);
	    }

	/* print the wsdl */
	void wsdl()
	    {
	    loadModel();
	    cgibuff->setContentType("text/xml");
	    model.wsdl(cout);
	    }


	/* main loop */
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
		else if(cgi.contains("action","wsdl"))
		    {
		    wsdl();
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
    };


int main(int argc,char** argv)
    {
    TabixServer app;
    return app.main(argc,argv);
    }

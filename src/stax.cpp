/*
 * stax.cpp
 *
 *  Created on: Jan 10, 2012
 *      Author: lindenb
 */
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlreader.h>
#include "stax.h"
#include "throw.h"
#include "where.h"
using namespace std;

#define CAST_READER(a) ((xmlTextReaderPtr)((a)->reader))


StreamXmlReader::QName::QName(const char* namespaceURI,const char* localPart):_prefix("")
    {
    if(namespaceURI==0) THROW("namespaceURI is NULL");
    this->_namespaceURI.assign(namespaceURI);
    if(localPart==0) THROW("LocalPart is NULL");
    this->_localPart.assign(localPart);
    }
StreamXmlReader::QName::QName(const char* namespaceURI,const char* localPart,const char* prefix)
    {
    if(namespaceURI==0) THROW("namespaceURI is NULL");
    this->_namespaceURI.assign(namespaceURI);
    if(localPart==0) THROW("LocalPart is NULL");
    this->_localPart.assign(localPart);
    if(prefix==0) THROW("Prefix is NULL");
    this->_prefix.assign(localPart);
    }
StreamXmlReader::QName::QName(const char* localPart):
	_namespaceURI(""),
	_prefix("")

    {
    if(localPart==0) THROW("LocalPart is NULL");
    this->_localPart.assign(localPart);
    }

StreamXmlReader::QName::QName(const StreamXmlReader::QName& cp):
	_namespaceURI(cp._namespaceURI),
	_prefix(cp._prefix),
	_localPart(cp._localPart)
    {
    }


StreamXmlReader::QName::~QName()
    {
    }


bool StreamXmlReader::QName::operator==(const StreamXmlReader::QName& cp) const
    {
    return  _namespaceURI.compare(cp._namespaceURI)==0 &&
	    _prefix.compare(cp._prefix)==0 &&
	    _localPart.compare(cp._localPart)==0
	    ;
    }

bool StreamXmlReader::QName::operator<(const StreamXmlReader::QName& cp) const
    {
    int i= _namespaceURI.compare(cp._namespaceURI);
    if(i!=0) return i<0;
    i= _prefix.compare(cp._prefix);
    if(i!=0) return i<0;
    i=_localPart.compare(cp._localPart);
    return i<0;
    }

StreamXmlReader::QName& StreamXmlReader::QName::operator=(const StreamXmlReader::QName& cp)
    {
    if(this!=&cp)
	{
	_namespaceURI.assign(cp._namespaceURI);
	_prefix.assign(cp._prefix);
	_localPart.assign(cp._localPart);
	}
    return *this;
    }

const char* StreamXmlReader::QName::namespaceURI() const
    {
    return _namespaceURI.c_str();
    }
const char* StreamXmlReader::QName::prefix() const
    {
    return _prefix.c_str();
    }
const char* StreamXmlReader::QName::localPart() const
    {
    return _localPart.c_str();
    }
/**************************************************************************************/
/**************************************************************************************/
/**************************************************************************************/


StreamXmlReader::Attribute::Attribute(const StreamXmlReader::QName name,const char* value):_name(name),_value(value)
    {
    }

StreamXmlReader::Attribute::Attribute(const StreamXmlReader::Attribute& cp):_name(cp._name),_value(cp._value)
    {
    }


StreamXmlReader::Attribute::~Attribute()
    {
    }


bool StreamXmlReader::Attribute::operator==(const StreamXmlReader::Attribute& cp) const
    {
    return  _name==cp._name && _value.compare(cp._value)==0;
    }

bool StreamXmlReader::Attribute::operator<(const StreamXmlReader::Attribute& cp) const
    {
    if(_name<cp._name) return true;
    if(cp._name<_name) return false;
    return _value.compare(cp._value)< 0;
    }

StreamXmlReader::Attribute&
StreamXmlReader::Attribute::operator=(const StreamXmlReader::Attribute& cp)
    {
    if(this!=&cp)
	{
	_name=cp._name;
	_value.assign(cp._value);
	}
    return *this;
    }

const StreamXmlReader::QName& StreamXmlReader::Attribute::name() const
    {
    return this->_name;
    }
const char* StreamXmlReader::Attribute::value() const
    {
    return this->_value.c_str();
    }

const char* StreamXmlReader::Attribute::namespaceURI() const
    {
    return _name.namespaceURI();
    }
const char* StreamXmlReader::Attribute::prefix() const
    {
    return _name.prefix();
    }
const char* StreamXmlReader::Attribute::localPart() const
    {
    return _name.localPart();
    }




/**************************************************************************************/
/**************************************************************************************/
/**************************************************************************************/

StreamXmlReader::Attributes::Attributes()
    {

    }
StreamXmlReader::Attributes::~Attributes()
    {

    }

uint64_t StreamXmlReader::Attributes::size() const
    {
    return _atts.size();
    }

const StreamXmlReader::Attribute& StreamXmlReader::Attributes::item(uint64_t i) const
    {
    return _atts.at(i);
    }

const char*
StreamXmlReader::Attributes::get( StreamXmlReader::QName qName) const
    {
    for(uint64_t i=0;i< _atts.size();++i)
	{
	if(qName==_atts[i].name()) return _atts[i].value();
	}
    return 0;
    }


/**************************************************************************************/
/**************************************************************************************/
/**************************************************************************************/

StreamXmlReader::Event::Event()
    {
    }

StreamXmlReader::Event::~Event()
    {
    }

bool StreamXmlReader::Event::isStartElement() const
    {
    return type()==StreamXmlReader::START_ELEMENT;
    }
bool StreamXmlReader::Event::isEndElement() const
    {
    return type()==StreamXmlReader::END_ELEMENT;
    }
bool StreamXmlReader::Event::isText() const
    {
    return type()==StreamXmlReader::TEXT;
    }

/**************************************************************************************/
/**************************************************************************************/
/**************************************************************************************/


StreamXmlReader::ElementEvt::ElementEvt():_name(0)
    {
    }

StreamXmlReader::ElementEvt::~ElementEvt()
    {
    if(_name!=0) delete _name;
    }

const StreamXmlReader::QName& StreamXmlReader::ElementEvt::name() const
    {
    return *(_name);
    }


StreamXmlReader::StartElement::StartElement()
    {
    }
StreamXmlReader::StartElement::~StartElement()
    {
    }
StreamXmlReader::EventType StreamXmlReader::StartElement::type() const
    {
    return StreamXmlReader::START_ELEMENT;
    }


StreamXmlReader::EndElement::EndElement()
    {
    }
StreamXmlReader::EndElement::~EndElement()
    {
    }
StreamXmlReader::EventType StreamXmlReader::EndElement::type() const
    {
    return StreamXmlReader::END_ELEMENT;
    }


StreamXmlReader::Text::Text()
    {
    }
StreamXmlReader::Text::~Text()
    {
    }
StreamXmlReader::EventType StreamXmlReader::Text::type() const
    {
    return StreamXmlReader::TEXT;
    }


const char* StreamXmlReader::Text::data() const
    {
    return this->_data.c_str();
    }

StreamXmlReader::StreamXmlReader(std::istream& in):in(&in),reader(0)
    {
    const char* url=0;
    int options=0;
    reader=(void*)::xmlReaderForIO(
		StreamXmlReader::_xmlInputReadCallback,
		StreamXmlReader::_xmlInputCloseCallback,
   		 this,
   		 url,
   		 "UTF-8",
   		 options);
    }

int StreamXmlReader::_xmlInputReadCallback(
		 void * context,
		 char * buffer,
		 int len)
    {
    if( ((StreamXmlReader*)context)->in==0) return 0;
    WHERE(len);
    ((StreamXmlReader*)context)->in->read(buffer,len);
    WHERE(((StreamXmlReader*)context)->in->gcount());
    int n= (int) ((StreamXmlReader*)context)->in->gcount();
    WHERE("asked " << len << " return "<< n);
    return n;
    }
int StreamXmlReader::_xmlInputCloseCallback(void * context)
    {
    ((StreamXmlReader*)context)->in=0;
    return 0;
    }


StreamXmlReader::~StreamXmlReader()
    {
    close();
    }
bool StreamXmlReader::hasNext()
    {
    if(!queue.empty()) return true;
    return _fill();
    }

std::auto_ptr<StreamXmlReader::Event> StreamXmlReader::next()
    {
    WHERE("");
    std::auto_ptr<StreamXmlReader::Event> ret(0);
    for(;;)
	{
	if(!queue.empty())
	    {
	    ret.reset(queue.front());
	    queue.pop_front();
	    break;
	    }
	if(!_fill()) break;
	}
    return ret;
    }
void StreamXmlReader::close()
    {
    if(reader!=0)
	{
	::xmlFreeTextReader(CAST_READER(this));
	}
    reader=0;
    while(!queue.empty())
	{
	delete queue.back();
	queue.pop_back();
	}
    in=0;
    }

StreamXmlReader::QName StreamXmlReader::makeQName()
    {
    const char* L=(char*)xmlTextReaderConstLocalName(CAST_READER(this));
    const char* P=(char*)xmlTextReaderConstPrefix(CAST_READER(this));
    const char* N=(char*)xmlTextReaderConstNamespaceUri(CAST_READER(this));

    if(L==0) THROW("Local is null");
    if(P!=0 && N!=0)
	{
	WHERE(N << " " << P << " : " << L);
	return QName(N,L,P);
	}
    if(N!=0)
	{
	WHERE(N << " " << L);
	return QName(N,L);
	}

    return QName(L);
    //.assign((const char*)::xmlTextReaderConstName(CAST_READER(this)));
    }

bool StreamXmlReader::_fill()
    {
    WHERE("");
    if(!queue.empty()) return true;
    WHERE("");
    if(in==0 || reader==0) return false;
    while(queue.empty())
   	{
	WHERE("");
   	int ret = ::xmlTextReaderRead(CAST_READER(this));

   	if(ret==-1 || ret==0) return false;
   	int nodeType=xmlTextReaderNodeType(CAST_READER(this));
   	WHERE(nodeType);
   	switch(nodeType)
   	     {

   	     case XML_READER_TYPE_ELEMENT:
   		 {
   		 StartElement* e=new StartElement;

   		 e->_name=new QName(makeQName());

   		 queue.push_back(e);
   		 if(xmlTextReaderHasAttributes(CAST_READER(this)))
   		     {
		     int n_att=xmlTextReaderAttributeCount(CAST_READER(this));
		     for(int i=0;i< n_att;++i)
			 {
			 xmlTextReaderMoveToAttributeNo(CAST_READER(this),i);

			 xmlChar* v = xmlTextReaderValue(CAST_READER(this));
			 Attribute att(makeQName(),(const char*) v);
			 e->_attributes._atts.push_back(att);
			 xmlFree(v);
			 }
		     xmlTextReaderMoveToElement(CAST_READER(this));
   		     }
   		 if(xmlTextReaderIsEmptyElement(CAST_READER(this)))
   		     {
   		     EndElement* c=new EndElement;
   		     c->_name=new QName(*(e->_name));
   		     queue.push_back(c);
   		     }
   		 break;
   		 }
   	     case XML_READER_TYPE_END_ELEMENT:
   		 {
   		 EndElement* c=new EndElement;
   		 c->_name=new QName(makeQName());
   		 queue.push_back(c);
   		 break;
   		 }
   	     case XML_READER_TYPE_TEXT:
   		 {
   		 Text* t=new Text();
   		xmlChar* v = xmlTextReaderValue(CAST_READER(this));
   		 t->_data.assign((const char*)v);
   		 xmlFree(v);
   		queue.push_back(t);
   		 break;
   		 }
   	     case XML_READER_TYPE_COMMENT:
   		 {
   		WHERE(nodeType);
   		 break;
   		 }
   	     default:
   		 {
   		WHERE(nodeType);
   		 break;
   		 }
   	     }
   	}
    return true;
    }

#ifdef TEST_THIS_CODE

int main(int argc,char** argv)
    {
    StreamXmlReader in(cin);
    while(in.hasNext())
	{
	auto_ptr<StreamXmlReader::Event> evt=in.next();
	if(evt->isStartElement())
	    {
	    cout << "Start" << endl;
	    }
	else if(evt->isEndElement())
	    {
	    cout << "End" << endl;
	    }
	else if(evt->isText())
	    {
	    WHERE("");
	    cout << "#Text" << ((StreamXmlReader::Text*)evt.get())->data() << endl;
	    }
	}
    return 0;
    }

#endif

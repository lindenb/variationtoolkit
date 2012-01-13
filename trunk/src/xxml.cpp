
#include "xxml.h"
#include "throw.h"
#include "where.h"
using namespace std;


QName::QName(const char* namespaceURI,const char* localPart):_prefix("")
    {
    if(namespaceURI==0) THROW("namespaceURI is NULL");
    this->_namespaceURI.assign(namespaceURI);
    if(localPart==0) THROW("LocalPart is NULL");
    this->_localPart.assign(localPart);
    }
QName::QName(const char* namespaceURI,const char* localPart,const char* prefix)
    {
    if(namespaceURI==0) THROW("namespaceURI is NULL");
    this->_namespaceURI.assign(namespaceURI);
    if(localPart==0) THROW("LocalPart is NULL");
    this->_localPart.assign(localPart);
    if(prefix==0) THROW("Prefix is NULL");
    this->_prefix.assign(localPart);
    }
QName::QName(const char* localPart):
	_namespaceURI(""),
	_prefix("")

    {
    if(localPart==0) THROW("LocalPart is NULL");
    this->_localPart.assign(localPart);
    }

QName::QName(const QName& cp):
	_namespaceURI(cp._namespaceURI),
	_prefix(cp._prefix),
	_localPart(cp._localPart)
    {
    }


QName::~QName()
    {
    }


bool QName::operator==(const QName& cp) const
    {
    return  _namespaceURI.compare(cp._namespaceURI)==0 &&
	    _prefix.compare(cp._prefix)==0 &&
	    _localPart.compare(cp._localPart)==0
	    ;
    }

bool QName::operator<(const QName& cp) const
    {
    int i= _namespaceURI.compare(cp._namespaceURI);
    if(i!=0) return i<0;
    i= _prefix.compare(cp._prefix);
    if(i!=0) return i<0;
    i=_localPart.compare(cp._localPart);
    return i<0;
    }

QName& QName::operator=(const QName& cp)
    {
    if(this!=&cp)
	{
	_namespaceURI.assign(cp._namespaceURI);
	_prefix.assign(cp._prefix);
	_localPart.assign(cp._localPart);
	}
    return *this;
    }

const char* QName::namespaceURI() const
    {
    return _namespaceURI.c_str();
    }
const char* QName::prefix() const
    {
    return _prefix.c_str();
    }
const char* QName::localPart() const
    {
    return _localPart.c_str();
    }
/**************************************************************************************/
/**************************************************************************************/
/**************************************************************************************/


Attribute::Attribute(const QName name,const char* value):_name(name),_value(value)
    {
    }

Attribute::Attribute(const Attribute& cp):_name(cp._name),_value(cp._value)
    {
    }


Attribute::~Attribute()
    {
    }


bool Attribute::operator==(const Attribute& cp) const
    {
    return  _name==cp._name && _value.compare(cp._value)==0;
    }

bool Attribute::operator<(const Attribute& cp) const
    {
    if(_name<cp._name) return true;
    if(cp._name<_name) return false;
    return _value.compare(cp._value)< 0;
    }

Attribute&
Attribute::operator=(const Attribute& cp)
    {
    if(this!=&cp)
	{
	_name=cp._name;
	_value.assign(cp._value);
	}
    return *this;
    }

const QName& Attribute::name() const
    {
    return this->_name;
    }
const char* Attribute::value() const
    {
    return this->_value.c_str();
    }

const char* Attribute::namespaceURI() const
    {
    return _name.namespaceURI();
    }
const char* Attribute::prefix() const
    {
    return _name.prefix();
    }
const char* Attribute::localPart() const
    {
    return _name.localPart();
    }




/**************************************************************************************/
/**************************************************************************************/
/**************************************************************************************/

Attributes::Attributes()
    {

    }
Attributes::~Attributes()
    {

    }

uint64_t Attributes::size() const
    {
    return _atts.size();
    }

const Attribute& Attributes::item(uint64_t i) const
    {
    return _atts.at(i);
    }

const char*
Attributes::get( QName qName) const
    {
    for(uint64_t i=0;i< _atts.size();++i)
	{
	if(qName==_atts[i].name()) return _atts[i].value();
	}
    return 0;
    }

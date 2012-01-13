/*
 * XML utilities
 */

#ifndef EXTEND_XML_H
#define EXTEND_XML_H
#include <string>
#include <stdint.h>
#include <vector>


class QName
    {
    public:
	QName(const char* namespaceURI,const char* local);
	QName(const char* namespaceURI,const char* localPart,const char* prefix);
	QName(const char* localPart);
	QName(const QName& cp);
	~QName();
	bool operator==(const  QName& cp) const;
	bool operator<(const  QName& cp) const;
	QName& operator=(const QName& cp);
	const char* namespaceURI() const;
	const char* prefix() const;
	const char* localPart() const;
    private:
	std::string _namespaceURI;
	std::string _prefix;
	std::string _localPart;
    };
class Attribute
    {
    public:
	Attribute(const QName name,const char* value);
	Attribute(const Attribute& cp);
	~Attribute();
	bool operator==(const  Attribute& cp) const;
	bool operator<(const  Attribute& cp) const;
	Attribute& operator=(const Attribute& cp);
	const QName& name() const;
	const char* value() const;
	const char* namespaceURI() const;
	const char* prefix() const;
	const char* localPart() const;
    private:
	QName _name;
	std::string _value;
	friend class StreamXmlReader;
    };

class Attributes
    {
    public:
	Attributes();
	~Attributes();
	uint64_t size() const;
	const Attribute& item(uint64_t i) const;
	const char* get(QName qName) const;
    private:
	std::vector<Attribute> _atts;
	friend class StreamXmlReader;
    };




#endif

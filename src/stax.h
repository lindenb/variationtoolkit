/*
 * StreamXmlReader.h
 *
 *  Created on: Jan 10, 2012
 *      Author: lindenb
 */

#ifndef STREAMXMLREADER_H_
#define STREAMXMLREADER_H_
#include <iostream>
#include <memory>
#include <vector>
#include <deque>
#include <string>
#include <stdint.h>
class StreamXmlReader
    {
    public:
	enum EventType {START_ELEMENT,END_ELEMENT,TEXT};
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

	class Event
	    {
	    protected:
		Event();
	    public:
		virtual ~Event();
		virtual EventType type() const=0;
		bool isStartElement() const;
		bool isEndElement() const;
		bool isText() const;
		friend class StreamXmlReader;
	    };
	class ElementEvt:public Event
	    {
	    protected:
		ElementEvt();
		QName* _name;
	    public:
		const QName& name() const;
		virtual ~ElementEvt();
		friend class StreamXmlReader;
	    };
	class StartElement:public ElementEvt
	    {
	    public:

		StartElement();
		virtual ~StartElement();
		virtual EventType type() const;
		friend class StreamXmlReader;
		const Attributes& attributes() const;
	    private:

		Attributes _attributes;
	    };
	class EndElement:public ElementEvt
	    {
	    public:
		EndElement();
		virtual ~EndElement();
		virtual EventType type() const;
		friend class StreamXmlReader;
	    };
	class Text:public Event
	    {
	    public:
		Text();
		virtual ~Text();
		virtual EventType type() const;
		const char* data() const;
		friend class StreamXmlReader;
	    private:
		std::string _data;
	    };

	StreamXmlReader(std::istream& in);
	~StreamXmlReader();
	bool hasNext();
	std::auto_ptr<Event> next();
	void close();
    private:
	std::istream* in;
	std::deque<Event*> queue;
	void* reader;
	bool _fill();
	static int _xmlInputReadCallback(
			 void * context,
			 char * buffer,
			 int len);
	static int _xmlInputCloseCallback(void * context);
	QName makeQName();
    };



#endif /* STREAMXMLREADER_H_ */

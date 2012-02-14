/*
 * XML utilities
 */

#ifndef EXTEND_LIBXML_H
#define EXTEND_LIBXML_H
#include <iostream>
#include <sstream>
#include <string>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlreader.h>
#include <libxml/xmlwriter.h>

xmlOutputBufferPtr xmlOutputBufferCreateIOStream(std::ostream* out, xmlCharEncodingHandlerPtr encoder);
xmlOutputBufferPtr xmlOutputBufferCreateString(std::string* out, xmlCharEncodingHandlerPtr encoder);

template<typename T>
int xmlTextWriterWriteAttr(
	xmlTextWriterPtr writer,
	const xmlChar * name,
	const T value)
    {
    std::ostringstream os;
    os << value;
    std::string s(os.str());
    return ::xmlTextWriterWriteAttribute(writer,name,BAD_CAST s.c_str());
    }

template<typename T>
int xmlTextWriterWriteText(xmlTextWriterPtr writer,const T value)
    {
    std::ostringstream os;
    os << value;
    std::string s(os.str());
    return ::xmlTextWriterWriteString(writer,BAD_CAST s.c_str());
    }
/** simple wrapper around xmlTextWriterPtr */
class XmlStreamWriter
    {
    private:
	xmlTextWriterPtr delegate;
    public:
	XmlStreamWriter(xmlTextWriterPtr delegate);
	virtual ~XmlStreamWriter();
	xmlTextWriterPtr getDelegate();
	void writeStartDocument();
	void writeEndDocument();
	void writeAttribute(const char* name,const char* value);
	void writeStartElementNS(const char* prefix,const char* element,const char* ns);
	void writeStartElement(const char* name);
	void writeEndElement();
	void writeText(const char* s);
	void writeComment(const char* s);
	template <typename T>  void writeString(const T value)
	    {
	    std::ostringstream os;
	    os << value;
	    std::string s(os.str());
	    this->writeText(s.c_str());
	    }
	template <typename T>
	void writeAttr(const char* name,const T value)
	    {
	    std::ostringstream os;
	    os << value;
	    std::string s(os.str());
	    return this->writeAttribute(name,s.c_str());
	    }
    };

#endif

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


#endif

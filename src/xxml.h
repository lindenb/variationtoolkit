/*
 * XML utilities
 */

#ifndef EXTEND_LIBXML_H
#define EXTEND_LIBXML_H
#include <iostream>
#include <string>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlreader.h>

xmlOutputBufferPtr xmlOutputBufferCreateIOStream(std::ostream* out, xmlCharEncodingHandlerPtr encoder);
xmlOutputBufferPtr xmlOutputBufferCreateString(std::string* out, xmlCharEncodingHandlerPtr encoder);



#endif

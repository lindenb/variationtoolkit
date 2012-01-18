#ifndef XML_STREAM_H
#define XML_STREAM_H
#include <iostream>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlreader.h>

class XmlStream
    {
    public:
	XmlStream(std::istream& in);
	virtual ~XmlStream();
	xmlDocPtr next();
	virtual void close();
	/** enable/disable comments */
	void setAllowComments(bool b);
	/** return wether the current node is the pivot node
	 * default behavior : elements under the root node
	 */
	virtual bool isPivotNode(const xmlNodePtr element) const;
    private:
	std::istream* in;
	xmlTextReaderPtr reader;
	xmlDocPtr dom;
	xmlNodePtr root;
	xmlNodePtr current;
	xmlNodePtr pivot;
	bool allow_comments;
	static int _xmlInputReadCallback(
			     void * context,
			     char * buffer,
			     int len);
	static int _xmlInputCloseCallback(void * context);
    };



#endif

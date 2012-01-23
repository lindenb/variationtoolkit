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
	/** returns the next DOM fragment, the class XmlStream is responsible for releasing the memory */
	xmlDocPtr next();
	virtual void close();
	/** enable/disable comments */
	void setAllowComments(bool b);
	/** return wether the current node is the pivot node
	 * default behavior : elements under the root node
	 * @param element the element
	 * @param depth1 depth for this element. root is '1'
	 */
	virtual bool isPivotNode(const xmlNodePtr element,int depth1) const;
	/* get the current pivot node, can be called only after next() */
	virtual  xmlNodePtr getCurrentPivot() const;
	/** utility, return the depth of an ELEMENT: root is '1' */
	static int  depth(const xmlNodePtr element);
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

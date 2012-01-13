#include "xstream.h"
#include "throw.h"

using namespace std;

XmlStream::XmlStream(std::istream& in):
	in(&in),reader(0),dom(0)
    {
    const char* url=0;
    int options=0;
    reader= ::xmlReaderForIO(
	    XmlStream::_xmlInputReadCallback,
	    XmlStream::_xmlInputCloseCallback,
	     this,
	     url,
	     "UTF-8",
	     options
	     );
    }
XmlStream::~XmlStream()
    {
    close();
    }

void XmlStream::close()
    {
    if(reader!=0)
	{
	::xmlFreeTextReader(reader);
	reader=0;
	}
    if(dom!=0)
	{
	::xmlFreeDoc(dom);
	dom=0;
	}
    in=0;
    }

int XmlStream::_xmlInputReadCallback(
		 void * context,
		 char * buffer,
		 int len)
    {
    if( ((XmlStream*)context)->in==0) return 0;
    ((XmlStream*)context)->in->read(buffer,len);
    int n= (int) ((XmlStream*)context)->in->gcount();
    return n;
    }
int XmlStream::_xmlInputCloseCallback(void * context)
    {
    ((XmlStream*)context)->in=0;
    return 0;
    }


xmlDocPtr XmlStream::next()
    {
    for(;;)
	{
	int ret = ::xmlTextReaderRead(reader);
	if(ret==-1 || ret==0)
	    {
	    close();
	    return 0;
	    }
	int nodeType= ::xmlTextReaderNodeType(reader);
	switch(nodeType)
	     {

	     case XML_READER_TYPE_ELEMENT:
		 {


		 xmlNodePtr node= 0;
		 xmlNsPtr newNs=0;
		 //no namespace defined
		 if(xmlTextReaderConstNamespaceUri(reader)==0)
		     {
		     node=::xmlNewNode(0,xmlTextReaderConstLocalName(reader));
		     }
		 else
		     {
		     node=::xmlNewNode(0,xmlTextReaderConstLocalName(reader));
		     }



		 if(dom==0)
		     {
		     dom = ::xmlNewDoc(BAD_CAST "1.0");
		     ::xmlDocSetRootElement(dom, node);
		     this->root=node;
		     this->current=root;
		     }
		 else
		     {
		     ::xmlAddChild(this->current,node);
		     this->current=node;
		     }

		 if(xmlTextReaderHasAttributes(reader))
		     {
		     int n_att=xmlTextReaderAttributeCount(reader);
		     for(int i=0;i< n_att;++i)
			 {
			 xmlTextReaderMoveToAttributeNo(reader,i);

			 xmlChar* v = xmlTextReaderValue(reader);
			 xmlNewProp(node, BAD_CAST "attribute", v);
			 xmlFree(v);
			 }
		     xmlTextReaderMoveToElement(reader);
		     }
		 if(xmlTextReaderIsEmptyElement(reader))
		     {

		     }
		 break;
		 }
	     case XML_READER_TYPE_END_ELEMENT:
		 {
		 xmlNodePtr parent=this->current->parent;
		 break;
		 }
	     case XML_READER_TYPE_TEXT:
		 {
		 const xmlChar* v = xmlTextReaderConstValue(reader);
		 xmlNodePtr node= xmlNewDocText(this->dom,v);
		 ::xmlAddChild(this->current,node);
		 break;
		 }
	     case XML_READER_TYPE_COMMENT:
		 {
		 xmlChar* v = xmlTextReaderValue(reader);
		 xmlNodePtr node= xmlNewComment(v);
		 xmlFree(v);
		 ::xmlAddChild(this->current,node);
		 break;
		 }
	     default:
		 {
		 break;
		 }
	     }
	}
    return dom;
    }

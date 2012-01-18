#include <cassert>
#include "xstream.h"
#include "throw.h"
#define NOWHERE
#include "where.h"

using namespace std;

XmlStream::XmlStream(std::istream& in):
	in(&in),reader(0),dom(0),root(0),current(0),pivot(0),
	allow_comments(false)
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

void XmlStream::setAllowComments(bool b)
    {
    allow_comments=b;
    }

void XmlStream::close()
    {
    WHERE("");
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
    pivot=0;
    current=0;
    root=0;
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

bool XmlStream::isPivotNode(const xmlNodePtr element) const
    {
    return element!=0 && element->parent==root;
    }

xmlDocPtr XmlStream::next()
    {
    if(pivot!=0)
	{
	assert(current==pivot->parent);
	::xmlUnlinkNode(pivot);
	::xmlFreeNode(pivot);
	pivot=0;
	}
    WHERE("");
    if(reader==0) return 0;
    for(;;)
	{

	int ret = ::xmlTextReaderRead(reader);
	if(ret<=0)
	    {
	    WHERE("");
	    reader=0;//close() = NON
	    if(dom==0) return 0;
	    break;
	    }
	WHERE(ret);
	int nodeType= ::xmlTextReaderNodeType(reader);
	switch(nodeType)
	            {
	            case XML_READER_TYPE_ELEMENT:
	                {
	                WHERE("");
	                xmlNodePtr node;
	                if(dom==NULL)
	                    {
	                    dom =::xmlNewDoc( BAD_CAST "1.0");
	                    }
	                if(xmlTextReaderConstNamespaceUri(reader)!=0)
	                    {
	                    xmlNsPtr ns=xmlSearchNs(dom,current,xmlTextReaderConstNamespaceUri(reader));
	                    node=xmlNewNode(ns, xmlTextReaderConstName(reader));
	                    if(ns==0)
	                        {
	                        ns=xmlNewNs(node,
	                            xmlTextReaderConstNamespaceUri(reader),
	                            xmlTextReaderConstPrefix(reader)
	                            );
	                        }
	                    }
	                else
	                    {
	                    node=xmlNewNode(0, xmlTextReaderConstName(reader));
	                    }

	                if(current==NULL)
	                    {
	                    xmlDocSetRootElement(dom,node);
	                    }
	                else
	                    {

	                    xmlAddChild(current,node);
	                    }

	                current=node;


	                if(xmlTextReaderHasAttributes(reader))
	                     {
	                     int i;
	                     int n_att=xmlTextReaderAttributeCount(reader);
	                     for(i=0;i< n_att;++i)
				 {
				 const xmlChar* k;
				 xmlChar* v;
				 xmlTextReaderMoveToAttributeNo(reader,i);
				 k = xmlTextReaderConstName(reader);
				 v = xmlTextReaderValue(reader);
				 if(xmlTextReaderConstNamespaceUri(reader)!=0)
				    {
				    if(!::xmlStrEqual(xmlTextReaderConstNamespaceUri(reader),BAD_CAST "http://www.w3.org/2000/xmlns/"))
					{
					xmlNsPtr ns=xmlSearchNs(dom,current,xmlTextReaderConstNamespaceUri(reader));
					if(ns==0)
					    {
					    ns=xmlNewNs(node,
						    xmlTextReaderConstNamespaceUri(reader),
						    xmlTextReaderConstPrefix(reader)
					    );
					    }
					xmlNewNsProp(current,ns,
						xmlTextReaderConstLocalName(reader) ,
						v);
					}
				    }
				 else
				    {
				    xmlNewProp(current,k, v);
				    }


				 xmlFree(v);
				 }
	                     xmlTextReaderMoveToElement(reader);
	                     }
	                if(xmlTextReaderIsEmptyElement(reader))
	                     {
	                     current= current->parent;
	                     }
	                WHERE("");
	                break;
	                }
	             case XML_READER_TYPE_END_ELEMENT:
	                 {

	                 if(current!=0 )
	                     {

	                     if(isPivotNode(current))
	                	 {
	                	 WHERE("OK it's a PIVOT: "<< current->name);
	                	 pivot=current;
	                	 current= current->parent;
	                	 return dom;
	                	 }
	                     else
	                	 {
	                	 current= current->parent;
	                	 }
	                     }
	                 break;
	                 }
	             case XML_READER_TYPE_SIGNIFICANT_WHITESPACE:
	             case XML_READER_TYPE_TEXT:
	                 {
	                 const xmlChar* v = xmlTextReaderConstValue(reader);
	                 xmlNodePtr node= xmlNewDocText(dom,v);
	                 xmlAddChild(current,node);
	                 break;
	                 }
	             case XML_READER_TYPE_PROCESSING_INSTRUCTION:
	        	 {
	        	 xmlNodePtr n=xmlNewPI(
	        		 xmlTextReaderConstName(reader),
	        		 xmlTextReaderConstValue(reader)
	        	     );
	        	 xmlAddChild(current,n);
	        	 break;
	        	 }
	             case XML_READER_TYPE_COMMENT:
	        	 {
	        	 if(allow_comments)
	        	     {
	        	     xmlNodePtr	n=xmlNewComment(xmlTextReaderConstValue(reader));
	        	     xmlAddChild(current,n);
	        	     }
	        	 break;
	        	 }
	            default:
	                {
	                fprintf(stderr,"Ignoring node Type %d\n",nodeType);
	                break;
	                }
	            }
	}
    close();
    return 0;
    }


#ifdef TEST_THIS_CODE
#include <fstream>
int main(int argc,char** argv)
    {
    xmlDocPtr dom;
    fstream in(argv[1]);
    XmlStream app(in);
    while((dom=app.next())!=0)
	{

	}
    WHERE("");
    xmlDocPtr d = xmlReadFile(argv[1], NULL, 0);
    xmlDocDump(stdout,d);
    xmlFreeDoc(d);
    return 0;
    }

#endif

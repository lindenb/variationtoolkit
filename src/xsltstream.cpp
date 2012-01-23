/**
 * Author:
 *	Pierre Lindenbaum PhD
 * Contact:
 *	plindenbaum@yahoo.fr
 * Date:
 *	Jan 2012
 * WWW:
 *	http://plindenbaum.blogspot.com
 * Motivation:
 *	XSLT operations on DOM fragments
 */
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cctype>
#include <map>
#include <iostream>
#include <memory>
#include <stdint.h>
#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#include "zstreambuf.h"
#include "application.h"
#include "numeric_cast.h"
#define NOWHERE
#include "where.h"
#include "xstream.h"

using namespace std;

class XsltStream:public AbstractApplication
    {
    private:
	xsltStylesheetPtr stylesheet;
	map<string,string> params;
	int target_depth;
	xmlChar* target_name;

	   class Handler:public XmlStream
	       {
		public:
		   XsltStream* owner;
		   Handler(std::istream& in,XsltStream* owner):XmlStream(in),owner(owner) {}
		   virtual ~Handler() {}

		   virtual bool isPivotNode(const xmlNodePtr element,int depth1) const
		       {
		       if(owner->target_name==0 && owner->target_depth<0 )
			   {
			   return depth(element)==2;
			   }

		       if(owner->target_name!=0)
			   {
			   if(!xmlStrEqual(owner->target_name,element->name))
			       {
			       return false;
			       }
			   }
		       if(owner->target_depth!=-1)
			   {
			   if(depth1!=owner->target_depth)
			       {
			       return false;
			       }
			   }
		       return true;
		       }

	       };

    public:


	XsltStream():stylesheet(0),
	    target_depth(-1),
	    target_name(0)
	    {
	    }

	virtual ~XsltStream()
	    {
	    if(stylesheet!=0) ::xsltFreeStylesheet(stylesheet);
	    }

	void run(const char* filename,std::istream& in)
	    {
	    const char** params=new const char*[1];
	    params[0]=0;


	    std::auto_ptr<Handler> p=std::auto_ptr<Handler>(new Handler(in,this));
	    for(;;)
		{
		xmlDocPtr doc= p->next();
		if(doc==0) break;
		if(stylesheet==0)
		    {
		    xmlElemDump(stdout,doc,xmlDocGetRootElement(doc));

		    }
		else
		    {
		    stylesheet->omitXmlDeclaration=1;
		    xmlDocPtr res=::xsltApplyStylesheet(stylesheet, doc, params);
		    if(res==0)
			{
			THROW("Cannot transform XML 2 HTML.");
			}
		    xmlOutputBufferPtr outbuff=xmlOutputBufferCreateFile(stdout,0);
		    if(outbuff==0)
			{
			THROW("xmlOutputBufferCreateFile failed");
			}
		    ::xsltSaveResultTo(outbuff,res,stylesheet);
		    xmlOutputBufferClose(outbuff);
		    ::xmlFreeDoc(res);
		    }
		}

	    delete[] params;
	    }

	void usage(ostream& out,int argc,char** argv)
	    {
	    out << argv[0] << " Pierre Lindenbaum PHD. 2012.\n";
	    out << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
	    out << "Usage" << endl
		    << "   "<< argv[0]<< " [options] (stdin|file.xml|file.xml.gz)"<< endl;
	    out << "Options:\n";
	    out << "    -f (filename) xslt stylesheet.\n";
	    out << "    -n (name) target element name (default:0).\n";
	    out << "    -d (int) target element depth (default:none) root is '1'.\n";
	    out << endl;
	    }

	int main(int argc,char** argv)
	    {
	    char* xsltfile=0;
	    int optind=1;
	    srand(time(0));
	    while(optind < argc)
		{
		if(std::strcmp(argv[optind],"-h")==0)
			{
			usage(cerr,argc,argv);
			return (EXIT_FAILURE);
			}
		else if(std::strcmp(argv[optind],"-f")==0 && optind+1< argc)
			{
			xsltfile=argv[++optind];
			}
		else if(std::strcmp(argv[optind],"-n")==0 && optind+1< argc)
		    {
		     this->target_name=(xmlChar*)argv[++optind];
		    }
		else if(std::strcmp(argv[optind],"-d")==0 && optind+1< argc)
		    {
		    if(!numeric_cast<int>(argv[++optind],&(this->target_depth)) || this->target_depth<1)
			{
			cerr << "bad depth '"<< argv[optind]<< "'"<< endl;
			usage(cerr,argc,argv);
			}
		    }
		else if(argv[optind][0]=='-')
			{
			cerr << "unknown option '"<< argv[optind]<< "'"<< endl;
			usage(cerr,argc,argv);
			return (EXIT_FAILURE);
			}
		else
			{
			break;
			}
		++optind;
		}
	    if(xsltfile!=0)
		{
		xmlDocPtr xsl=::xmlParseFile(xsltfile);
		if(xsl==0)
		    {
		    cerr << "Cannot read XSLT file: "<< xsltfile << endl;
		    return EXIT_FAILURE;
		    }
		this->stylesheet=::xsltParseStylesheetDoc(xsl);
		if(stylesheet==0)
		    {
		    xmlFreeDoc(xsl);
		    cerr << "Cannot compile XSLT file: "<< xsltfile << endl;
		    return EXIT_FAILURE;
		    }
		}

	    if(optind==argc)
	     	{
	     	igzstreambuf buf;
	     	istream in(&buf);
	     	this->run("-",in);
	     	}
	    else
	     	{
	     	while(optind< argc)
	     	    {
	     	    igzstreambuf buf(argv[optind++]);
	     	    istream in(&buf);
	     	    this->run(argv[optind],in);
	     	    buf.close();
	     	    ++optind;
	     	    }
	     	}
	    return EXIT_SUCCESS;
	    }
    };

int main(int argc,char** argv)
    {
    XsltStream app;
    return app.main(argc,argv);
    }

#ifndef XML_ESCAPE_H
#define XML_ESCAPE_H

#include <string>
#include <iostream>
#include <cstdio>
#include <cctype>
#include <sstream>

class xmlEscape
    {
    private:
	std::string content;
    public:
	xmlEscape(std::string s):content(s)
	    {
	    }
	std::ostream& print(std::ostream& out) const
	    {
	    for(std::string::size_type i=0;i< content.size();++i)
		{
		char c=content[i];
		switch(c)
		    {
		    case '<': out << "&lt;";break;
		    case '>': out << "&gt;";break;
		    case '&': out << "&amp;";break;
		    case '\'': out << "&apos;";break;
		    case '\"': out << "&quot;";break;
		    default: out << c;break;
		    }
		}
	    return out;
	    }
	std::string str() const
	    {
	    std::ostringstream os;
	    print(os);
	    return os.str();
	    }
    };

static std::ostream& operator << (std::ostream& out,const xmlEscape& cp)
    {
    return cp.print(out);
    }

#endif

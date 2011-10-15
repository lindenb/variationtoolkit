#ifndef HTTP_ESCAPE_H
#define HTTP_ESCAPE_H

#include <string>
#include <iostream>
#include <cstdio>
#include <cctype>
#include <sstream>

class httpEscape
    {
    private:
	std::string content;
    public:
	httpEscape(std::string s):content(s)
	    {
	    }
	std::ostream& print(std::ostream& out) const
	    {
	    for(std::string::size_type i=0;i< content.size();++i)
		{
		char c=content[i];
		if(c==' ')
		    {
		    out << "+";
		    }
		else if(std::isalpha(c) || std::isdigit(c))
		    {
		    out <<c;
		    }
		else
		    {
		    char tmp[10];
		    std::sprintf(tmp,"%02X",(int)c);
		    out << "%" << tmp;
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

static std::ostream& operator << (std::ostream& out,const httpEscape& cp)
    {
    return cp.print(out);
    }

#endif

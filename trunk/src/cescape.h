#ifndef C_ESCAPE_H
#define C_ESCAPE_H

#include <string>
#include <iostream>
#include <cstdio>
#include <cctype>
#include <sstream>

class CEscape
    {
    private:
	std::string content;
    public:
	CEscape(std::string s):content(s)
	    {
	    }
	std::ostream& print(std::ostream& os) const
	    {
	    for(std::string::size_type i=0;i< content.size();++i)
		{
		char c=content[i];
		switch(c)
			 {
			 case '\\': os << "\\\\"; break;
			 case '\'': os << "\\\'"; break;
			 case '\"': os << "\\\""; break;
			 case '\t': os << "\\t"; break;
			 case '\n': os << "\\n"; break;
			 case '\r': os << "\\r"; break;
			 case '\b': os << "\\b"; break;
			 default:os << c;break;
			 }
		}
	    return os;
	    }
	std::string str() const
	    {
	    std::ostringstream os;
	    print(os);
	    return os.str();
	    }
    };

static std::ostream& operator << (std::ostream& out,const CEscape& cp)
    {
    return cp.print(out);
    }

#endif

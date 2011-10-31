#ifndef SMART_COMPARATOR_H
#define SMART_COMPARATOR_H
#include <string>
#include <cstring>
#include <cctype>
#include <cstdlib>

class SmartComparator
    {
    public:
    
   bool operator() (const char* s1, const char* s2) const
	{
	return compare(s1,s2)<0;
	}

    bool operator() (const std::string& a, const std::string& b) const
	{
	return compare(a,b)<0;
	}
      
    bool operator()(
		const char* s1,std::size_t len1,
		const char* s2,std::size_t len2
		) const
	{
	return compare(s1,len1,s2,len2)<0;
	}

    int compare(const std::string& a, const std::string& b) const
    	{
    	return compare(a.c_str(),a.size(),b.c_str(),b.size())<0;
    	}


    int compare(
    		const char* s1,
    		const char* s2
    		) const
	{
	return compare(s1,std::strlen(s1),s2,std::strlen(s2))<0;
	}

    int compare(
		const char* s1,std::size_t len1,
		const char* s2,std::size_t len2
		) const
	  {
	  char* p1=(char*)s1;
	  char* p2=(char*)s2;
	  const char* p1_end=&s1[len1];
	  const char* p2_end=&s2[len2];
	  for(;;)
	      {
	      if(p1==p1_end && p2==p2_end) return 0;
	      if(p1==p1_end && p2!=p2_end) return -1;
	      if(p1!=p1_end && p2==p2_end) return 1;
	      if(isdigit(*p1) && isdigit(*p2))
		  {
		  char* pp1;
		  char* pp2;
		  long n1=strtol(p1,&pp1,10);
		  long n2=strtol(p2,&pp2,10);
		  if(n1!=n2) return (n1<n2?-1:1);
		  p1=pp1;
		  p2=pp2;
		  continue;
		  }
	      int i=toupper(*p1)-toupper(*p2);
	      if(i!=0) return i;
	      ++p1;
	      ++p2;
	      }
	  }
    };

#endif

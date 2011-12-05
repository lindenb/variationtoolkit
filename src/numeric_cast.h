#ifndef MY_NUMERIC_CAST_H
#define MY_NUMERIC_CAST_H
#include <string>
#include <sstream>
#include <iostream>



template<typename T>
bool numeric_cast(char const* s,T* v)
	{
	if(s==NULL) return false;
	std::string ss(s);
	while(!ss.empty() && std::isspace(ss.at(ss.size()-1)))
		{
		ss=ss.substr(0,ss.size()-1);
		}
	while(!ss.empty() && std::isspace(ss.at(0)))
		{
		ss=ss.substr(1,ss.size()-1);
		}
		
	std::istringstream in(ss);
	T t;
	in >> t;
	if(!in || !in.eof()) return false;
	if(v!=NULL) *v=t;
	return true;
	}


#endif


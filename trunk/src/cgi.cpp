/*
 * cgi.cpp
 *
 *  Created on: Sep 23, 2011
 *      Author: lindenb
 */
#include <sstream>
#include "cgi.h"

using namespace std;


cgistreambuf::cgistreambuf(std::streambuf* delegate):printed(false),delegate(delegate)
	{
	setbuf(buffer,BUFSIZ);
	header.insert(make_pair<string,string>("Content-Type","text-plain"));
	header.insert(make_pair<string,string>("Status","200 OK"));
	}

cgistreambuf::~cgistreambuf()
	{
	}

void cgistreambuf::setStatus(int status)
	{
	ostringstream os;
	os << status;
	header["Status"]=os.str();
	}
void cgistreambuf::setContentType(const char* mime)
	{
	header["Content-Type"]=mime;
	}
int cgistreambuf::overflow(int c)
	{
	flushHeaders();
	return delegate->sputc(c);
	}

void cgistreambuf::flushHeaders()
	{
	if(printed) return;
	ostream os(delegate);
	for(map<string,string>::iterator r=header.begin();
		r!=header.end();
		++r)
		{
		os << r->first << ":" << r->second << endl;
		}
	os << endl;
	printed=true;
	}

bool cgistreambuf::headersFlushed()
	{
	return printed;
	}

CGI::Param::Param(std::string key,std::string value):_key(key),_value(value)
	{
	}

const char* CGI::Param::name() const
	{
	return _key.c_str();
	}


const char* CGI::Param::value() const
	{
	return _value.c_str();
	}

CGI::Param::~Param()
	{
	}


std::size_t CGI::max_input_size() const
	{
	return BUFSIZ;
	}

bool CGI::parseGET()
	{
	const char* query_string=::getenv("QUERY_STRING");
	if(query_string==NULL)
	    {
	    _last_error.reset(new string("QUERY_STRING==nil"));
	    return false;
	    }
	size_t contentLength=std::strlen(query_string);
	if(contentLength> max_input_size() )
	    {
	    _last_error.reset(new string("Content too large"));
	    return false;
	    }
	std::string query(query_string);
	std::istringstream in(query);
	if(contentLength>0)
		{
		return parseQueryString(in,contentLength);
		}
	return true;
	}
bool CGI::parsePOST()
	{
	char* contentLengthStr= getenv("CONTENT_LENGTH");
	if(contentLengthStr==NULL)
	    {
	    _last_error.reset(new string("CONTENT_LENGTH missing"));
	    return false;
	    }
	int contentLength=atoi(contentLengthStr);
	if(contentLength<0)
	    {
	    _last_error.reset(new string("Bad content Length "));
	    return false;
	    }
	if((size_t)contentLength> max_input_size() )
	    {
	    _last_error.reset(new string("Content too large"));
	    return false;
	    }

	if(!isMultipart())
		{
		if(contentLength>0)
			{
			return parseQueryString(std::cin,contentLength);
			}
		return true;
		}
	_last_error.reset(new string("Cannot parse multipart actions"));
	return false;
	}

bool CGI::parseQueryString(std::istream& in,int maxCharRead)
	{
	int c;
	std::string key;
	std::string value;
	int count=0;
	bool in_key=true;
	while((c=in.get())!=EOF && count < maxCharRead)
		{
		++count;
		if(c=='+') c=' ';
		if(c=='&')
			{
			if(!key.empty())
				{
				parameters.push_back(new Param(key,value));
				}
			in_key=true;
			key.clear();
			value.clear();
			}
		else if(c=='=')
			{
			in_key=false;
			value.clear();
			}
		else
			{
			if(c=='%' && count+2<=maxCharRead)
				{
				int c2= in.get();
				if(c2==EOF)
				    {
				    _last_error.reset(new string("Bad Input"));
				    return false;
				    }
				int c3= in.get();
				if(c3==EOF)
				    {
				    _last_error.reset(new string("Bad Input"));
				    return false;
				    }
				c=x2c(c2,c3);
				count+=2;
				}
			if(!in_key)
				{
				value+=(char)c;
				}
			else
				{
				key+=(char)c;
				}
			}
		}
	if(!key.empty())
		{
		parameters.push_back(new Param(key,value));
		}
	return true;
	}

int CGI::x2c(int c1,int c2)
	{
	char buff[3]={c1,c2,0};
	return std::strtol(buff,NULL,16);
	}
bool CGI::isMultipart()
	{
	char* contentType= getenv("CONTENT_TYPE");
	if(contentType==NULL) return false;
	return std::strstr(contentType,"multipart/form-data")!=NULL;
	}

CGI::CGI()
	{
	}

CGI::~CGI()
	{
	std::vector<Param*>::iterator r=parameters.begin();
	while(r!=parameters.end())
		{
		delete (*r);
		++r;
		}
	}

const char* CGI::requestMethod() const
	{
	return ::getenv("REQUEST_METHOD");
	}

bool CGI::parse()
	{
	const char* method= requestMethod();
	if(method==NULL)
	    {
	    _last_error.reset(new string("Cannot find REQUEST_METHOD"));
	    return false;
	    }
	if(std::strcmp(method,"POST")==0)
		{
		return parsePOST();
		}
	else if(std::strcmp(method,"GET")==0)
		{
		return parseGET();
		}

	_last_error.reset(new string("Unknown REQUEST_METHOD"));
	return false;
	}

bool CGI::contains(std::string key) const
	{
	std::vector<Param*>::const_iterator r=parameters.begin();
	while(r!=parameters.end())
		{
		if(key.compare((*r)->name())==0)
			{
			return true;
			}
		++r;
		}
	return false;
	}


bool CGI::contains(std::string key,std::string value) const
    {
    std::vector<Param*>::const_iterator r=parameters.begin();
    while(r!=parameters.end())
    		{
    		if(key.compare((*r)->name())==0 && value.compare((*r)->value())==0)
    			{
    			return true;
    			}
    		++r;
    		}
    return false;
    }


const char* CGI::getParameter(std::string key) const
	{
	std::vector<Param*>::const_iterator r=parameters.begin();
	while(r!=parameters.end())
		{
		if(key.compare((*r)->name())==0)
			{
			return (*r)->value();
			}
		++r;
		}
	return NULL;
	}

std::set<std::string> CGI::getParameters(std::string key) const
	{
	std::set<std::string> vals;
	std::vector<Param*>::const_iterator r=parameters.begin();
	while(r!=parameters.end())
		{
		if(key.compare((*r)->name())==0)
			{
			vals.insert(std::string( (*r)->value() ));
			}
		++r;
		}
	return vals;
	}


std::set<std::string> CGI::getParameterNames() const
	{
	std::set<std::string> keys;
	std::vector<Param*>::const_iterator r=parameters.begin();
	while(r!=parameters.end())
		{
		keys.insert(std::string((*r)->name()));
		++r;
		}
	return keys;
	}

 void CGI::_var(std::ostream& out,const char* s)
	{
	char* p=::getenv(s);
	out << s << ":" << (p==NULL?"null":p) << "\n";
	}


std::ostream& CGI::dump(std::ostream& out)
	{
	std::set<std::string> keys;
	_var(out,"DOCUMENT_ROOT");
	_var(out,"HTTP_REFERER");
	_var(out,"HTTP_USER_AGENT");
	_var(out,"PATH_INFO");
	_var(out,"PATH_TRANSLATED");
	_var(out,"QUERY_STRING");
	_var(out,"REMOTE_ADDR");
	_var(out,"REQUEST_METHOD");
	_var(out,"SCRIPT_NAME");
	_var(out,"SERVER_NAME");
	_var(out,"SERVER_PORT");

	std::vector<Param*>::iterator r=parameters.begin();
	while(r!=parameters.end())
		{
		out	<< (*r)->name()
			<< " : "
			<< (*r)->value()
			<< std::endl;
		++r;
		}
	return out;
	}

const char* CGI::last_error() const
    {
    if(_last_error.get()==0) return 0;
    return _last_error->c_str();
    }

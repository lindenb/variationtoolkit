/*
 * cgi.h
 *
 *  Created on: Sep 23, 2011
 *      Author: lindenb
 */

#ifndef CGI_H_
#define CGI_H_


#include <sstream>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <cstring>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <memory>
/**
 * name:
 * 	CGI
 *
 * author:
 *	Pierre Lindenbaum PhD. (2010)
 *	http://plindenbaum.blogspot.com
 *	plindenbaum@yahoo.fr
 */
enum HttpStatusCode
	{
        SC_CONTINUE = 100,
        SC_SWITCHING_PROTOCOLS = 101,
        SC_PROCESSING = 102,
        SC_OK = 200,
        SC_CREATED = 201,
        SC_ACCEPTED = 202,
        SC_NON_AUTHORITATIVE_INFORMATION = 203,
        SC_NO_CONTENT = 204,
        SC_RESET_CONTENT = 205,
        SC_PARTIAL_CONTENT = 206,
        SC_MULTI_STATUS = 207,
        SC_MULTIPLE_CHOICES = 300,
        SC_MOVED_PERMANENTLY = 301,
        SC_MOVED_TEMPORARILY = 302,
        SC_SEE_OTHER = 303,
        SC_NOT_MODIFIED = 304,
        SC_USE_PROXY = 305,
        SC_TEMPORARY_REDIRECT = 307,
        SC_BAD_REQUEST = 400,
        SC_UNAUTHORIZED = 401,
        SC_PAYMENT_REQUIRED = 402,
        SC_FORBIDDEN = 403,
        SC_NOT_FOUND = 404,
        SC_METHOD_NOT_ALLOWED = 405,
        SC_NOT_ACCEPTABLE = 406,
        SC_PROXY_AUTHENTICATION_REQUIRED = 407,
        SC_REQUEST_TIMEOUT = 408,
        SC_CONFLICT = 409,
        SC_GONE = 410,
        SC_LENGTH_REQUIRED = 411,
        SC_PRECONDITION_FAILED = 412,
        SC_REQUEST_TOO_LONG = 413,
        SC_REQUEST_URI_TOO_LONG = 414,
        SC_INSUFFICIENT_SPACE_ON_RESOURCE = 419,
        SC_METHOD_FAILURE = 420,
        SC_UNPROCESSABLE_ENTITY = 422,
        SC_LOCKED = 423,
        SC_FAILED_DEPENDENCY = 424,
        SC_INTERNAL_SERVER_ERROR = 500,
        SC_SERVICE_UNAVAILABLE = 503,
        SC_GATEWAY_TIMEOUT = 504,
        SC_HTTP_VERSION_NOT_SUPPORTED = 505
        };


class cgistreambuf:public std::streambuf
	{
	private:
		char buffer[BUFSIZ];
		bool printed;
		std::streambuf* delegate;
		std::map<std::string,std::string> header;

	public:
		cgistreambuf(std::streambuf* sb);
		void setStatus(int status);
		void setContentType(const char* s);
		virtual ~cgistreambuf();
		virtual int overflow(int c=EOF);
		virtual void flushHeaders();
		virtual bool headersFlushed();
	protected:
		int sync();
	};


class CGI
	{
	public:
		/** simple parameter */
		class Param
			{
			private:
				std::string _key;
				std::string _value;
			public:
				Param(std::string key,std::string value);
				const char* name() const;
				const char* value() const;
				~Param();
			};

	protected:
		std::size_t max_input_size() const;
	private:
		std::auto_ptr<std::string> _last_error;
		std::multimap<std::string,Param*> parameters;
		bool parseGET();
		bool parsePOST();
		bool parseQueryString(std::istream& in,int maxCharRead);
		static int x2c(int c1,int c2);
		bool isMultipart();
	public:
		CGI();
		virtual ~CGI();
		const char* requestMethod() const;
		bool parse();
		bool contains(std::string key) const;
		bool contains(std::string key,std::string value) const;
		const char* getParameter(std::string key) const;
		std::set<std::string> getParameters(std::string key) const;
		std::set<std::string> getParameterNames() const;
		const char* last_error() const;
		void putParameter(const char* k,const char* v);
		void setParameter(const char* k,const char* v);
		void removeParameter(const char* k);
	private:
		 void _var(std::ostream& out,const char* s);
	public:

		std::ostream& dump(std::ostream& out);
	};


#endif /* CGI_H_ */

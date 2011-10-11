/*
 * netstreambuf.h
 *
 *  Created on: Oct 11, 2011
 *      Author: lindenb
 */

#ifndef NET_STREAMBUF_H
#define NET_STREAMBUF_H


#include <iostream>
#include <string>

class netstreambuf:public std::streambuf
    {
    private:
	char* buffer;
	std::size_t buffer_size;
	void* curl_handle;
	void* multi_handle;
	bool callback_was_called;
	/**  number of CURL running */
	int still_running;
	/** called by the CURL  callback */
	std::size_t call(void *ptr, std::size_t size, std::size_t nmemb);
	static std::size_t write_callback(void *ptr, std::size_t size, std::size_t nmemb, void *userp);
    public:
	netstreambuf();
	void open(const char* url);
	virtual ~netstreambuf();
	virtual void close();
	virtual int underflow ();
	virtual std::streamsize read(char* s,std::size_t len);
	std::string content();
    };


#endif

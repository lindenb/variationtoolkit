#include <curl/curl.h>
#include "ncbi.h"
using namespace std;

#define CAST_CURL(a) ((CURL*)a)
#define CAST_CURLM(a) ((CURLM*)a)

Service::Service():curl_handle(NULL),multi_handle(NULL)
	{
	this->curl_handle=::curl_easy_init( );
	if(this->curl_handle==NULL)
	    {
	    THROW("Cannot curl_easy_init");
	    }
	if(this->curl_handle==NULL)
							{
							throw std::runtime_error("::curl_easy_init failed");
							}

						ret=::curl_easy_setopt(this->curl_handle, CURLOPT_URL, this->url.c_str());
						check_error(ret);


						ret=::curl_easy_setopt(this->curl_handle, CURLOPT_VERBOSE, 0);
						check_error(ret);
						ret=::curl_easy_setopt(this->curl_handle, CURLOPT_WRITEDATA, this);
						check_error(ret);
						ret=::curl_easy_setopt(this->curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
						check_error(ret);
						ret=::curl_easy_setopt(this->curl_handle, CURLOPT_WRITEFUNCTION,
							curl_streambuf::write_callback
							);
						check_error(ret);


						this->multi_handle=::curl_multi_init();
						if(this->multi_handle==NULL)
							{
							:: curl_easy_cleanup(this->curl_handle);
							this->curl_handle=NULL;
							throw std::runtime_error("curl_multi_init failed");
							}

						ret2=::curl_multi_add_handle(
							this->multi_handle,
							this->curl_handle
							);

	}

Service::~Service()
	{
	}

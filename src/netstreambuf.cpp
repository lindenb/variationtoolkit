#include <cstdlib>
#include <cstring>
#include <curl/curl.h>
#include "netstreambuf.h"
#include "throw.h"

using namespace std;
#define NETSIZE BUFSIZ

#define CAST_CURL(a) ((CURL*)a)
#define CAST_CURLM(a) ((CURLM*)a)

static void check_error(CURLcode code)
    {
    if(code==CURLE_OK) return;
    THROW(::curl_easy_strerror(code));
    }

static void check_error(CURLMcode code)
    {
    if(code==CURLM_OK) return;
    THROW(::curl_multi_strerror(code));
    }


netstreambuf::netstreambuf():buffer_size(0),
	curl_handle(NULL),
	multi_handle(NULL),
	total_read(0UL)
    {

    buffer=(char*)malloc(NETSIZE*sizeof(char));
    if(buffer==NULL) THROW("out of memory");
    buffer_size=0;
    setg( &this->buffer[0],
	&this->buffer[NETSIZE],
	&this->buffer[NETSIZE]
	);



    }
netstreambuf::~netstreambuf()
    {
    close();
    free(buffer);
    }
void netstreambuf::open(const char* url)
    {
    CURLcode ret;
    CURLMcode ret2;
    close();
    this->total_read=0UL;
    /* init curl */
    ::curl_global_init(CURL_GLOBAL_ALL);
    this->curl_handle=::curl_easy_init( );
    if(this->curl_handle==NULL) THROW("::curl_easy_init failed");

    ret=::curl_easy_setopt(this->curl_handle, CURLOPT_URL,url);
    check_error(ret);
    ret=::curl_easy_setopt(this->curl_handle, CURLOPT_VERBOSE,0);
    check_error(ret);

    ret=::curl_easy_setopt(this->curl_handle, CURLOPT_FAILONERROR,true);
    check_error(ret);

    ret=::curl_easy_setopt(this->curl_handle, CURLOPT_WRITEDATA, this);
    check_error(ret);
    ret=::curl_easy_setopt(this->curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    check_error(ret);
    //ret=::curl_easy_setopt(this->curl_handle, CURLOPT_HTTP_CONTENT_DECODING, 1);
    //check_error(ret);
    //ret=::curl_easy_setopt(this->curl_handle, CURLOPT_HTTP_TRANSFER_DECODING, 1);
    //check_error(ret);


    ret=::curl_easy_setopt(this->curl_handle, CURLOPT_WRITEFUNCTION,
	    netstreambuf::write_callback );
    check_error(ret);
    char* proxy=NULL;
    if(strncmp(url,"http://",7)==0)
    	{
    	proxy=getenv("http_proxy");
    	}
    else if(strncmp(url,"https://",7)==0)
	{
	proxy=getenv("https_proxy");
	}
    else if(strncmp(url,"ftp://",7)==0)
	{
	proxy=getenv("ftp_proxy");
	}

    if(proxy!=NULL)
    	{
    	ret=::curl_easy_setopt(this->curl_handle, CURLOPT_PROXY,proxy);
    	check_error(ret);
    	}

    this->multi_handle=::curl_multi_init();
    if(this->multi_handle==NULL)
	{
	:: curl_easy_cleanup(this->curl_handle);
	this->curl_handle=NULL;
	THROW("curl_multi_init failed");
	}

    ret2=::curl_multi_add_handle(
	this->multi_handle,
	this->curl_handle
	);
    check_error(ret2);
    }

void netstreambuf::close()
    {
    if(multi_handle!=NULL)
	    {
	    if(curl_handle!=NULL)
		    {
		    ::curl_multi_remove_handle(
			    CAST_CURLM(multi_handle),
			    CAST_CURL(curl_handle)
			    );
		    }
	    ::curl_multi_cleanup(CAST_CURLM(multi_handle));
	    multi_handle=NULL;
	    }
    if(curl_handle!=NULL)
	    {
	    :: curl_easy_cleanup(CAST_CURL(curl_handle));
	    curl_handle=NULL;
	    }
    callback_was_called=false;
    }

int netstreambuf::underflow ( )
    {
    CURLMcode ret2;
    if(curl_handle==NULL) return EOF;
    this->buffer_size=0;
    do
	    {
	    this->callback_was_called=false;
	    ret2= ::curl_multi_perform(this->multi_handle,&( this->still_running));
	    check_error(ret2);
	    } while( this->still_running!=0 && this->callback_was_called==false);


    if( this->buffer_size==0)
	    {
	    close();
	    return EOF;
	    }
    return this->buffer[0];
    }

size_t netstreambuf::write_callback(void *ptr, size_t size, size_t nmemb, void *userp)
    {
    return ((netstreambuf*)userp)->call(ptr,  size, nmemb);
    }

/** called by the CURL  callback */
size_t netstreambuf::call(void *ptr, size_t size, size_t nmemb)
    {
    size_t remains=((size_t)egptr() - (size_t)gptr());
    
    	
    this->callback_was_called=true;
    this->buffer_size = remains+size*nmemb;

    this->total_read+=this->buffer_size;

    char *array=(char*)std::malloc(this->buffer_size);
    if(array==NULL) THROW("out of memory ("<< this->buffer_size << " bytes)");
    if(remains>0)
    	{
    	std::memcpy((void*)&array[0],  gptr(), remains);
    	}
    std::memcpy((void*)&array[remains],ptr, (size*nmemb));
    std::free(this->buffer);
    this->buffer=array;

    setg(	(char*)this->buffer,
	    (char*)&this->buffer[0],
	    (char*)&this->buffer[this->buffer_size]
	    );
    return (size*nmemb);
    }

std::size_t netstreambuf::gcount() const
    {
    return this->total_read;
    }

streamsize netstreambuf::read(char* s,std::size_t len)
    {
    return this->xsgetn(s,len);
    }

std::string netstreambuf::content()
    {
    std::string page;
    char t[BUFSIZ];
    streamsize nRead;
    while((nRead=read(t,BUFSIZ))!=0)
		{
		page.append(t,nRead);
		}
    close();
    return page;
    }


#ifdef TEST_THIS_CODE
int main(int argc,char** argv)
    {
    for(int i=1;i< argc;++i)
	{
	cout << argv[i]<< endl;
	netstreambuf buf;
	istream in(&buf);
	buf.open( argv[i]);
	char t[BUFSIZ];
	streamsize nRead;
	    while((nRead=buf.read(t,BUFSIZ))!=0)
	    {
	cout.write(t,nRead);
	    }
	//cout << "\ndone " <<buf.gcount() << "\n";
	}
    return 0;
    }
#endif

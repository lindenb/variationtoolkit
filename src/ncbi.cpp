#include <curl/curl.h>
#include "ncbi.h"
using namespace std;

Service::Service():curl_handle(NULL),multi_handle(NULL)
	{
	}

Service::~Service()
	{
	}

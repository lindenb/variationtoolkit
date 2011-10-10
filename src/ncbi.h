#ifndef NCBI_H
#define NCBI_H

class Service
	{
	private:
		void* curl_handle;
		void* multi_handle;
	public:
		Service();
		virtual ~Service();
	};

class ESearch
	{
	private:
		CURL *curl_handle;
		/** CURLM ptr */
		CURLM *multi_handle;
		/** the buffer for streambug */
	public:
		
		ESearch();
		~ESearch();
		
	};



#endif

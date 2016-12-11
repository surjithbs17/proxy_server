//============================================================================
// Name        : webproxy.cpp
// Author      : Surjith Bhagavath Singh
// Version     : 0.1
// Description : Web Proxy in C++
//============================================================================


Readme for webproxy (Programming Assignment 4)

1. Mozilla Firefox has to be configured to support the proxy (Given in the PA document)
2. Browser sends a HTTP request to proxy server,Proxy server will forwarded to the corresponding sites and the reply has been forwarded back to server
3. Web proxy acts like a middleman
4. This proxy server supports only http connections. It doesnt support https connections
5. It supports both HTTP/1.0 and 1.1
6. Link Prefetching has been implemented 
	- The file recieved has been checked for any links, then the links are pre fetched and stored in cache folder with their url encoded using md5 algorithm.
7. Time out, The file modification time has been checked to know whether it is a old file or new file.
	- If there is any prefetched file available then it will send the cached copy to the user


Python Script -
	I tried the python script, It worked at times but not always. I can schedule a skype session to show the working of my code. Hope my video clarifies it.

	
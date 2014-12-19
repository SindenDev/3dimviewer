///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// Copyright (c) 2010 by 3Dim Laboratory s.r.o.
//
///////////////////////////////////////////////////////////////////////////////

#include <network/CCurl.h>

#include <cstdio>

//==================================================================================================
CCurl::CCurl()
{
	if ( !CCurl::bInitialized ) 
	{
		curl_global_init(CURL_GLOBAL_ALL);
		CCurl::bInitialized = true;
	}

	pBuffer = pError = 0;
	pHandle = curl_easy_init();
	init();
}

//==================================================================================================
CCurl::~CCurl()
{
	curl_easy_cleanup( pHandle );

	if ( pBuffer ) delete [] pBuffer;
	if ( pError ) delete [] pError;
}

//==================================================================================================
void	CCurl::init()
{
	pBuffer = new char [ CCurl::DEFAULT_BUFFER_SIZE ];
	pError  = new char [ 16 * CURL_ERROR_SIZE ];

	if ( pHandle )
	{
        curl_easy_setopt(pHandle, CURLOPT_HTTPAUTH, CURLAUTH_DIGEST);//CURLAUTH_BASIC);
        curl_easy_setopt(pHandle, CURLOPT_USE_SSL, CURLUSESSL_ALL);


		//curl_easy_setopt( pHandle, CURLOPT_PROTOCOLS, CURLPROTO_HTTPS);
		curl_easy_setopt( pHandle, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt( pHandle, CURLOPT_SSL_VERIFYHOST, 0L);
		curl_easy_setopt( pHandle, CURLOPT_VERBOSE, 1L);
		curl_easy_setopt( pHandle, CURLOPT_HEADER, 1L);
		curl_easy_setopt( pHandle, CURLOPT_COOKIEFILE, "" );
		curl_easy_setopt( pHandle, CURLOPT_WRITEFUNCTION, CCurl::readBuffer );			
		curl_easy_setopt( pHandle, CURLOPT_WRITEDATA, pBuffer);
		curl_easy_setopt( pHandle, CURLOPT_ERRORBUFFER, pError ); 
	}
}

//==================================================================================================
void	CCurl::setUrl( const std::string & url )
{
	if ( pHandle )
	{
		curl_easy_setopt( pHandle, CURLOPT_URL, url.c_str() );
	}
}

//==================================================================================================
void	CCurl::setLoginInfo( const std::string & user, const std::string & pw )
{
	if ( pHandle )
	{
      std::string usrpwd( user + ":" + pw );
//		curl_easy_setopt( pHandle, CURLOPT_USERNAME, user.c_str() );
//		curl_easy_setopt( pHandle, CURLOPT_PASSWORD, pw.c_str() );
      curl_easy_setopt( pHandle, CURLOPT_USERPWD, usrpwd.c_str() );
	}
}

//==================================================================================================
bool	CCurl::perform( std::string & output )
{
	CURLcode res = curl_easy_perform( pHandle );

	if ( res == 0 )
	{
		output = std::string( pBuffer );
		return true;
	}	
	else return false;
}

//==================================================================================================
bool	CCurl::sendString( const std::string & input, std::string & output )
{
	if ( pHandle )
	{
		char inbuff[1024];

		sprintf( inbuff, "current_3dim=%s", input.c_str() );
		curl_easy_setopt( pHandle, CURLOPT_POSTFIELDS, inbuff );
			
		return perform( output );
	}
	else return false;
}

//==================================================================================================
void	CCurl::addPost( const std::string & field, const std::string & contents )
{
	if ( pHandle )
	{
		int length = field.length() + contents.length() + 32;
		char * buffer = new char[ length ];

		sprintf( buffer, "%s=%s", field.c_str(), contents.c_str() );
		curl_easy_setopt( pHandle, CURLOPT_POSTFIELDS, buffer );

		delete [] buffer;
	}
}

//==================================================================================================
bool	CCurl::sendFile( char * buffer, int length, const std::string & name, const std::string & data, std::string & output, std::vector< std::string > & additional )
{	 
	 if ( pHandle )
	 {
		 struct curl_httppost * post = NULL;
		 struct curl_httppost * last = NULL;

		 curl_formadd( &post, &last,
					   CURLFORM_COPYNAME, name.c_str(),
					   CURLFORM_BUFFER, data.c_str(),
					   CURLFORM_BUFFERPTR, buffer,
					   CURLFORM_BUFFERLENGTH, length,
					   CURLFORM_END );

		 if ( additional.size() == 8 )
		 {

			 curl_formadd( &post, &last,
						   CURLFORM_COPYNAME, additional[0].c_str(),
						   CURLFORM_COPYCONTENTS, additional[1].c_str(),
						   CURLFORM_END );

			 curl_formadd( &post, &last,
						   CURLFORM_COPYNAME, additional[2].c_str(),
						   CURLFORM_COPYCONTENTS, additional[3].c_str(),
						   CURLFORM_END );

			 curl_formadd( &post, &last,
						   CURLFORM_COPYNAME, additional[4].c_str(),
						   CURLFORM_COPYCONTENTS, additional[5].c_str(),
						   CURLFORM_END );

			 curl_formadd( &post, &last,
						   CURLFORM_COPYNAME, additional[6].c_str(),
						   CURLFORM_COPYCONTENTS, additional[7].c_str(),
						   CURLFORM_END );
		 }


		 curl_easy_setopt( pHandle, CURLOPT_HTTPPOST, post );
		 return perform( output );
	 }
	 else return false;
}


//==================================================================================================
volatile bool CCurl::bInitialized	=	false;

//==================================================================================================
const int CCurl::DEFAULT_BUFFER_SIZE = 1024;

//==================================================================================================
size_t CCurl::readBuffer( char * data, size_t size, size_t nmemb, char * buffer )
{
    size_t result = 0;  
    if( buffer != NULL )
    {
        result = size * nmemb;
        if( result >= size_t(DEFAULT_BUFFER_SIZE) )
        {
            result = size_t(DEFAULT_BUFFER_SIZE - 1);
        }

        for( size_t i = 0; i < result; i++ )
        {
            buffer[i] = data[i];
        }
        buffer[result] = '\0';
    }
    return result;
}

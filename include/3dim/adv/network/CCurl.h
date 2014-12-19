///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// Copyright (c) 2010 by 3Dim Laboratory s.r.o.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef CCurl_H
#define CCurl_H

#include <VPL/Base/Object.h>
#include <VPL/Base/SharedPtr.h>

#include <curl/curl.h>
//#include <curl/types.h> // deprecated for long time
#include <curl/easy.h>

#include <vector>
#include <string>


///////////////////////////////////////////////////////////////////////////////
//! CCurl
//! - 

class CCurl : public vpl::base::CObject
{
	public :

		//!
		CCurl();

		//!
		~CCurl();

		//!
		void	init();

		//!
		void	setUrl( const std::string & url );

		//!
		void	setLoginInfo( const std::string & user, const std::string & pw );

		//!
		bool	perform( std::string & output );

		//!
		bool	sendString( const std::string & input, std::string & output );

		//!
		bool	sendFile( char * buffer, int length, const std::string & name, const std::string & data, std::string & output, std::vector< std::string > & additional );

		//!
		void	addPost( const std::string & field, const std::string & contents );

	protected :

		//!
		static volatile bool bInitialized; 

		//!
		static const int DEFAULT_BUFFER_SIZE;

		//!
		char *	pBuffer;

		char *  pError;

	public :
		
		//!
		static size_t readBuffer( char  *data, size_t size, size_t nmemb, char * buffer );

	protected :

		//!
		CURL * pHandle;

};

#endif // CCurl_H

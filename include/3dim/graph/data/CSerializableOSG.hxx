///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// 3DimViewer
// Lightweight 3D DICOM viewer.
//
// Copyright 2008-2016 3Dim Laboratory s.r.o.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
///////////////////////////////////////////////////////////////////////////////

//! Serialize matrix
template < class tpMatrix >
template< class tpSerializer, int size >
void COSGMatrixSerializer< tpMatrix >::serialize( const tpMatrix & m, vpl::mod::CChannelSerializer<tpSerializer> & s )
{
	// serialize matrix size
	s.write( (vpl::sys::tInt32)size );

	// serialize elements
	for( int r = 0; r < size; ++r )
		for( int c = 0; c < size; ++c )
			s.write( m( r, c ) );
}

//! Deserialize matrix
template < class tpMatrix >
template< class tpSerializer, int size >
void COSGMatrixSerializer< tpMatrix >::deserialize( tpMatrix & m, vpl::mod::CChannelSerializer<tpSerializer> & s )
{
	// serialize matrix size
    vpl::sys::tInt32 n = 0;
	s.read( n ); 
    size = n;

	// serialize elements
	for( int r = 0; r < size; ++r )
		for( int c = 0; c < size; ++c )
			s.read( m( r, c ) );
}

///////////////////////////////////////////////////////////////////////////////
// $Id: CFloodRasterizer.h 
//
// Copyright (c) 2009 by 3Dim Laboratory s.r.o.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef CFloodRasterizer_H_included
#define CFloodRasterizer_H_included

namespace seg
{

/******************************************************************************
	Template class for 2D fill algorithm. Testing and filling functors
	ar used.
******************************************************************************/
template
<
	class tpTestFunctor,
	class tpFillFunctor
>
class CFlood2DRasterizer
{
public:
	//! Do filling
	void Rasterize( int startx, int starty );

protected:
	//! Stack

}; // CFlood2DRasterizer


/******************************************************************************
	Template class for 3D fill algorithm. Testing and filling functors
	ar used.
******************************************************************************/
template
<
	class tpTestFunctor,
	class tpFillFunctor
>
class CFlood3DRasterizer
{
public:
	//! Do filling
	void Rasterize( int startx, int starty );

protected:
	//! Stack

}; // CFlood2DRasterizer

#include "CFloodRasterizer.hxx"

} // namespace seg

// CFloodRasterizer_H_included
#endif



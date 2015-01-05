///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// Copyright (c) 2009 by 3Dim Laboratory s.r.o.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef CISSEEDFILL_H_INCLUDED
#define CISSEEDFILL_H_INCLUDED

//#include <stack>
#include <deque>

/******************************************************************************
	TEMPLATE CLASS CSeedFill2D

	Do a seed fill on mds slice.
******************************************************************************/
template
< 
	class tpTestFunctor, 
	class tpModifyFunctor 
>
class CSeedFill2D
{
protected:

	//! 2D seed
	class CSeed2D
	{
	public:
		//! Constructor
		CSeed2D( int _x, int _y ) : x( _x ), y( _y ) {}

		//! Copy constructor
		CSeed2D( const CSeed2D & seed ) : x( seed.x ), y( seed.y ) {}

		//! Coordinates
		int x, y;
	};

//	typedef std::stack< CSeed2D > tSeed2DStack;
	typedef std::deque< CSeed2D > tSeed2DStack;

public:
	//! Do a seed fill
	long Fill( int x, int y, tpTestFunctor fncTest, tpModifyFunctor fncModify );

	//! Set sizes
	void SetSizes( int xmin, int xmax, int ymin, int ymax )
	{
		m_xmin = xmin; m_xmax = xmax; m_ymin = ymin; m_ymax = ymax;
	}
protected:
	//! Test if seed is in volume
	bool IsIn( const CSeed2D & seed ){ return seed.x < m_xmax && seed.x >= m_xmin && seed.y < m_ymax && seed.y >= m_ymin; }

protected:
	//! Image size
	int m_xmin, m_xmax, m_ymin, m_ymax;
};

/******************************************************************************
	TEMPLATE CLASS CSeedFill3D

	Do a seed fill on mds volume.
******************************************************************************/
template
< 
	class tpTestFunctor, 
	class tpModifyFunctor 
>
class CSeedFill3D
{
public:
	//! Do a seed fill
	long Fill( int x, int y, int z, tpTestFunctor fncTest, tpModifyFunctor fncModify );

	//! Set sizes
	void SetSizes( int xmin, int xmax, int ymin, int ymax, int zmin, int zmax )
	{
		m_xmin = xmin; m_xmax = xmax; m_ymin = ymin; m_ymax = ymax; m_zmin = zmin; m_zmax = zmax;
	}

protected:
	//! Volume size
	int m_xmin, m_xmax, m_ymin, m_ymax, m_zmin, m_zmax;
};


/******************************************************************************
	TEMPLATE CSeedFill2D::
******************************************************************************/
template
< 
	class tpTestFunctor, 
	class tpModifyFunctor 
>
long CSeedFill2D::Fill( int x, int y, tpTestFunctor fncTest, tpModifyFunctor fncModify )
{
	if( ! fncTest( x, y ) )
		return 0;

	// Seed stack
	tSeed2DStack stack;

	//stack.push
//	stack.push( CSeed2D( x, y ) );
	stack.push_back( CSeed2D( x, y ) );

	CSeed2D seed;

	// While seeds on stack, 
	while( !stack.empty() )
	{
//		seed = stack.top();
//		stack.pop();
		seed = stack.front();
		stack.pop_front();

		// Get and test seed
		if( IsIn( seed ) && fncTest( seed.x, seed.y ) )
		{
			fncModify( seed.x, seed.y );

			// store neigbors on stack
//			stack.push( seed.x - 1, seed.y );
//			stack.push( seed.x + 1, seed.y );
//			stack.push( seed.x, seed.y - 1 );
//			stack.push( seed.x, seed.y + 1 );
			stack.push_back( seed.x - 1, seed.y );
			stack.push_back( seed.x + 1, seed.y );
			stack.push_back( seed.x, seed.y - 1 );
			stack.push_back( seed.x, seed.y + 1 );
		}
	}
}

// CISSEEDFILL_H_INCLUDED
#endif

///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// Copyright (c) 2009 by 3Dim Laboratory s.r.o.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef CRASTERIZERBASE_H_INCLUDED
#define CRASTERIZERBASE_H_INCLUDED

#include <osg/Array>
#include <VPL/base/Functor.h>
#include <VPL/Base/Types.h>

namespace seg
{
/******************************************************************************
	CLASS CModifier base - 2D version
******************************************************************************/
template
<
	class I
>
class CISModifierBase2D
{
public:
	//! Is plane image?
	enum { TEMPLATE_PARAMETER_IS_NOT_IMAGE=I::CLASS_IMAGE };

	//! Image type
	typedef typename I::tImage tImage;

	//! Pixel type
	typedef typename I::tPixel tPixel;
public:
	//! Constructor
	CISModifierBase2D(){}

	//! Destructor 
	~CISModifierBase2D(){}

	//! Operator
	virtual bool operator() ( const vpl::tSize x, const vpl::tSize y  ) { return false; }

	
	//! Plane pointer
	I * m_planePtr;
};


/******************************************************************************
	CLASS CModifier base - 3D version
******************************************************************************/
template
<
	class V
>
class CISModifierBase3D
{
public:
	//! Is plane image?
	enum { TEMPLATE_PARAMETER_IS_NOT_VOLUME=V::CLASS_VOLUME };

	//! Image type
	typedef typename V::tVolume tVolume;

	//! Pixel type
	typedef typename V::tVoxel tVoxel;

public:
	//! Constructor
	CISModifierBase3D(){}

	//! Destructor 
	~CISModifierBase3D(){}

	//! Operator
	virtual bool operator() ( const vpl::tSize x, const vpl::tSize y, const vpl::tSize z ) { return false;	}

	
	//! Volume pointer
	V * m_volumePtr;
};

/******************************************************************************
	CLASS CModifier base - hybrid version (handles 2D and 3D points)
******************************************************************************/
template
<
	class V
>
class CISModifierBaseHybrid : public CISModifierBase3D< V >
{
public:
	//! Constructor
	CISModifierBaseHybrid(){}

	//! Destructor 
	~CISModifierBaseHybrid(){}

	//! Operator
	virtual bool operator() ( const osg::Vec2 & point ) { return false;	}

};

/******************************************************************************
	CLASS CISTestBase2D
******************************************************************************/
template
<
	class I
>
class CISTestBase2D
{
public:
	//! Is plane image?
	enum { TEMPLATE_PARAMETER_IS_NOT_IMAGE=I::CLASS_IMAGE };

	//! Image type
	typedef typename I::tImage tImage;

	//! Pixel type
	typedef typename I::tPixel tPixel;
public:
	//! Constructor
	CISTestBase2D(){}

	//! Destructor 
	~CISTestBase2D(){}

	//! Operator
	virtual bool operator() ( int x, int y, tImage * image, const tPixel & testValue ) { return false; }
}; 



/******************************************************************************
	CLASS CISTestBase3D
******************************************************************************/
template
<
	class V
>
class CISTestBase3D
{
public:
	//! Is plane image?
	enum { TEMPLATE_PARAMETER_IS_NOT_VOLUME=V::CLASS_VOLUME };

	//! Image type
	typedef typename V::tVolume tVolume;

	//! Pixel type
	typedef typename V::tVoxel tVoxel;

public:
	//! Constructor
	CISTestBase3D(){}

	//! Destructor 
	~CISTestBase3D(){}

	//! Operator
	virtual bool operator() ( int x, int y, int z, tVolume * volume, const tVoxel & testValue ) { return false;	}
};


} // namespace seg

// CRASTERIZERBASE_H_INCLUDED
#endif

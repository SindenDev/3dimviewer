///////////////////////////////////////////////////////////////////////////////
// 
// Copyright 2008-2015 3Dim Laboratory s.r.o.
// All rights reserved
//

#ifndef CRegionCounter_H_included
#define CRegionCounter_H_included

#include <geometry/base/CMesh.h>

// STL
#include <deque>

class CRegionCounter
{
public:
	//! Count regions
	static size_t count(geometry::CMesh &mesh);
};

// CRegionCounter_H_included
#endif
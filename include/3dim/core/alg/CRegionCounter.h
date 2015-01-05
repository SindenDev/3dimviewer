#ifndef CRegionCounter_H_included
#define CRegionCounter_H_included

#include <data/CMesh.h>

// STL
#include <deque>

class CRegionCounter
{
public:
	//! Count regions
	static size_t count(data::CMesh &mesh);
};

// CRegionCounter_H_included
#endif
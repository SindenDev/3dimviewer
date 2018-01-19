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

#include <geometry/base/types.h>
#include <vpl/Image/Image.h>

#ifndef CMARCHINGSQUARES_H
#define CMARCHINGSQUARES_H

#define P1 geometry::Vec2(0, 1)
#define P2 geometry::Vec2(1, 2)
#define P3 geometry::Vec2(2, 1)
#define P4 geometry::Vec2(1, 0)
#define P5 geometry::Vec2(1, 1)

/*#define P1 geometry::Vec2(0.5, 1)
#define P2 geometry::Vec2(1, 1.5)
#define P3 geometry::Vec2(1.5, 1)
#define P4 geometry::Vec2(1, 0.5)*/

class CMarchingSquares
{

public:
	CMarchingSquares(vpl::img::CImage16& sliceData);
	~CMarchingSquares();

	void process(std::vector<geometry::Vec2>& vertices, std::vector<int>& indices);

private:
	vpl::img::CImage16 m_sliceData;

	void addBoundaryPoints(std::vector<geometry::Vec2>& vertices, std::vector<int>& indices, int squareType, int x, int y);
};

#endif // CMARCHINGSQUARES_H
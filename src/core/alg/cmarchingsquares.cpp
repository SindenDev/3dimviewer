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

#include <core/alg/cmarchingsquares.h>
//#include <QDebug>

CMarchingSquares::CMarchingSquares(vpl::img::CImage16& sliceData) :
	m_sliceData(sliceData)
{

}

CMarchingSquares::~CMarchingSquares()
{

}

void CMarchingSquares::process(std::vector<geometry::Vec2>& vertices, std::vector<int>& indices)
{
	vpl::tSize xSize = m_sliceData.getXSize();
	vpl::tSize ySize = m_sliceData.getYSize();

	/*int maxRegionIndex = 0;

	for (vpl::tSize y = 0; y < ySize; ++y)
	{
		for (vpl::tSize x = 0; x < xSize; ++x)
		{
			int index = m_sliceData.at(x, y);

			if (index > maxRegionIndex)
			{
				maxRegionIndex = index;
			}
		}
	}

	std::vector<std::vector<vpl::math::CIVector2> > regionsBoundary(maxRegionIndex + 1);*/

	//std::vector<vpl::math::CIVector2> boundaryPoints;
	//std::vector<int> boundaryPoints;

	for (vpl::tSize y = 0; y < ySize - 1; ++y)
	{
		for (vpl::tSize x = 0; x < xSize - 1; ++x)
		{
			int regions[4];
			regions[0] = m_sliceData.at(x, y + 1);
			regions[1] = m_sliceData.at(x + 1, y + 1);
			regions[2] = m_sliceData.at(x + 1, y);
			regions[3] = m_sliceData.at(x, y);

			bool done[4];
			for (int i = 0; i < 4; ++i)
			{
				done[i] = false;
			}

			int squares[4];
			for (int i = 0; i < 4; ++i)
			{
				squares[i] = 0;
			}

			done[0] = true;

			if (regions[0] > 0)
			{
				squares[0] = 1;

				for (int i = 1; i < 4; ++i)
				{
					if (regions[i] == regions[0])
					{
						squares[0] += pow(2.0, i);
						done[i] = true;
					}
				}
			}

			if (!done[1] && regions[1] > 0)
			{
				squares[1] = 2;

				done[1] = true;

				for (int i = 2; i < 4; ++i)
				{
					if (!done[i] && regions[i] == regions[1])
					{
						squares[1] += pow(2.0, i);
						done[i] = true;
					}
				}
			}

			if (!done[2] && regions[2] > 0)
			{
				squares[2] = 4;

				done[2] = true;

				if (!done[3] && regions[3] == regions[2])
				{
					squares[2] += 8;
					done[3] = true;
				}
			}

			if (!done[3] && regions[3] > 0)
			{
				squares[3] = 8;
			}

			for (int i = 0; i < 4; ++i)
			{
				if (squares[i] != 0 && squares[i] != 15 && regions[i] != 0)
				{
					//addBoundaryPoints(regionsBoundary[regions[i]], squares[i], x, y);

					addBoundaryPoints(vertices, indices, squares[i], x, y);
				}
			}
		}
	}
}

void CMarchingSquares::addBoundaryPoints(std::vector<geometry::Vec2>& vertices, std::vector<int>& indices, int squareType, int x, int y)
{
	int index = vertices.size() - 1;

	geometry::Vec2 curPoint(x, y);

	if (squareType == 1 || squareType == 14)
	{
		vertices.push_back(curPoint + P1);
		vertices.push_back(curPoint + P2);
		vertices.push_back(curPoint + P5);
	}
	else if (squareType == 2 || squareType == 13)
	{
		vertices.push_back(curPoint + P2);
		vertices.push_back(curPoint + P3);
		vertices.push_back(curPoint + P5);
	}
	else if (squareType == 3 || squareType == 12)
	{
		vertices.push_back(curPoint + P1);
		vertices.push_back(curPoint + P3);
		vertices.push_back(curPoint + P5);
	}
	else if (squareType == 4 || squareType == 11)
	{
		vertices.push_back(curPoint + P3);
		vertices.push_back(curPoint + P4);
		vertices.push_back(curPoint + P5);
	}
	else if (squareType == 5 || squareType == 10)
	{
		vertices.push_back(curPoint + P1);
		vertices.push_back(curPoint + P2);
		vertices.push_back(curPoint + P3);
		vertices.push_back(curPoint + P4);
		vertices.push_back(curPoint + P5);
	}
	else if (squareType == 6 || squareType == 9)
	{
		vertices.push_back(curPoint + P2);
		vertices.push_back(curPoint + P4);
		vertices.push_back(curPoint + P5);
	}
	else
	{
		vertices.push_back(curPoint + P1);
		vertices.push_back(curPoint + P4);
		vertices.push_back(curPoint + P5);
	}

	if (squareType == 5 || squareType == 10)
	{
		indices.push_back(1 + index);
		indices.push_back(5 + index);
		indices.push_back(2 + index);
		indices.push_back(5 + index);
		indices.push_back(3 + index);
		indices.push_back(5 + index);
		indices.push_back(4 + index);
		indices.push_back(5 + index);
	}
	else
	{
		indices.push_back(1 + index);
		indices.push_back(3 + index);
		indices.push_back(2 + index);
		indices.push_back(3 + index);
	}

	/*if (squareType == 1 || squareType == 14)
	{
		vertices.push_back(curPoint + P1);
		vertices.push_back(curPoint + P2);
	}
	else if (squareType == 2 || squareType == 13)
	{
		vertices.push_back(curPoint + P2);
		vertices.push_back(curPoint + P3);
	}
	else if (squareType == 3 || squareType == 12)
	{
		vertices.push_back(curPoint + P1);
		vertices.push_back(curPoint + P3);
	}
	else if (squareType == 4 || squareType == 11)
	{
		vertices.push_back(curPoint + P3);
		vertices.push_back(curPoint + P4);
	}
	else if (squareType == 5)
	{
		vertices.push_back(curPoint + P1);
		vertices.push_back(curPoint + P2);
		vertices.push_back(curPoint + P3);
		vertices.push_back(curPoint + P4);
	}
	else if (squareType == 6 || squareType == 9)
	{
		vertices.push_back(curPoint + P2);
		vertices.push_back(curPoint + P4);
	}
	else if(squareType == 7 || squareType == 8)
	{
		vertices.push_back(curPoint + P4);
		vertices.push_back(curPoint + P1);
	}
	else
	{
		vertices.push_back(curPoint + P4);
		vertices.push_back(curPoint + P1);
		vertices.push_back(curPoint + P2);
		vertices.push_back(curPoint + P3);
	}

	if (squareType == 5 || squareType == 10)
	{
		indices.push_back(1 + index);
		indices.push_back(2 + index);
		indices.push_back(3 + index);
		indices.push_back(4 + index);
	}
	else
	{
		indices.push_back(1 + index);
		indices.push_back(2 + index);
	}*/
}
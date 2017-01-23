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

///////////////////////////////////////////////////////////////////////////////
// include files

#include <data/CRegionDataCalculator.h>
#include <data/CRegionData.h>
#include <data/CDensityData.h>
#include <data/CRegionColoring.h>

#ifdef _OPENMP
#    include <omp.h>
#endif


namespace data
{
///////////////////////////////////////////////////////////////////////////////
//

CRegionDataCalculator::CRegionDataCalculator()
{ }

///////////////////////////////////////////////////////////////////////////////
//

CRegionDataCalculator::~CRegionDataCalculator()
{
}

////////////////////////////////////////////////////////////
//

bool CRegionDataCalculator::checkDependency(CStorageEntry *pParent)
{
	return true;
}


////////////////////////////////////////////////////////////
//

void CRegionDataCalculator::update(const CChangedEntries& Changes)
{
	CObjectPtr< CRegionColoring > spColoring(APP_STORAGE.getEntry(Storage::RegionColoring::Id));
	CObjectPtr< CRegionData > rVolume(APP_STORAGE.getEntry(Storage::RegionData::Id));
	CObjectPtr< CDensityData> spVolume(APP_STORAGE.getEntry(Storage::PatientData::Id));

	const vpl::tSize xSize = rVolume->getXSize();
	const vpl::tSize ySize = rVolume->getYSize();
	const vpl::tSize zSize = rVolume->getZSize();

	const vpl::tSize xOffsetDV = spVolume->getXOffset();
	const vpl::tSize xOffset = rVolume->getXOffset();

	vpl::img::tPixel16 maxId = 0;

	for (int i = 0; i < spColoring->getNumOfRegions(); ++i)
	{
		if (!spColoring->getRegionInfo(i).isAuxiliary() && i > maxId)
		{
			maxId = i;
		}
	}

	if (maxId == 0)
	{
		maxId = spColoring->getNumOfRegions();
	}

	m_voxelsCnt.resize(maxId + 1);
	m_minDensity.resize(maxId + 1);
	m_maxDensity.resize(maxId + 1);
	m_sumDensity.resize(maxId + 1);
	m_densityVariance.resize(maxId + 1);
	m_minXCoord.resize(maxId + 1);
	m_maxXCoord.resize(maxId + 1);
	m_minYCoord.resize(maxId + 1);
	m_maxYCoord.resize(maxId + 1);
	m_minZCoord.resize(maxId + 1);
	m_maxZCoord.resize(maxId + 1);

	for (int i = 0; i < maxId + 1; ++i)
	{
		m_voxelsCnt[i] = 0;
		m_minDensity[i] = 20000;
		m_maxDensity[i] = -20000;
		m_sumDensity[i] = 0;
		m_densityVariance[i] = 0;
		m_minXCoord[i] = 20000;
		m_maxXCoord[i] = 0;
		m_minYCoord[i] = 20000;
		m_maxYCoord[i] = 0;
		m_minZCoord[i] = 20000;
		m_maxZCoord[i] = 0;
	}

	std::vector<double> sumSqrt(maxId + 1, 0);

	vpl::img::tPixel16 val;

	for (int z = 0; z < zSize; ++z)
	{
		for (int y = 0; y < ySize; ++y)
		{
			vpl::tSize index = rVolume->getIdx(0, y, z);
			vpl::tSize indexDV = spVolume->getIdx(0, y, z);

			for (int x = 0; x < xSize; ++x, index += xOffset, indexDV += xOffsetDV)
			{
				val = rVolume->at(index);

				if (val > 0 && val < maxId + 1)
				{
					++(m_voxelsCnt[val]);

					vpl::img::tDensityPixel density = spVolume->at(indexDV);

					sumSqrt[val] += density * density;

					m_sumDensity[val] += density;

					if (density < m_minDensity[val])
					{
						m_minDensity[val] = density;
					}

					if (density > m_maxDensity[val])
					{
						m_maxDensity[val] = density;
					}

					if (x < m_minXCoord[val])
					{
						m_minXCoord[val] = x;
					}

					if (y < m_minYCoord[val])
					{
						m_minYCoord[val] = y;
					}

					if (z < m_minZCoord[val])
					{
						m_minZCoord[val] = z;
					}

					if (x > m_maxXCoord[val])
					{
						m_maxXCoord[val] = x;
					}

					if (y > m_maxYCoord[val])
					{
						m_maxYCoord[val] = y;
					}

					if (z > m_maxZCoord[val])
					{
						m_maxZCoord[val] = z;
					}
				}
			}
		}
	}

	for (int i = 0; i < maxId + 1; ++i)
	{
		if (m_voxelsCnt[i] == 0)
			continue;

		double tmp = 1.0 / m_voxelsCnt[i];

		m_densityVariance[i] = (sumSqrt[i] * tmp) - (m_sumDensity[i] * tmp * m_sumDensity[i] * tmp);
	}
}

////////////////////////////////////////////////////////////
//

void CRegionDataCalculator::init()
{
	m_voxelsCnt.clear();
	m_minDensity.clear();
	m_maxDensity.clear();
	m_sumDensity.clear();
	m_densityVariance.clear();
	m_minXCoord.clear();
	m_maxXCoord.clear();
	m_minYCoord.clear();
	m_maxYCoord.clear();
	m_minZCoord.clear();
	m_maxZCoord.clear();
}

///////////////////////////////////////////////////////////////////////////////
//

bool CRegionDataCalculator::hasData()
{
	return true;
}

} // namespace data

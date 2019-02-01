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
#include <data/CMultiClassRegionData.h>
#include <data/CDensityData.h>
#include <data/CMultiClassRegionColoring.h>

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
	CObjectPtr< CMultiClassRegionColoring > spColoring(APP_STORAGE.getEntry(Storage::MultiClassRegionColoring::Id));
	CObjectPtr< CMultiClassRegionData > rVolume(APP_STORAGE.getEntry(Storage::MultiClassRegionData::Id));
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

	data::CMultiClassRegionData::tVoxel val;

	for (int z = 0; z < zSize; ++z)
	{
		for (int y = 0; y < ySize; ++y)
		{
			vpl::tSize index = rVolume->getIdx(0, y, z);
			vpl::tSize indexDV = spVolume->getIdx(0, y, z);

			for (int x = 0; x < xSize; ++x, index += xOffset, indexDV += xOffsetDV)
			{
				val = rVolume->at(index);

                if (val > 0)
                {
                    vpl::img::tDensityPixel density = spVolume->at(indexDV);
                    double sgrtDensity = density * density;

                    int pos = 0;

                    while (pos <= maxId)
                    {
                        if (val & 1)
                        {
                            ++(m_voxelsCnt[pos]);

                            sumSqrt[pos] += sgrtDensity;

                            m_sumDensity[pos] += density;

                            if (density < m_minDensity[pos])
                            {
                                m_minDensity[pos] = density;
                            }

                            if (density > m_maxDensity[pos])
                            {
                                m_maxDensity[pos] = density;
                            }

                            if (x < m_minXCoord[pos])
                            {
                                m_minXCoord[pos] = x;
                            }

                            if (y < m_minYCoord[pos])
                            {
                                m_minYCoord[pos] = y;
                            }

                            if (z < m_minZCoord[pos])
                            {
                                m_minZCoord[pos] = z;
                            }

                            if (x > m_maxXCoord[pos])
                            {
                                m_maxXCoord[pos] = x;
                            }

                            if (y > m_maxYCoord[pos])
                            {
                                m_maxYCoord[pos] = y;
                            }

                            if (z > m_maxZCoord[pos])
                            {
                                m_maxZCoord[pos] = z;
                            }
                        }

                        val >>= 1;
                        ++pos;
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

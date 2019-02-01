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

#include <data/COrthoSlice.h>

#include <coremedi/app/Signals.h>
#include <data/CActiveDataSet.h>
#include <data/CRegionData.h>
#include <data/CMultiClassRegionData.h>
#include <data/CVolumeOfInterestData.h>

namespace data
{

///////////////////////////////////////////////////////////////////////////////
//

bool COrthoSlice::checkDependency(CStorageEntry *pParent)
{
    return true;
}

///////////////////////////////////////////////////////////////////////////////
//

void COrthoSlice::init()
{
    CSlice::init();

    m_Position = CSlice::INIT_SIZE / 2;
    m_Mode = DEFAULT_MODE;
}

///////////////////////////////////////////////////////////////////////////////
//

void COrthoSliceXY::update(const CChangedEntries& Changes)
{
    if (!m_updateEnabled)
    {
        return;
    }

    // Get the density data
    int datasetId = VPL_SIGNAL(SigGetActiveDataSet).invoke2();
    if (datasetId == CUSTOM_DATA)
    {
        return;
    }

	bool bRTGDensityWindowChanged(Changes.hasChanged(Storage::RTGDensityWindow::Id));

    CObjectPtr<CDensityData> spVolume(APP_STORAGE.getEntry(datasetId));

    // Modify the slice position
    bool bDataChanged = Changes.hasChanged(Storage::ActiveDataSet::Id);
    if( bDataChanged )
    {
        m_Position = spVolume->getZSize() / 2;
    }
    else
    {
        m_Position = vpl::math::getMax(m_Position, (vpl::tSize)0);
        m_Position = vpl::math::getMin(m_Position, spVolume->getZSize() - 1);
    }

	// Image size
	vpl::tSize XSize = spVolume->getXSize();
	vpl::tSize YSize = spVolume->getYSize();

	CObjectPtr<CVolumeOfInterestData> spVOI(APP_STORAGE.getEntry(Storage::VolumeOfInterestData::Id));

	if (spVOI->isSet())
	{
		if (m_Position >= spVOI->getMinZ() && m_Position <= spVOI->getMaxZ())
		{
			m_minX = spVOI->getMinX();
			m_minY = spVOI->getMinY();
			m_maxX = spVOI->getMaxX();
			m_maxY = spVOI->getMaxY();
		}
		else
		{
			m_minX = spVOI->getMaxX();
			m_minY = spVOI->getMaxY();
			m_maxX = spVOI->getMinX();
			m_maxY = spVOI->getMinY();
		}
	}
	else
	{
		m_minX = 0;
		m_minY = 0;
		m_maxX = XSize;
		m_maxY = YSize;
	}

    // Check the imaging mode
    if( m_Mode == MODE_MIP )
    {
        if( bDataChanged || Changes.checkFlagAny(MODE_CHANGED) )
        {
            updateMIP(*spVolume);
        }
        return;
    }
    else if( m_Mode == MODE_RTG )
    {
        if (bDataChanged || Changes.checkFlagAny(MODE_CHANGED) || bRTGDensityWindowChanged )
        {
            updateRTG(*spVolume);
        }
        return;
    }

    // Has the size changed?
    bool bSizeChanged = (m_DensityData.getXSize() != XSize || m_DensityData.getYSize() != YSize);

    // Get the density data
    m_DensityData.resize(XSize, YSize);
    if( !spVolume->getPlaneXY(m_Position, m_DensityData) )
    {
        return;
    }

    // Unlock the density data
    spVolume.release();

    // Check if region coloring is enabled
//    if( APP_STORAGE.isEntryValid(Storage::RegionData::Id) )
    if( APP_STORAGE.isEntryValid(Storage::RegionData::Id) && VPL_SIGNAL(SigIsRegionColoringEnabled).invoke2() )
    {
        // Get the segmented data
        CObjectPtr<CRegionData> spRegionVolume( APP_STORAGE.getEntry(Storage::RegionData::Id) );

        if (spRegionVolume->hasData())
        {
            if (spRegionVolume->getXSize() == XSize && spRegionVolume->getYSize() == YSize)
            {
                m_RegionData.resize(XSize, YSize);
                if (!spRegionVolume->getPlaneXY(m_Position, m_RegionData))
                {
                    return;
                }
            }
            else
            {
                if (m_RegionData.width() != 0 || m_RegionData.height() != 0)
                    m_RegionData.resize(0, 0);
            }
        }

        // Unlock the region data
        spRegionVolume.release();
    }
    else
    {
        if (m_RegionData.width()!=0 || m_RegionData.height()!=0)
            m_RegionData.resize(0, 0);
    }

    // Check if region coloring is enabled
    if (APP_STORAGE.isEntryValid(Storage::MultiClassRegionData::Id) && VPL_SIGNAL(SigIsMultiClassRegionColoringEnabled).invoke2())
    {
        // Get the segmented data
        CObjectPtr<CMultiClassRegionData> spRegionVolume(APP_STORAGE.getEntry(Storage::MultiClassRegionData::Id));

        if (spRegionVolume->hasData())
        {
            if (spRegionVolume->getXSize() == XSize && spRegionVolume->getYSize() == YSize)
            {
                m_multiClassRegionData.resize(XSize, YSize);
                if (!spRegionVolume->getPlaneXY(m_Position, m_multiClassRegionData))
                {
                    return;
                }
            }
            else
            {
                if (m_multiClassRegionData.width() != 0 || m_multiClassRegionData.height() != 0)
                    m_multiClassRegionData.resize(0, 0);
            }
        }

        // Unlock the region data
        spRegionVolume.release();
    }
    else
    {
        if (m_multiClassRegionData.width() != 0 || m_multiClassRegionData.height() != 0)
            m_multiClassRegionData.resize(0, 0);
    }

    CSlicePropertyContainer::tPropertyList propertyList = m_properties.propertyList();
    for (CSlicePropertyContainer::tPropertyList::iterator it = propertyList.begin(); it != propertyList.end(); ++it)
    {
        (*it)->update(this);
    }

    // Update RGB data
    bSizeChanged = updateRGBData(bSizeChanged, data::Storage::DensityWindow::Id);

    // Update the OSG texture
    updateTexture(bSizeChanged);
}

///////////////////////////////////////////////////////////////////////////////
//

void COrthoSliceXY::updateMIP(const CDensityData& Volume)
{
    // Check the volume size
    if ((Volume.getXSize() <= 0) || (Volume.getYSize() <= 0) || (Volume.getZSize() <= 0))
    {
        return;
    }

    // Has the size changed?
    bool bSizeChanged = (m_DensityData.getXSize() != Volume.getXSize() || m_DensityData.getYSize() != Volume.getYSize());

    // Resize the density image
    m_DensityData.resize(Volume.getXSize(), Volume.getYSize());
    m_RegionData.resize(0, 0);
    m_multiClassRegionData.resize(0, 0);

    // MIP
    for( vpl::tSize j = 0; j < Volume.getYSize(); ++j )
    {
        for( vpl::tSize i = 0; i < Volume.getXSize(); ++i )
        {
            vpl::img::tDensityPixel Max = Volume(i,j,0);
            for( vpl::tSize k = 1; k < Volume.getZSize(); ++k )
            {
                Max = (Volume(i,j,k) > Max) ? Volume(i,j,k) : Max;
            }
            m_DensityData(i,j) = Max;
        }
    }

    // Update rgba data 
	bSizeChanged = this->updateRGBData2(bSizeChanged);

    // Update the OSG texture
    updateTexture(bSizeChanged);
}

///////////////////////////////////////////////////////////////////////////////
//

void COrthoSliceXY::updateRTG(const CDensityData& Volume)
{
	m_minX = 0;
	m_minY = 0;
	m_maxX = Volume.getXSize();
	m_maxY = Volume.getYSize();

    // Check the volume size
    if ((Volume.getXSize() <= 0) || (Volume.getYSize() <= 0) || (Volume.getZSize() <= 0))
    {
        return;
    }

    // Has the size changed?
    bool bSizeChanged = (m_DensityData.getXSize() != Volume.getXSize() || m_DensityData.getYSize() != Volume.getYSize());

    // Resize the density image
    m_DensityData.resize(Volume.getXSize(), Volume.getYSize());
    m_RegionData.resize(0, 0);
    m_multiClassRegionData.resize(0, 0);

    // RTG
#pragma omp parallel for
    for( vpl::tSize j = 0; j < Volume.getYSize(); ++j )
    {
        for( vpl::tSize i = 0; i < Volume.getXSize(); ++i )
        {
            double dSum = 0.0;
            unsigned int samples = 0;
            for( vpl::tSize k = 0; k < Volume.getZSize(); ++k )
            {
                vpl::img::tDensityPixel voxel = Volume(i,j,k);
                if (voxel >= -1000)
                {
                    dSum += voxel;
                    samples++;
                }
            }
            m_DensityData(i,j) = vpl::img::tDensityPixel(samples == 0 ? vpl::img::CPixelTraits<vpl::img::tDensityPixel>::getPixelMin() : dSum / samples);
        }
    }

    // Recalculate density window if it should be used and was not user-modified.
    if (m_bUseDensityWindow)
    {
        // Get the density window
		CObjectPtr<CDensityWindow> spDensityWindow(APP_STORAGE.getEntry(data::Storage::RTGDensityWindow::Id));

        // Estimate optimal settings
        if (!spDensityWindow->wasModified())
            spDensityWindow->estimateOptimal(m_DensityData);
    }

    // Update rgba data 
    if (!m_bUseDensityWindow)
        bSizeChanged = this->updateRGBData2(bSizeChanged);
    else
		bSizeChanged = this->updateRGBData(bSizeChanged, data::Storage::RTGDensityWindow::Id);

    // Update the OSG texture
    updateTexture(bSizeChanged);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//

void COrthoSliceXZ::update(const CChangedEntries& Changes)
{
    if (!m_updateEnabled)
    {
        return;
    }

    // Get the density data
    int datasetId = VPL_SIGNAL(SigGetActiveDataSet).invoke2();
    if (datasetId == CUSTOM_DATA)
    {
        return;
    }

	bool bRTGDensityWindowChanged(Changes.hasChanged(Storage::RTGDensityWindow::Id));

    CObjectPtr<CDensityData> spVolume(APP_STORAGE.getEntry(datasetId));

    // Modify the slice position
    bool bDataChanged = Changes.hasChanged(Storage::ActiveDataSet::Id);
    if( bDataChanged )
    {
        m_Position = spVolume->getYSize() / 2;
    }
    else
    {
        m_Position = vpl::math::getMax(m_Position, (vpl::tSize)0);
        m_Position = vpl::math::getMin(m_Position, spVolume->getYSize() - 1);
    }

	// Image size
	vpl::tSize XSize = spVolume->getXSize();
	vpl::tSize ZSize = spVolume->getZSize();

	CObjectPtr<CVolumeOfInterestData> spVOI(APP_STORAGE.getEntry(Storage::VolumeOfInterestData::Id));

	if (spVOI->isSet())
	{
		if (m_Position >= spVOI->getMinY() && m_Position <= spVOI->getMaxY())
		{
			m_minX = spVOI->getMinX();
			m_minY = spVOI->getMinZ();
			m_maxX = spVOI->getMaxX();
			m_maxY = spVOI->getMaxZ();
		}
		else
		{
			m_minX = spVOI->getMaxX();
			m_minY = spVOI->getMaxZ();
			m_maxX = spVOI->getMinX();
			m_maxY = spVOI->getMinZ();
		}
	}
	else
	{
		m_minX = 0;
		m_minY = 0;
		m_maxX = XSize;
		m_maxY = ZSize;
	}

    // Check the imaging mode
    if( m_Mode == MODE_MIP )
    {
        if( bDataChanged || Changes.checkFlagAny(MODE_CHANGED) )
        {
            updateMIP(*spVolume);
        }
        return;
    }
    else if( m_Mode == MODE_RTG )
    {
        if (bDataChanged || Changes.checkFlagAny(MODE_CHANGED) || bRTGDensityWindowChanged)
        {
            updateRTG(*spVolume);
        }
        return;
    }

    // Has the size changed?
    bool bSizeChanged = (m_DensityData.getXSize() != XSize || m_DensityData.getYSize() != ZSize);

    // Get the density data
    m_DensityData.resize(XSize, ZSize);
    if( !spVolume->getPlaneXZ(m_Position, m_DensityData) )
    {
        return;
    }

    // Unlock the density data
    spVolume.release();

    // Check if region coloring is enabled
//    if( APP_STORAGE.isEntryValid(Storage::RegionData::Id) )
    if( APP_STORAGE.isEntryValid(Storage::RegionData::Id) && VPL_SIGNAL(SigIsRegionColoringEnabled).invoke2() )
    {
        // Get the segmented data
        CObjectPtr<CRegionData> spRegionVolume( APP_STORAGE.getEntry(Storage::RegionData::Id) );

        if(spRegionVolume->getXSize() == XSize && spRegionVolume->getZSize() == ZSize)
        {
            m_RegionData.resize(XSize, ZSize);
            if( !spRegionVolume->getPlaneXZ(m_Position, m_RegionData) )
            {
                return;
            }
        }
        else
        {
            if (m_RegionData.width()!=0 || m_RegionData.height()!=0)
                m_RegionData.resize(0, 0);
        }

        // Unlock the region data
        spRegionVolume.release();
    }
    else
    {
        if (m_RegionData.width()!=0 || m_RegionData.height()!=0)
            m_RegionData.resize(0, 0);
    }

    // Check if region coloring is enabled
    if (APP_STORAGE.isEntryValid(Storage::MultiClassRegionData::Id) && VPL_SIGNAL(SigIsMultiClassRegionColoringEnabled).invoke2())
    {
        // Get the segmented data
        CObjectPtr<CMultiClassRegionData> spRegionVolume(APP_STORAGE.getEntry(Storage::MultiClassRegionData::Id));

        if (spRegionVolume->hasData())
        {
            if (spRegionVolume->getXSize() == XSize && spRegionVolume->getZSize() == ZSize)
            {
                m_multiClassRegionData.resize(XSize, ZSize);
                if (!spRegionVolume->getPlaneXZ(m_Position, m_multiClassRegionData))
                {
                    return;
                }
            }
            else
            {
                if (m_multiClassRegionData.width() != 0 || m_multiClassRegionData.height() != 0)
                    m_multiClassRegionData.resize(0, 0);
            }
        }

        // Unlock the region data
        spRegionVolume.release();
    }
    else
    {
        if (m_multiClassRegionData.width() != 0 || m_multiClassRegionData.height() != 0)
            m_multiClassRegionData.resize(0, 0);
    }

    CSlicePropertyContainer::tPropertyList propertyList = m_properties.propertyList();
    for (CSlicePropertyContainer::tPropertyList::iterator it = propertyList.begin(); it != propertyList.end(); ++it)
    {
        (*it)->update(this);
    }

    // Update RGB data
    bSizeChanged = updateRGBData(bSizeChanged, data::Storage::DensityWindow::Id);

    // Update the OSG texture
    updateTexture(bSizeChanged);
}

///////////////////////////////////////////////////////////////////////////////
//

void COrthoSliceXZ::updateMIP(const CDensityData& Volume)
{
    // Check the volume size
    if ((Volume.getXSize() <= 0) || (Volume.getYSize() <= 0) || (Volume.getZSize() <= 0))
    {
        return;
    }

    // Has the size changed?
    bool bSizeChanged = (m_DensityData.getXSize() != Volume.getXSize() || m_DensityData.getYSize() != Volume.getZSize());

    // Resize the density image
    m_DensityData.resize(Volume.getXSize(), Volume.getZSize());
    m_RegionData.resize(0, 0);
    m_multiClassRegionData.resize(0, 0);

    // MIP
    for( vpl::tSize j = 0; j < Volume.getZSize(); ++j )
    {
        for( vpl::tSize i = 0; i < Volume.getXSize(); ++i )
        {
            vpl::img::tDensityPixel Max = Volume(i,0,j);
            for( vpl::tSize k = 1; k < Volume.getYSize(); ++k )
            {
                Max = (Volume(i,k,j) > Max) ? Volume(i,k,j) : Max;
            }
            m_DensityData(i,j) = Max;
        }
    }

    // Update RGB data
    bSizeChanged = updateRGBData2(bSizeChanged);

    // Update the OSG texture
    updateTexture(bSizeChanged);
}

///////////////////////////////////////////////////////////////////////////////
//

void COrthoSliceXZ::updateRTG(const CDensityData& Volume)
{
	m_minX = 0;
	m_minY = 0;
	m_maxX = Volume.getXSize();
	m_maxY = Volume.getZSize();

    // Check the volume size
    if ((Volume.getXSize() <= 0) || (Volume.getYSize() <= 0) || (Volume.getZSize() <= 0))
    {
        return;
    }

    // Has the size changed?
    bool bSizeChanged = (m_DensityData.getXSize() != Volume.getXSize() || m_DensityData.getYSize() != Volume.getZSize());

    // Resize the density image
    m_DensityData.resize(Volume.getXSize(), Volume.getZSize());
    m_RegionData.resize(0, 0);
    m_multiClassRegionData.resize(0, 0);

    // RTG
    // - Use "density window" to fill the RGB image
#pragma omp parallel for
    for( vpl::tSize j = 0; j < Volume.getZSize(); ++j )
    {
        for( vpl::tSize i = 0; i < Volume.getXSize(); ++i )
        {
            double dSum = 0.0;
            unsigned int samples = 0;
            for( vpl::tSize k = 0; k < Volume.getYSize(); ++k )
            {
                vpl::img::tDensityPixel voxel = Volume(i,k,j);
                if (voxel >= -1000)
                {
                    dSum += voxel;
                    samples++;
                }
            }
            m_DensityData(i,j) = vpl::img::tDensityPixel(samples == 0 ? vpl::img::CPixelTraits<vpl::img::tDensityPixel>::getPixelMin() : dSum / samples);
        }
    }

    // Recalculate density window if it should be used and was not user-modified.
    if (m_bUseDensityWindow)
    {
        // Get the density window
		CObjectPtr<CDensityWindow> spDensityWindow(APP_STORAGE.getEntry(data::Storage::RTGDensityWindow::Id));

        // Estimate optimal settings
        if (!spDensityWindow->wasModified())
            spDensityWindow->estimateOptimal(m_DensityData);
    }

    // Update rgba data 
    if (!m_bUseDensityWindow)
        bSizeChanged = this->updateRGBData2(bSizeChanged);
    else
		bSizeChanged = this->updateRGBData(bSizeChanged, data::Storage::RTGDensityWindow::Id);

    // Update the OSG texture
    updateTexture(bSizeChanged);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//

void COrthoSliceYZ::update(const CChangedEntries& Changes)
{
    if (!m_updateEnabled)
    {
        return;
    }

    // Get the density data
    int datasetId = VPL_SIGNAL(SigGetActiveDataSet).invoke2();
    if (datasetId == CUSTOM_DATA)
    {
        return;
    }

    bool bRTGDensityWindowChanged(Changes.hasChanged(Storage::RTGDensityWindow::Id));

    CObjectPtr<CDensityData> spVolume(APP_STORAGE.getEntry(datasetId));

    // Modify the slice position
    bool bDataChanged = Changes.hasChanged(Storage::ActiveDataSet::Id);
    if( bDataChanged )
    {
        m_Position = spVolume->getXSize() / 2;
    }
    else
    {
        m_Position = vpl::math::getMax(m_Position, (vpl::tSize)0);
        m_Position = vpl::math::getMin(m_Position, spVolume->getXSize() - 1);
    }

	// Image size
	vpl::tSize YSize = spVolume->getYSize();
	vpl::tSize ZSize = spVolume->getZSize();

	CObjectPtr<CVolumeOfInterestData> spVOI(APP_STORAGE.getEntry(Storage::VolumeOfInterestData::Id));

	if (spVOI->isSet())
	{
		if (m_Position >= spVOI->getMinX() && m_Position <= spVOI->getMaxX())
		{
			m_minX = spVOI->getMinY();
			m_minY = spVOI->getMinZ();
			m_maxX = spVOI->getMaxY();
			m_maxY = spVOI->getMaxZ();
		}
		else
		{
			m_minX = spVOI->getMaxY();
			m_minY = spVOI->getMaxZ();
			m_maxX = spVOI->getMinY();
			m_maxY = spVOI->getMinZ();
		}
	}
	else
	{
		m_minX = 0;
		m_minY = 0;
		m_maxX = YSize;
		m_maxY = ZSize;
	}

    // Check the imaging mode
    if( m_Mode == MODE_MIP )
    {
        if( bDataChanged || Changes.checkFlagAny(MODE_CHANGED) )
        {
            updateMIP(*spVolume);
        }
        return;
    }
    else if( m_Mode == MODE_RTG )
    {
        if( bDataChanged || Changes.checkFlagAny(MODE_CHANGED) || bRTGDensityWindowChanged )
        {
            updateRTG(*spVolume);
        }
        return;
    }

    // Has the size changed?
    bool bSizeChanged = (m_DensityData.getXSize() != YSize || m_DensityData.getYSize() != ZSize);

    // Get the density data
    m_DensityData.resize(YSize, ZSize);
    if( !spVolume->getPlaneYZ(m_Position, m_DensityData) )
    {
        return;
    }

    // Unlock the density data
    spVolume.release();

    // Check if region coloring is enabled
//    if( APP_STORAGE.isEntryValid(Storage::RegionData::Id) )
    if( APP_STORAGE.isEntryValid(Storage::RegionData::Id) && VPL_SIGNAL(SigIsRegionColoringEnabled).invoke2() )
    {
        // Get the segmented data
        CObjectPtr<CRegionData> spRegionVolume( APP_STORAGE.getEntry(Storage::RegionData::Id) );

        // Check if region coloring is enabled
        if(spRegionVolume->getYSize() == YSize && spRegionVolume->getZSize() == ZSize )
        {
            m_RegionData.resize(YSize, ZSize);
            if( !spRegionVolume->getPlaneYZ(m_Position, m_RegionData) )
            {
                return;
            }
        }
        else
        {
            if (m_RegionData.width()!=0 || m_RegionData.height()!=0)
                m_RegionData.resize(0, 0);
        }
        
        // Unlock the region data
        spRegionVolume.release();
    }
    else
    {
        if (m_RegionData.width()!=0 || m_RegionData.height()!=0)
            m_RegionData.resize(0, 0);
    }

    // Check if region coloring is enabled
    if (APP_STORAGE.isEntryValid(Storage::MultiClassRegionData::Id) && VPL_SIGNAL(SigIsMultiClassRegionColoringEnabled).invoke2())
    {
        // Get the segmented data
        CObjectPtr<CMultiClassRegionData> spRegionVolume(APP_STORAGE.getEntry(Storage::MultiClassRegionData::Id));

        if (spRegionVolume->hasData())
        {
            if (spRegionVolume->getYSize() == YSize && spRegionVolume->getZSize() == ZSize)
            {
                m_multiClassRegionData.resize(YSize, ZSize);
                if (!spRegionVolume->getPlaneYZ(m_Position, m_multiClassRegionData))
                {
                    return;
                }
            }
            else
            {
                if (m_multiClassRegionData.width() != 0 || m_multiClassRegionData.height() != 0)
                    m_multiClassRegionData.resize(0, 0);
            }
        }

        // Unlock the region data
        spRegionVolume.release();
    }
    else
    {
        if (m_multiClassRegionData.width() != 0 || m_multiClassRegionData.height() != 0)
            m_multiClassRegionData.resize(0, 0);
    }

    CSlicePropertyContainer::tPropertyList propertyList = m_properties.propertyList();
    for (CSlicePropertyContainer::tPropertyList::iterator it = propertyList.begin(); it != propertyList.end(); ++it)
    {
        (*it)->update(this);
    }

    // Update RGB data
    bSizeChanged = updateRGBData(bSizeChanged, data::Storage::DensityWindow::Id);

    // Update the OSG texture
    updateTexture(bSizeChanged);
}

///////////////////////////////////////////////////////////////////////////////
//

void COrthoSliceYZ::updateMIP(const CDensityData& Volume)
{
    // Check the volume size
    if ((Volume.getXSize() <= 0) || (Volume.getYSize() <= 0) || (Volume.getZSize() <= 0))
    {
        return;
    }

    // Has the size changed?
    bool bSizeChanged = (m_DensityData.getXSize() != Volume.getYSize() || m_DensityData.getYSize() != Volume.getZSize());

    // Resize the density image
    m_DensityData.resize(Volume.getYSize(), Volume.getZSize());
    m_RegionData.resize(0, 0);
    m_multiClassRegionData.resize(0, 0);

    // MIP
    for( vpl::tSize j = 0; j < Volume.getZSize(); ++j )
    {
        for( vpl::tSize i = 0; i < Volume.getYSize(); ++i )
        {
            vpl::img::tDensityPixel Max = Volume(0,i,j);
            for( vpl::tSize k = 1; k < Volume.getXSize(); ++k )
            {
                Max = (Volume(k,i,j) > Max) ? Volume(k,i,j) : Max;
            }
            m_DensityData(i,j) = Max;
        }
    }

    // Update RGB data
    bSizeChanged = updateRGBData2(bSizeChanged);

    // Update the OSG texture
    updateTexture(bSizeChanged);
}

///////////////////////////////////////////////////////////////////////////////
//

void COrthoSliceYZ::updateRTG(const CDensityData& Volume)
{
	m_minX = 0;
	m_minY = 0;
	m_maxX = Volume.getYSize();
	m_maxY = Volume.getZSize();

    // Check the volume size
    if ((Volume.getXSize() <= 0) || (Volume.getYSize() <= 0) || (Volume.getZSize() <= 0))
    {
        return;
    }

    // Has the size changed?
    bool bSizeChanged = (m_DensityData.getXSize() != Volume.getYSize() || m_DensityData.getYSize() != Volume.getZSize());

    // Resize the density image
    m_DensityData.resize(Volume.getYSize(), Volume.getZSize());
    m_RegionData.resize(0, 0);
    m_multiClassRegionData.resize(0, 0);

    // RTG
    // - Use "density window" to fill the RGB image
#pragma omp parallel for
    for( vpl::tSize j = 0; j < Volume.getZSize(); ++j )
    {
        for( vpl::tSize i = 0; i < Volume.getYSize(); ++i )
        {
            double dSum = 0.0;
            unsigned int samples = 0;
            for( vpl::tSize k = 0; k < Volume.getXSize(); ++k )
            {
                vpl::img::tDensityPixel voxel = Volume(k,i,j);
                if (voxel >= -1000)
                {
                    dSum += voxel;
                    samples++;
                }
            }
            m_DensityData(i,j) = vpl::img::tDensityPixel(samples == 0 ? vpl::img::CPixelTraits<vpl::img::tDensityPixel>::getPixelMin() : dSum / samples);
        }
    }

    // Recalculate density window if it should be used and was not user-modified.
    if (m_bUseDensityWindow)
    {
        // Get the density window
		CObjectPtr<CDensityWindow> spDensityWindow(APP_STORAGE.getEntry(data::Storage::RTGDensityWindow::Id));

        // Estimate optimal settings
        if (!spDensityWindow->wasModified())
            spDensityWindow->estimateOptimal(m_DensityData);
    }

    // Update rgba data 
    if (!m_bUseDensityWindow)
        bSizeChanged = this->updateRGBData2(bSizeChanged);
    else
		bSizeChanged = this->updateRGBData(bSizeChanged, data::Storage::RTGDensityWindow::Id);


    // Update the OSG texture
    updateTexture(bSizeChanged);
}


} // namespace data

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

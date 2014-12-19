///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// 3DimViewer
// Lightweight 3D DICOM viewer.
//
// Copyright 2008-2012 3Dim Laboratory s.r.o.
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

#include <data/CSlice.h>
#include <data/CDensityWindow.h>
#include <data/CRegionColoring.h>
#include <data/CAppSettings.h>

#include <VPL/Image/ImageFunctions.h>
#include <VPL/Image/ImageFiltering.h>


namespace data
{

///////////////////////////////////////////////////////////////////////////////
//

CSlice::CSlice()
    : m_DensityData(INIT_SIZE, INIT_SIZE, 0)
    , m_RegionData(INIT_SIZE, INIT_SIZE, 0)
    , m_RGBData(INIT_SIZE, INIT_SIZE, 0)
    , m_spImage(new osg::Image)
    , m_spTexture(new tTexture)
    , m_fTextureWidth(1.0f)
    , m_fTextureHeight(1.0f)
{
    init();
}

///////////////////////////////////////////////////////////////////////////////
//

void CSlice::init()
{
    // Initialize the image data
    m_DensityData.resize(INIT_SIZE, INIT_SIZE);
    m_RegionData.resize(INIT_SIZE, INIT_SIZE);
    m_RGBData.resize(INIT_SIZE, INIT_SIZE);

    m_DensityData.fillEntire(vpl::img::CPixelTraits<vpl::img::tDensityPixel>::getPixelMin());
    m_RegionData.fillEntire(vpl::img::CPixelTraits<vpl::img::tDensityPixel>::getPixelMin());
    m_RGBData.fillEntire(vpl::img::CPixelTraits<vpl::img::tRGBPixel>::getPixelMin());

    // Protect from being optimized away as static state.
    m_spTexture->setDataVariance(osg::Object::DYNAMIC);
    m_spTexture->setImage(m_spImage.get());
    m_fTextureWidth = 1.0f;
    m_fTextureHeight = 1.0f;

    // Texture properties
    setupTexture();

    // Generate texture
    updateTexture(true);
}

///////////////////////////////////////////////////////////////////////////////
//

//void CSlice::update(const CChangedEntries& Changes)
//{
//}

///////////////////////////////////////////////////////////////////////////////
//

CSlice::~CSlice()
{ }

void CSlice::disconnectProperties()
{
    CSlicePropertyContainer::tPropertyList propertyList = m_properties.propertyList();
    for (CSlicePropertyContainer::tPropertyList::iterator it = propertyList.begin(); it != propertyList.end(); ++it)
    {
        if (APP_STORAGE.isEntryValid((*it)->propertySourceStorageId()) && m_signalConnections.find((*it)->name()) != m_signalConnections.end())
        {
            APP_STORAGE.getEntrySignal((*it)->propertySourceStorageId()).disconnect(m_signalConnections[(*it)->name()]);
            m_signalConnections.erase((*it)->name());
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//

void CSlice::setupTexture()
{
    m_spTexture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
    m_spTexture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
    m_spTexture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP);
    m_spTexture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP);
    m_spTexture->setUseHardwareMipMapGeneration(false);
    m_spTexture->setMaxAnisotropy(1.0f);
    m_spTexture->setResizeNonPowerOfTwoHint(false); 
}

///////////////////////////////////////////////////////////////////////////////
//

void CSlice::estimateTextureSize(vpl::tSize& Width, vpl::tSize& Height)
{
    CObjectPtr<CAppSettings> spAppSettings( APP_STORAGE.getEntry(Storage::AppSettings::Id) );
    if (spAppSettings->getNPOTTextures())
    {   // align them to multiple of 4
        vpl::tSize newWidth = (Width + 3) & ~3;
        vpl::tSize newHeight = (Height + 3) & ~3;
        m_fTextureWidth = float(Width) / newWidth;
        m_fTextureHeight = float(Height) / newHeight;
        Width = newWidth;
        Height = newHeight;
    }
    else // create square textures with the size of power of two
    {
        // Greater value
        vpl::tSize Max = vpl::math::getMax(Width, Height);

        // Texture dimensions
    //    vpl::tSize TexSize = 128;
        vpl::tSize TexSize = 16;
        while( TexSize < Max )
        {
            TexSize <<= 1;
        }

        // Used texture size
        float fInvTexSize = 1.0f / float(TexSize);
        m_fTextureWidth = fInvTexSize * float(Width);
        m_fTextureHeight = fInvTexSize * float(Height);

        // Modify the image dimension
        Width = Height = TexSize;
    }
}

///////////////////////////////////////////////////////////////////////////////
//

//! Applies current selected filter on m_DensityData and returns backup of original data
vpl::img::CDImage* CSlice::applyFilterAndGetBackup()
{
    CObjectPtr<CAppSettings> spAppSettings( APP_STORAGE.getEntry(Storage::AppSettings::Id) );
    CAppSettings::ETextureFilter filter=spAppSettings->getFilter();
    if (filter==CAppSettings::Blur)
    {
        vpl::img::CDImage *pDataWithMargin = new vpl::img::CDImage(m_DensityData.getSize(),5);
        *pDataWithMargin = m_DensityData;
        pDataWithMargin->mirrorMargin();
        vpl::img::CGaussFilter<vpl::img::CDImage> Filter(5);
        Filter(*pDataWithMargin,m_DensityData);
        return pDataWithMargin;
    }
    if (filter==CAppSettings::Sharpen)
    {
        vpl::img::CDImage *pDataWithMargin = new vpl::img::CDImage(m_DensityData.getSize(),5);
        *pDataWithMargin = m_DensityData;
        pDataWithMargin->mirrorMargin();
        // blur
        vpl::img::CDImage tmp(m_DensityData);
        vpl::img::CGaussFilter<vpl::img::CDImage> Filter(5);
        Filter(*pDataWithMargin,tmp);
        // perform unsharp mask
        m_DensityData-=tmp;
        m_DensityData*=vpl::CScalari(3);
        m_DensityData+=*pDataWithMargin;
        return pDataWithMargin;
    }
    if (filter==CAppSettings::SmoothSharpen)
    {
        vpl::img::CDImage *pDataWithMargin = new vpl::img::CDImage(m_DensityData.getSize(),5);
        *pDataWithMargin = m_DensityData;
        pDataWithMargin->mirrorMargin();
        // blur
        vpl::img::CDImage tmp(m_DensityData.getSize(),5);
        tmp=m_DensityData;
        vpl::img::CGaussFilter<vpl::img::CDImage> Filter(5);
        Filter(*pDataWithMargin,m_DensityData);
        // perform unsharp mask
        tmp-=m_DensityData;
        tmp*=vpl::CScalari(4);
        tmp+=*pDataWithMargin;
        // blur the result
        tmp.mirrorMargin();
        Filter(tmp,m_DensityData);
        return pDataWithMargin;
    }
    return NULL;
}

//! Restores m_DensityData and deletes backup
void CSlice::restoreFromBackupAndFree(vpl::img::CDImage* pBackup)
{
    if (NULL!=pBackup)
    {
        m_DensityData = *pBackup;
        delete pBackup;
    }
}

///////////////////////////////////////////////////////////////////////////////
//
void CSlice::onPropertySourceChanged(data::CStorageEntry *entry)
{
    data::CChangedEntries changes;
    entry->getChanges(changes);
    update(changes);

    std::set<int> invalidateIds;
    CSlicePropertyContainer::tPropertyList propertyList = m_properties.propertyList();
    for (CSlicePropertyContainer::tPropertyList::iterator it = propertyList.begin(); it != propertyList.end(); ++it)
    {
        (*it)->propertySourceStorageId();
        if (changes.hasChanged((*it)->propertySourceStorageId()))
        {
            invalidateIds.insert((*it)->sliceStorageId());
        }
    }
    for (std::set<int>::iterator it = invalidateIds.begin(); it != invalidateIds.end(); ++it)
    {
        data::CPtrWrapper<data::CStorageEntry> spEntry(APP_STORAGE.getEntry(*it));
        APP_STORAGE.invalidate(spEntry.get());
    }
}

///////////////////////////////////////////////////////////////////////////////
//

bool CSlice::updateRGBData(bool bSizeChanged)
{
    // Has the size changed?
    if( bSizeChanged || m_DensityData.getXSize() != m_RGBData.getXSize() || m_DensityData.getYSize() != m_RGBData.getYSize() )
    {
        // Estimate optimal texture size
        vpl::tSize Width = m_DensityData.getXSize();
        vpl::tSize Height = m_DensityData.getYSize();
        estimateTextureSize(Width, Height);

        // Resize the RGB image
        m_RGBData.resize(Width, Height, 0);
        m_RGBData.fillEntire(vpl::img::CPixelTraits<vpl::img::tRGBPixel>::getPixelMin());
        bSizeChanged = true;
    }

    // Get the density window
    CObjectPtr<CDensityWindow> spDensityWindow( APP_STORAGE.getEntry(Storage::DensityWindow::Id) );

    vpl::img::CDImage *pBackup = applyFilterAndGetBackup();

    // Is region coloring enabled?
    if( m_DensityData.getXSize() != m_RegionData.getXSize() || m_DensityData.getYSize() != m_RegionData.getYSize() )
    {
        spDensityWindow->colorize(m_RGBData, m_DensityData, m_properties);
    }
    else
    {
        // Get the region coloring object
        CObjectPtr<CRegionColoring> spColoring( APP_STORAGE.getEntry(Storage::RegionColoring::Id) );

        spDensityWindow->colorize(m_RGBData, m_DensityData, m_properties);
        spColoring->colorize(m_RGBData, m_RegionData);
    }

    restoreFromBackupAndFree(pBackup);

    return bSizeChanged;
}


///////////////////////////////////////////////////////////////////////////////
//

bool CSlice::updateRGBData2(bool bSizeChanged)
{
    // Size changed?
    if( bSizeChanged || m_DensityData.getXSize() != m_RGBData.getXSize() || m_DensityData.getYSize() != m_RGBData.getYSize() )
    {
        // Estimate optimal texture size
        vpl::tSize Width = m_DensityData.getXSize();
        vpl::tSize Height = m_DensityData.getYSize();
        estimateTextureSize(Width, Height);

        // Resize the RGB image
        m_RGBData.resize(Width, Height, 0);
        m_RGBData.fillEntire(vpl::img::CPixelTraits<vpl::img::tRGBPixel>::getPixelMin());
        bSizeChanged = true;
    }

    // Linear contrast enhancement
    vpl::img::tDensityPixel Min = 0;
    vpl::img::tDensityPixel Max = 32767;

    if(2==sizeof(vpl::img::tDensityPixel))
    {
        // compute histogram of all pixels that contain some data
        bool valid = false;
        int iCount = 0;
        int histogram[65536]={};
        vpl::img::tDensityPixel v0 = vpl::img::CPixelTraits<vpl::img::tDensityPixel>::getPixelMin();
        for( vpl::tSize j = 0; j < m_DensityData.getYSize(); ++j )
        {
            for( vpl::tSize i = 0; i < m_DensityData.getXSize(); ++i )
            {
                vpl::img::tDensityPixel Value = m_DensityData(i, j);

                // ignore useless values
                if (Value == vpl::img::CPixelTraits<vpl::img::tDensityPixel>::getPixelMin())
                {
                    continue;
                }

                // set reference value for validity consideration
                if (v0 == vpl::img::CPixelTraits<vpl::img::tDensityPixel>::getPixelMin())
                {
                    v0 = Value;
                }

                // check for validity of histogram
                if (Value != v0)
                {
                    valid = true;
                }

                histogram[(int)Value+32768]++; // because tDensityPixel is signed int
                ++iCount;
            }
        }
        // if any valid, find black and white point so tjat
        if (valid)
        {
            int nMin = 32768-1500;
            int nMax = 65535;
            int threshold = 0.02 * iCount; // 2% are clipped
            int sMin = histogram[nMin];
            while(sMin<threshold)
            {
                nMin++;
                sMin+=histogram[nMin];
            }

            int sMax = histogram[65535];
            while(sMax<threshold && nMax>Min)
            {
                nMax--;
                sMax+=histogram[nMax];
            }
            Min=nMin-32768;
            Max=nMax-32768;
        }
    }
    else
    {
        // original method which computes mean and squared variance
        int iCount = 0;
        double dSum = 0.0, dSumSqr = 0.0;
        for( vpl::tSize j = 0; j < m_DensityData.getYSize(); ++j )
        {
            for( vpl::tSize i = 0; i < m_DensityData.getXSize(); ++i )
            {
                vpl::img::tDensityPixel Value = m_DensityData(i, j);
                if( Value != m_DensityData(0, 0) )
                {
                    dSum += Value;
                    dSumSqr += double(Value) * Value;
                    ++iCount;
                }
            }
        }
        double dMean = 0.0, dVar = 0.0;
        if( iCount > 0 )
        {
            double dInvCount = 1.0 / iCount;
            dMean = dSum * dInvCount;
            dVar =  dSumSqr * dInvCount - (dMean * dMean);
        }
        double dWidth = 1.5 * std::sqrt(dVar) + 0.001;

        Min = vpl::img::tDensityPixel(dMean - dWidth);
        Max = vpl::img::tDensityPixel(dMean + dWidth);
    }

    // Normalization
    double dNorm = 255.0 / ((Max-Min) + 0.001);

    vpl::img::CDImage *pBackup = applyFilterAndGetBackup();

    // Prepare the RGB data
    for( vpl::tSize j = 0; j < m_DensityData.getYSize(); ++j )
    {
        for( vpl::tSize i = 0; i < m_DensityData.getXSize(); ++i )
        {
            vpl::img::tDensityPixel Value = m_DensityData(i,j);
            if( Value <= Min )
            {
                m_RGBData(i,j) = vpl::img::tRGBPixel(0, 0, 0);
            }
            else if( Value >= Max )
            {
                m_RGBData(i,j) = vpl::img::tRGBPixel(255, 255, 255);
            }
            else
            {
                vpl::img::tPixel8 NewValue = vpl::img::tPixel8(dNorm * (Value - Min));
                m_RGBData(i,j) = vpl::img::tRGBPixel(NewValue, NewValue, NewValue);
            }
        }
    }

    restoreFromBackupAndFree(pBackup);

    return bSizeChanged;
}

///////////////////////////////////////////////////////////////////////////////
//

void CSlice::updateTexture(bool bRecreateImage)
{
    // Check the data
    if( !bRecreateImage )
    {
        // Force osg::Texture to reload the image
        m_spImage->dirty();
        return;
    }
    
    // Change the data
    m_spImage->setImage(m_RGBData.getXSize(),
                        m_RGBData.getYSize(),
                        0,
                        4,
                        GL_RGBA,
                        GL_UNSIGNED_BYTE, 
                        (unsigned char *)m_RGBData.getPtr(0, 0),
                        osg::Image::NO_DELETE
                        );

    // Force osg::Texture to reload the image
    m_spImage->dirty();

    // Setup texture parameters
//    setupTexture();
    m_spTexture->dirtyTextureObject();
    m_spTexture->dirtyTextureParameters();
}


} // namespace data

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


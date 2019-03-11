///////////////////////////////////////////////////////////////////////////////
// 
// Copyright 2008-2016 3Dim Laboratory s.r.o.
//

#include "CRegion3DPreviewManager.h"
#include <VPL/System/Sleep.h>
#include <VPL/System/ScopedLock.h>
#include <data/CDrawingOptions.h>

namespace data
{

CRegion3DPreviewManager::CRegion3DPreviewManager()
    : m_mutex(false)
    , m_thread(backgroundWorker, this, false)
    , onStart(NULL)
    , onUpdate(NULL)
    , onStop(NULL)
    , m_dataChanged(false)
    , m_canUpdate(true)
    , m_redrawInterval(2)
{
    m_container = new geometry::CTrianglesContainer();
}

void CRegion3DPreviewManager::init(const vpl::img::CSize3d &voxelSize, const data::CMultiClassRegionData::tVoxel &bitIndex)
{
    m_mutex.lock();
    m_container->init();
    m_bitIndex = bitIndex;
    m_voxelSize = voxelSize;
    m_mutex.unlock();
}

CRegion3DPreviewManager::~CRegion3DPreviewManager()
{
    m_thread.terminate(true);
    delete m_container;
}

void CRegion3DPreviewManager::run()
{
    m_thread.resume();
    m_mutex.lock();
    m_condition.notifyOne();
    m_mutex.unlock();
}

void CRegion3DPreviewManager::stop()
{
    m_thread.suspend();
}

void CRegion3DPreviewManager::setRegionIndex(const data::CMultiClassRegionData::tVoxel &bitIndex)
{
    m_mutex.lock();
    m_bitIndex = bitIndex;
    m_mutex.unlock();
}

void CRegion3DPreviewManager::setRedrawInterval(int value)
{
    m_mutex.lock();
    m_redrawInterval = value;
    m_mutex.unlock();
}

void CRegion3DPreviewManager::regionDataChanged()
{
    m_mutex.lock();
    m_dataChanged = true;
    m_mutex.unlock();
}

void CRegion3DPreviewManager::setCanUpdate(bool canUpdate)
{
    m_mutex.lock();
    m_canUpdate = canUpdate;
    m_mutex.unlock();
}

void CRegion3DPreviewManager::startProgress()
{
    if (onStart != NULL)
    {
        (*onStart)();
    }
}

void CRegion3DPreviewManager::updateProgress()
{
    if (onUpdate != NULL)
    {
        (*onUpdate)();
    }
}

void CRegion3DPreviewManager::stopProgress()
{
    if (onStop != NULL)
    {
        (*onStop)(*m_container);
        m_container->init();
    }
}

VPL_THREAD_ROUTINE(CRegion3DPreviewManager::backgroundWorker)
{
    // Console object
    CRegion3DPreviewManager *pManager = static_cast<CRegion3DPreviewManager *>(pThread->getData());
    if (!pManager)
    {
        return -1;
    }

    // Main thread loop
    VPL_THREAD_MAIN_LOOP
    {
        // Wait for the data changed or "anything changed" event
        pManager->m_mutex.lock();

        if (!pManager->m_canUpdate)
        {
            pManager->m_mutex.unlock();
            vpl::sys::sleep(500);
            continue;
        }

        if (!pManager->m_dataChanged)
        {
            if (!pManager->m_condition.wait(pManager->m_mutex, 250))
            {
                int redraw = pManager->m_redrawInterval;
                pManager->m_mutex.unlock();
                vpl::sys::sleep(redraw * 1000);
                continue;
            }
        }

        pManager->m_dataChanged = false;
        data::CMultiClassRegionData::tVoxel bitIndex = pManager->m_bitIndex;
        geometry::CTrianglesContainer* container = pManager->m_container;
        container->init();
        vpl::img::CSize3d voxelSize = pManager->m_voxelSize;
        int redraw = pManager->m_redrawInterval;

        pManager->m_mutex.unlock();

        data::CObjectPtr<data::CMultiClassRegionData> spRegionData(APP_STORAGE.getEntry(data::Storage::MultiClassRegionData::Id, data::Storage::NO_UPDATE));
        vpl::sys::CMutex &sizeMutex = spRegionData->getSizeMutex();
        sizeMutex.lock();
        data::CMultiClassRegionData& volume = *spRegionData.get();
        spRegionData.release();

        if (bitIndex >= 0)
        {
            CBitLayerSelectFunctorPreview< data::CBitVolume<data::CMultiClassRegionData::tVoxel>, data::CMultiClassRegionData::tVoxel > functor(bitIndex, &volume, voxelSize);

            CMarchingCubesFast<geometry::CTrianglesContainer> marchingCubes;
            if (marchingCubes.generateMesh(container, &functor))
            {
                pManager->stopProgress();
            }
        }

        sizeMutex.unlock();

        // Sleep for a short period of time
        vpl::sys::sleep(redraw * 1000);
    }

    return 0;
}

}

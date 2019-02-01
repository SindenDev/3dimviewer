///////////////////////////////////////////////////////////////////////////////
// 
// Copyright 2008-2016 3Dim Laboratory s.r.o.
//

#ifndef CRegion3DPreviewManager_H
#define CRegion3DPreviewManager_H

#include <VPL/Base/Lock.h>
#include <VPL/System/Thread.h>
#include <VPL/System/Condition.h>
#include <VPL/System/Mutex.h>
#include <data/CMultiClassRegionData.h>
#include <geometry/base/CTrianglesContainer.h>
#include <alg/CMarchingCubesFast.h>


namespace data
{
class CRegion3DPreviewManager;

//! Scoped lock.
typedef vpl::base::CLockableObject<CRegion3DPreviewManager>::CLock tLock;

//! Class manages region 3D preview.
//! Starts a background thread, which waits for any change in segmentation volume
//! and runs fast marching cubes to create region preview.
class CRegion3DPreviewManager : public vpl::base::CLockableObject<CRegion3DPreviewManager>
{
public:
    vpl::base::CFunctor<void> *onStart;
    vpl::base::CFunctor<void> *onUpdate;
    vpl::base::CFunctor<void, geometry::CTrianglesContainer&> *onStop;

private:
    //! Container for storing result of fast marching cubes.
    geometry::CTrianglesContainer *m_container;

    //! Region index, for wich the preview will be created.
    data::CMultiClassRegionData::tVoxel m_bitIndex;

    //! Region volume voxel size.
    vpl::img::CSize3d m_voxelSize;

    //! Mutex for managing the access to data. 
    vpl::sys::CMutex m_mutex;

    //! Condition waiting for some change.
    vpl::sys::CCondition m_condition;

    //! Background thread.
    vpl::sys::CThread m_thread;

    //! Region volume changed?
    bool m_dataChanged;

    //! During some operation, we don+t want to update the preview (because of the performance), e.g. brush
    bool m_canUpdate;

    //! Interval in which the background thread will check changes in segmentation.
    int m_redrawInterval;

public:
    CRegion3DPreviewManager();

    ~CRegion3DPreviewManager();

    //! Initialize data.
    void init(const vpl::img::CSize3d &voxelSize, const data::CMultiClassRegionData::tVoxel &bitIndex);

    //! Suspends the background thread.
    void stop();

    //! Starts the background thread.
    void run();

    //! Sets region index, for wich the preview will be created.
    void setRegionIndex(const data::CMultiClassRegionData::tVoxel &bitIndex);

    //! Sets interval in which the background thread will check changes in segmentation.
    void setRedrawInterval(int value);

    //! Sets flag telling that region data has been changed and preview should be generated.
    void regionDataChanged();

    //! Sets flag telling that we want (or not) updating the preview.
    void setCanUpdate(bool canUpdate);

    //! Functor for fast marching cubes.
    template <typename V, typename T>
    class CBitLayerSelectFunctorPreview : public IMarchingCubesFastFunctor
    {
    public:
        //! Default constructor.
        CBitLayerSelectFunctorPreview(T layer, V * volume, vpl::img::CSize3d voxelSize)
            : m_layerBitIndex(layer)
            , m_volume(volume)
            , m_voxelSize(voxelSize)
        { }

        //! Limits value of a given parameter.
        unsigned char operator()(int x, int y, int z) const override
        {
            if (!m_volume->checkPosition(x, y, z))
            {
                return 0;
            }
            // test volume value to threshold
            if (m_volume->at(x, y, z, m_layerBitIndex))
            {
                return 1;
            }
            else
            {
                return 0;
            }
        }

        virtual vpl::img::CSize3i getVolumeDimensions() const override
        {
            return m_volume->getSize();
        }

        virtual vpl::img::CSize3d getVoxelSize() const override
        {
            return m_voxelSize;
        }

    protected:
        //! Bit index.
        T m_layerBitIndex;

        //! Volume used.
        V * m_volume;

        //! Voxel size.
        vpl::img::CSize3d m_voxelSize;
    };

protected:
    void startProgress();
    void updateProgress();
    void stopProgress();

public:
    static VPL_THREAD_ROUTINE(backgroundWorker);
};

}

#endif

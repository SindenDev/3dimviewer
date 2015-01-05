////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file   segmentation\CISTresholding.h
///
/// \brief  Declares the cis tresholding class. 
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CISTresholding_H_included
#define CISTresholding_H_included

namespace seg
{

template
<
    class tpVolume,
    class tpRVolume
>
class CThresholding : public vpl::mod::CProgress
{
protected:
    //! Volume type
    typedef tpVolume tVolume;

    //! Voxel type
    typedef typename tpVolume::tVoxel tVoxel;

    //! Region volume
    typedef tpRVolume tRVolume;

    //! Region voxel type
    typedef typename tpRVolume::tVoxel tRVoxel;

public:
    //! Constructor
    CThresholding( tVolume * src = NULL, tRVolume *dst = NULL )
        : m_src( src ), m_dst( dst ), m_clear( true )
    {}

    //! Set source volume
    void setSource( tVolume * volume ){ m_src = volume; }

    //! Set destination volume
    void setDestination( tRVolume * volume ) { m_dst = volume; }

    //! Set clear flag
    void setClear(bool value) { m_clear = value; }

    //! Set coloring value
    void setColor( tRVoxel color ) { m_color = color; }

    //! Set clearing value
    void setClearColor( tRVoxel color ) { m_clearColor = color; }

    //! Set thresholds
    void setWindow( tVoxel min, tVoxel max ) { m_min = min; m_max = max; }

    //! Apply operation
    bool apply(int quality = 0)
    {
        if( !this->testData() )
            return false;

        CProgress::tProgressInitializer StartProgress( *this );

        // Get volume sizes
        const vpl::tSize sx( this->m_src->getXSize() ), sy( this->m_src->getYSize() ), sz( this->m_src->getZSize() );

        CProgress::setProgressMax( sz );
        
        if (0 == quality)
        {
            // Do thresholding
            for( vpl::tSize z = 0; z < sz; ++z )
            {
                for( vpl::tSize y = 0; y < sy; ++y )
                {
                    vpl::tSize idxData = this->m_src->getIdx( 0, y, z );
                    vpl::tSize idxRegion = this->m_dst->getIdx( 0, y, z );

                    for( vpl::tSize x = 0; x < sx; ++x, idxData += this->m_src->getXOffset(), idxRegion += this->m_dst->getXOffset() )
                    {
    //                    tRVoxel color = this->m_clearColor;
                        tVoxel  data = this->m_src->at(idxData);
                        if( data >= this->m_min && data <= this->m_max )
                            this->m_dst->set(idxRegion,this->m_color);
                        else if (m_clear)
                            this->m_dst->set(idxRegion,this->m_clearColor);
                    }
                }

                // Notify progress observers...
                if( !CProgress::progress() )
                {
                    return false;
                }
            }
        }
        else
        {
            // setup params
            const int m_samples = quality;
            const double limit = quality>1 ? 0.033 : 0;

            const double step = m_samples == 0 ? 0.0 : 0.5 / m_samples;
            const int start = m_samples > 0 ? 1 - m_samples : 0;
            int sampleCount = m_samples * 2 + (m_samples > 0 ? 0 : 1);
            sampleCount = sampleCount * sampleCount * sampleCount;
            const int threshold = sampleCount * limit;

            bool bBroken = false;
            // Do thresholding
#pragma omp parallel for shared(bBroken)
            for( vpl::tSize z = 0; z < sz; ++z )
            {
                if (!bBroken)
                {
                    for( vpl::tSize y = 0; y < sy; ++y )
                    {
                        vpl::tSize idxRegion = this->m_dst->getIdx( 0, y, z );

                        for( vpl::tSize x = 0; x < sx; ++x, idxRegion += this->m_dst->getXOffset() )
                        {
                            // analyze voxel and its neighbourhood                        
                            int sum = 0;
                            for (int zOffset = start; zOffset <= m_samples && sum <= threshold; ++zOffset)
                            {
                                double fz = z + zOffset * step;
                                if (fz < 0.0 || fz > sz - 1)
                                    continue;
                                for (int yOffset = start; yOffset <= m_samples && sum <= threshold; ++yOffset)
                                {
                                    double fy = y + yOffset * step;
                                    if (fy < 0.0 || fy > sy - 1)
                                        continue;
                                    for (int xOffset = start; xOffset <= m_samples && sum <= threshold; ++xOffset)
                                    {
                                        double fx = x + xOffset * step;
                                        if (fx < 0.0 || fx > sx - 1)
                                            continue;
                                        // take voxel value for given coordinates
                                        tVoxel voxel_value = this->m_src->interpolate(vpl::img::CPoint3d(fx,fy,fz));
                                        // test volume value to threshold
                                        if ((voxel_value >= this->m_min) && (voxel_value <= this->m_max))
                                            sum++;
                                    }
                                }
                            }
                            if( sum > threshold )
                                this->m_dst->set(idxRegion,this->m_color);
                            else if (m_clear)
                                this->m_dst->set(idxRegion,this->m_clearColor);
                        }
                    }

                    // Notify progress observers...
    #pragma omp critical
                    if( !CProgress::progress() )
                        bBroken = true;
                }
            }
            return !bBroken;
        }
        return true;
    }
protected:
    //! Test if data pointers are correctly set
    bool testData() { return this->m_src != NULL && this->m_dst != NULL
                            && this->m_src->getXSize() <= this->m_dst->getXSize()
                            && this->m_src->getYSize() <= this->m_dst->getYSize()
                            && this->m_src->getZSize() <= this->m_dst->getZSize(); }

protected:
    //! Volume data
    tVolume * m_src;

    //! Region data
    tRVolume *m_dst;

    //! Fill color
    tRVoxel m_color;

    //! Flag if clear should be used on data outside thresholds
    bool m_clear;

    //! Clear color
    tRVoxel m_clearColor;

    //! Thresholds
    tVoxel m_min, m_max;

}; // template class CISThresholding

} // namespace seg

// CISTresholding_H_included
#endif


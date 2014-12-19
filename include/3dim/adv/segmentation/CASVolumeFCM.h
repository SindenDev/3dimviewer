///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// Copyright (c) 2009 by 3Dim Laboratory s.r.o.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __CASVOLUME_FCM__
#define __CASVOLUME_FCM__

#include <VPL/Math/Random.h>
#include <VPL/Math/Base.h>
#include <VPL/Math/LogNum.h>
#include <VPL/Math/Matrix.h>
#include <VPL/Math/Vector.h>
#include <VPL/Image/Image.h>
#include <VPL/Image/ImageHistogram.h>
#include <VPL/Image/VolumeHistogram.h>

#include <data/CDensityData.h>
#include <data/CRegionData.h>
#include <data/COrthoSlice.h>

namespace seg
{

using namespace vpl;

class CASVolumeFCM : public vpl::mod::CProgress
{
public:
    //! Image type.
    typedef vpl::img::CDImage	tImage;
	typedef vpl::img::CDVolume	tVolume;

    //! Image pixel type.
	typedef vpl::img::CDVolume::tVoxel	tPixel;
    //typedef vpl::img::CDImage::tPixel	tPixel;

    //! Constant used to enable automatic estimation of the number
    //! of clusters by FCM algorithm.
    enum { UNKNOWN = 0 };

    //! Default weighting factor (>1).
    static const double DEFAULT_WEIGHT;

    //! Minimal required change of the objective function.
    static const double MIN_CHANGE;

    //! Maximal allowed number of iterations.
    static const tSize MAX_ITERS;

    //! On what number to start labeling
    static const int REGION_OFFSET;

public:
    //! Constructor.
    CASVolumeFCM(tSize NumOfClusters = UNKNOWN,
                      double dWeight = DEFAULT_WEIGHT
                      );

    //! Virtual destructor.
    virtual ~CASVolumeFCM() {}

    //! Segmentation of a given image.
    //! - Uses vpl::img::CDImage image type.
    //! - Return false on failure.
    //bool execute( data::CDensityData & density, data::CRegionData & region );
    bool execute( vpl::img::CDensityVolume & density, data::CRegionData & region );


	//! Computes thresholds for FCM segmentation
	//void computeThresholds( data::CDensityData & density, int clusters, std::vector< tPixel > & thresholds );
	void computeThresholds( vpl::img::CDensityVolume & density, int clusters, std::vector< tPixel > & thresholds );

   //! Get real number of clusters
   tSize getNumOfClusters() { return m_NumOfClusters; }

protected:
    //! Vector of cluster centers.
    typedef vpl::math::CVector<double> tCenters;

    //! Membership matrix.
    typedef vpl::math::CMatrix<double> tMembership;

protected:
    //! The number of clusters.
    tSize m_NumOfClusters;

    //! Weighting factor.
    double m_dWeight;

    //! Allowed pixel values.
    tPixel m_PixelMin, m_PixelMax;

    //! Histogram size.
    tSize m_Span;

    //! Helper variables.
    double m_dExponent, m_dInvSpan;

    //! Cluster centers.
    tCenters m_Centers;

    //! Membership matrix.
    tMembership m_Membership;

    //! Helper matrix.
    tMembership m_Powers;

    //! Uniform random number generator.
    vpl::math::CUniformPRNG m_Uniform;

    //! Image histogram.
	vpl::img::CDVolumeHistogram		m_Histogram;

protected:
    //! Randomly initializes the membership matrix.
    void initMembership();

    //! Checks if the membership matrix is O.K.
    bool checkMembership();

    //! Returns membership of a given pixel in i-th cluster.
    double getMembership(double dValue, tSize i);

    //! Re-computes membership for every pixel.
    void recomputeMembership();

    //! Recomputes matrix of powers of the membership function.
    void recomputePowers();

    //! Compute new positions of cluster centers.
    void recomputeClusterCenters();

    //! Returns value of the objective function.
    double recomputeObjectiveFunction();

    //! Computes the Dunn's partitions coefficient.
    double computeDunnCoefficient();

    //! Computes FCM of a given image.
    void iterateFCM(double dMinChange);


//public :

    //! Segments a given input image.
    //void segmentVolume(data::CDensityData & density, data::CRegionData & region );
    void segmentVolume( vpl::img::CDensityVolume & density, data::CRegionData & region );
};


}

#endif


///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// Copyright (c) 2009 by 3Dim Laboratory s.r.o.
//
///////////////////////////////////////////////////////////////////////////////

#include <segmentation/CASVolumeFCM.h>

#include <VPL/Base/Logging.h>
#include <VPL/Math/MatrixFunctions.h>
#include <VPL/Image/ImageFunctions.h>

//#include <base/Macros.h>

#include <float.h>
#include <cmath>



namespace seg
{

//const double CImageFuzzyCMeans::DEFAULT_WEIGHT      = 2;
const double	CASVolumeFCM::DEFAULT_WEIGHT      = 1.5;
const double	CASVolumeFCM::MIN_CHANGE          = 1.0e-6;
const tSize		CASVolumeFCM::MAX_ITERS            = 100;
const int      CASVolumeFCM::REGION_OFFSET        = 1;


CASVolumeFCM::CASVolumeFCM(tSize NumOfClusters, double dWeight)
    : m_NumOfClusters(NumOfClusters)
    , m_dWeight(dWeight)
    , m_PixelMin(vpl::img::CPixelTraits<tPixel>::getPixelMin()+1) // ignore the lowest value because volume could be expanded to a multiple of 4 by this value
    , m_PixelMax(vpl::img::CPixelTraits<tPixel>::getPixelMax())
    , m_Span(tSize(m_PixelMax - m_PixelMin + 1))
    , m_Histogram(m_PixelMin, m_PixelMax)
{
    VPL_ASSERT(m_NumOfClusters >= 0 && m_dWeight >= 1.0);

    if( m_NumOfClusters > 0 )
    {
       m_Centers.resize(NumOfClusters);
       m_Membership.resize( NumOfClusters, m_Span );
       m_Powers.resize(NumOfClusters, m_Span);

    }

    m_dExponent = 2.0 / (m_dWeight - 1.0);
    m_dInvSpan = 1.0 / m_Span;
}



//bool CASVolumeFCM::execute(data::CDensityData & density, data::CRegionData & region)
bool CASVolumeFCM::execute( vpl::img::CDensityVolume & density, data::CRegionData & region)
{
	CProgress::tProgressInitializer StartProgress( *this );

	CProgress::setProgressMax( MAX_ITERS );

    // Compute histogram of input image
    m_Histogram(density);

    // Is the number of clusters known?
    if( m_NumOfClusters > 0 )
    {
        // Resize the membership matrix
        m_Membership.resize(m_NumOfClusters, m_Span);
        m_Powers.resize(m_NumOfClusters, m_Span);

        // Resize the vector of cluster centers
        m_Centers.resize(m_NumOfClusters);

        // Compute the FCM
        iterateFCM(MIN_CHANGE);

        // Final segmentation
        segmentVolume(density, region);

        // Add offset
        region += vpl::CScalari(REGION_OFFSET);

        // O.K.
        return true;
    }

    // Initial Dunn's coefficient
    double dDunnCoeff = 0.0;

    // Test various numbers of clusters
    m_NumOfClusters = 1;
    for( ;; )
    {

        // Resize the membership matrix
        m_Membership.resize(m_NumOfClusters, m_Span);
        m_Powers.resize(m_NumOfClusters, m_Span);

        // Resize the vector of cluster centers
        m_Centers.resize(m_NumOfClusters);

        // Compute the FCM
        iterateFCM(MIN_CHANGE);

        // Compute the Dunn's coefficient
        double dNewValue = computeDunnCoefficient();

        // Estimate changes
        if( dNewValue < dDunnCoeff || dNewValue > 1.0 )
        {
            break;
        }

        // Image segmentation
        segmentVolume(density, region);

        // Update current Dunn's coefficient
        dDunnCoeff = dNewValue;

        // Increment the number of clusters
        ++m_NumOfClusters;
    }

    // Add offset
    region += vpl::CScalari(REGION_OFFSET);

    // O.K.
    return true;
}


void CASVolumeFCM::iterateFCM(double dMinChange)
{
    // Random membership function
    initMembership();

    // Initial value of the objective function
    double dObjectiveFunc = 1.0;

    // Iterate while the function converges
    for( int i = 0; i < MAX_ITERS; i++ )
    {
		// Notify progress observers...
        if( !CProgress::progress() )
        {
            return;
        }

        // Recompute matrix of membership powers
        recomputePowers();

        // Update cluster centers
        recomputeClusterCenters();

#ifdef FCM_LOGGING_ENABLED
        VPL_LOG_INFO("iterateFCM()");
        for( tSize k = 0; k < m_NumOfClusters; ++k )
        {
            VPL_LOG_INFO("  Cluster " << k << ": " << m_Centers(k));
        }
#endif // FCM_LOGGING_ENABLED

        // Update the membership matrix
        recomputeMembership();

        // Evaluate the objective function
        double dNewValue = recomputeObjectiveFunction();

        // Estimate change of the objective function
        double dDelta = vpl::math::getAbs(dNewValue / dObjectiveFunc - 1.0);

#ifdef FCM_LOGGING_ENABLED
        VPL_LOG_INFO("iterateFCM()");
        VPL_LOG_INFO("  Objective Function = " << dNewValue);
        VPL_LOG_INFO("  Delta = " << dDelta);
#endif // FCM_LOGGING_ENABLED

        // Estimate changes
        if( dDelta < dMinChange || dNewValue < dMinChange )
        {
            break;
        }

        // Update the current value
        dObjectiveFunc = dNewValue;
    }
}


void CASVolumeFCM::initMembership()
{
    VPL_ASSERT(m_NumOfClusters == m_Membership.getNumOfRows());

    m_Membership.zeros();

    for( tSize j = 0; j < m_Span; ++j )
    {
        tSize Index = vpl::math::round2Int(m_Uniform.random(0, m_NumOfClusters - 1));
        m_Membership(Index, j) = 1;
    }
}


bool CASVolumeFCM::checkMembership()
{
    double dTemp = vpl::math::getSum<double>(m_Membership) * m_dInvSpan;

    return (vpl::math::getAbs(dTemp - 1.0) < 0.001);
}


double CASVolumeFCM::getMembership(double dValue, tSize i)
{
    double dCenter = m_Centers(i);
    double dNumerator = vpl::math::getAbs(dCenter - dValue);

    double dMembership = 0.0;
    for( tSize j = 0; j < m_NumOfClusters; ++j )
    {
        double dTemp = dNumerator / vpl::math::getAbs(m_Centers(j) - dValue);
        dMembership += pow(dTemp, m_dExponent);
    }

    return (dMembership > 0.0) ? 1.0 / dMembership : 1.0;
}


void CASVolumeFCM::recomputeMembership()
{
    for( tSize i = 0; i < m_NumOfClusters; ++i )
    {
        for( tSize j = 0; j < m_Span; ++j )
        {
            m_Membership(i, j) = getMembership(m_Histogram.getLowerBound(j), i);
        }
    }
}


void CASVolumeFCM::recomputePowers()
{
    for( tSize i = 0; i < m_NumOfClusters; ++i )
    {
        for( tSize j = 0; j < m_Span; ++j )
        {
            m_Powers(i, j) = pow(m_Membership(i, j), m_dWeight);
        }
    }
}


void CASVolumeFCM::recomputeClusterCenters()
{
    for( tSize i = 0; i < m_NumOfClusters; ++i )
    {
        m_Centers(i) = 0.0;
        double dDenom = 0.0;

        for( tSize j = 0; j < m_Span; ++j )
        {
            double dTemp = m_Histogram.getCount(j) * m_Powers(i, j);
            m_Centers(i) += dTemp * m_Histogram.getLowerBound(j);
            dDenom += dTemp;
        }

        if( dDenom > 0.0 )
        {
            m_Centers(i) /= dDenom;
        }
    }
}


double CASVolumeFCM::recomputeObjectiveFunction()
{
    double dResult = 0.0;

    for( tSize i = 0; i < m_NumOfClusters; ++i )
    {
        for( tSize j = 0; j < m_Span; ++j )
        {
            double dTemp = m_Centers(i) - m_Histogram.getLowerBound(j);
            dResult += m_Histogram.getCount(j) * m_Powers(i, j) * dTemp * dTemp;
        }
    }

    return dResult;
}


double CASVolumeFCM::computeDunnCoefficient()
{
    double dInvNumOfClusters = 1.0 / m_Membership.getNumOfRows();
    double dInvNumOfSamples = 1.0 / m_Membership.getNumOfCols();

    double dCoefficient = 0.0;
    for( tSize i = 0; i < m_Membership.getNumOfRows(); ++i )
    {
        for( tSize j = 0; j < m_Membership.getNumOfCols(); ++j )
        {
            dCoefficient += m_Membership(i, j) * m_Membership(i, j) * dInvNumOfSamples;
        }
    }

    return (dCoefficient - dInvNumOfClusters) / (1.0 - dInvNumOfClusters + 0.001);
}

//void CASVolumeFCM::computeThresholds( data::CDensityData & density, int clusters, std::vector< tPixel > & thresholds )
void CASVolumeFCM::computeThresholds( vpl::img::CDensityVolume & density, int clusters, std::vector< tPixel > & thresholds )
{
	// Get the effective size
//    tSize XCount = vpl::math::getMin(density.getXSize(), density.getXSize());
//    tSize YCount = vpl::math::getMin(density.getYSize(), density.getYSize());
//    tSize ZCount = vpl::math::getMin(density.getZSize(), density.getZSize());

	// Erase threshold vector
	thresholds.erase( thresholds.begin(), thresholds.end() );

    // Compute histogram of input image
    m_Histogram(density);

    // Is the number of clusters known?
    if( clusters > 0 )
    {
        // Resize the membership matrix
        m_Membership.resize( clusters, m_Span );
        m_Powers.resize( clusters, m_Span );

        // Resize the vector of cluster centers
        m_Centers.resize( clusters );

        // Compute the FCM
        iterateFCM(MIN_CHANGE);


		tPixel	min = m_Histogram.getLowerBound(0);
		tPixel	max = min + m_Histogram.getSize();

		tPixel  last = min;
		tSize   lastIndex = m_Histogram.getIndex( last );

		

		tSize Max = 0;
		for( tSize i = 1; i < clusters; ++i )
		{
			if( m_Membership( i, lastIndex ) > m_Membership( Max, lastIndex ) )
			{
				Max = i;
			}
		}

		tSize   lastCluster = Max;

		for ( tPixel current = min + 1;  current < max; current++ )
		{
			tSize	currentIndex = m_Histogram.getIndex( current );

			tSize Max = 0;
			for( tSize i = 1; i < clusters; ++i )
			{
				if( m_Membership( i, currentIndex ) > m_Membership( Max, currentIndex ) )
				{
					Max = i;
				}
			}

			tSize currentCluster = Max;

			if ( currentCluster != lastCluster ) thresholds.push_back( current );

			lastCluster = currentCluster;
		}
    }
}

//void CASVolumeFCM::segmentVolume(data::CDensityData & density, data::CRegionData & region)
void CASVolumeFCM::segmentVolume( vpl::img::CDensityVolume & density, data::CRegionData & region)
{
	vpl::tSize	XSize = density.getXSize();
	vpl::tSize	YSize = density.getYSize();
	vpl::tSize  ZSize = density.getZSize();

    // Classify input image pixels
	for ( vpl::tSize z = 0; z < ZSize; ++z )
	{
		//_TRACEW( "slice " << z << " of " << ZSize << std::endl );
		for( vpl::tSize y = 0; y < YSize; ++y )
		{
			for( vpl::tSize x = 0; x < XSize; ++x )
			{
				// Get index of the histogram bin
				tSize Index = m_Histogram.getIndex( density( x, y, z ) );

				// Find cluster whose membership function is maximal
				tSize Max = 0;
				for( tSize i = 1; i < m_NumOfClusters; ++i )
				{
					if( m_Membership(i, Index) > m_Membership(Max, Index) )
					{
						Max = i;
					}
				}
				region.set( x, y, z, Max );
			}
		}
	}
}


}


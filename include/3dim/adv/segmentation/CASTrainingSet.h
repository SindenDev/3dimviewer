///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// Copyright (c) 2009 by 3Dim Laboratory s.r.o.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __CASTRAININGSAMPLER__
#define __CASTRAININGSAMPLER__

// drawing tool access
#include <osg/SceneDraw.h>

// data access
#include <data/CStorageInterface.h>
#include <data/COrthoSlice.h>

// MDSTk
#include <VPL/Image/PixelTypes.h>
#include <VPL/Math/StaticVector.h>
#include <VPL/Base/STLIterator.h>

// Eigen
#include <Eigen/StdVector>

#include <base/Macros.h>

namespace seg
{
    ///////////////////////////////////////////////////////////////////////////////
    //! CTraining sample
    //! - One sample contained within the training set
	template < typename _Value, unsigned _Dim >
	class CTrainingSample : public vpl::math::CStaticVector< _Value, _Dim >
	{
		public :

            //! Naming the baseclass
			typedef vpl::math::CStaticVector< _Value, _Dim >	base;

            //! 
			typedef _Value	tValue;

			enum { DIM = _Dim };

		public :

            //! Empty constructor
			CTrainingSample() 
			{
			}

            //! Cop constructor for the training sample
			CTrainingSample( const CTrainingSample & sample ) : base( sample )
			{			
			}

            //! Assignment between two training sets 
			CTrainingSample & operator= ( const CTrainingSample & sample )
			{
				if ( &sample != this )
				{
					base::operator=( sample );
				}
				return *this;
			}

	};

    ///////////////////////////////////////////////////////////////////////////////
    //! CTrainingSet
    //! - Wrapper aroung a vector 
	template < class _Sample >
	class CTrainingSet : public vpl::base::CObject
	{		
		public :

			//! Smart pointer declaration 
			VPL_SHAREDPTR( CTrainingSet );

			//! Internal storage for training samples
			typedef std::vector< _Sample, Eigen::aligned_allocator<_Sample> > tSampleVector;

		protected :

			//! Internal storage object
			tSampleVector		mSamples;

		public :

			//! Constructor sets container to reference internal storage vector
			CTrainingSet()
			{
			}

			//! Virtual destructor
			virtual ~CTrainingSet()
			{
				mSamples.clear();
			}

			//! Adds a training sample to the set
			void	add( const _Sample & sample )
			{
				mSamples.push_back( sample );
			}

			//! Returns i-th training sample
			_Sample &	at( int i )
			{
				return mSamples[i];
			}

			//! Removes all samples from the set
			void	clear()
			{
				mSamples.clear();
			}

			//! Returns the size of the training set
			int		size()
			{
				return mSamples.size();
			}

			//! Returns reference to a container
			tSampleVector &	container()
			{
				return mSamples;
			}
	};

	template < class _Sample >
	class CSamplerBase : public vpl::base::CObject
	{
		public :

			//!
			typedef CTrainingSet< _Sample >		tTrainingSet;

			//!
			VPL_SHAREDPTR( CSamplerBase );

		protected :

			//!
			tTrainingSet						mTrainingSet;

		public :

			//! Constructor parametrized by training set
			CSamplerBase()
			{
			}

			//! Adds sample to the internal training set
			void addSample( const _Sample & sample )
			{
				mTrainingSet.add( sample );
			}

			//!
			tTrainingSet &	set()
			{
				return mTrainingSet;
			}

			void clear()
			{
				mTrainingSet.clear();
			}

	};

	template < class _Sample, class _Coord, class _Data >
	class CSampler : public CSamplerBase< _Sample >
	{
		public :

			typedef _Coord						tCoord;

			typedef _Sample						tSample;

			typedef _Data						tData;

			//!
			VPL_SHAREDPTR( CSampler );

		public :
		
			CSampler()
			{
			}

			virtual void	getSample( const _Coord & coord, _Data & data )
			{				
			}

			virtual void	loadSample( tSample & sample, const tCoord & coord, tData & data )
			{
			}

	};
}

#endif

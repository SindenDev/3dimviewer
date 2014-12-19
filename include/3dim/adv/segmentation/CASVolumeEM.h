///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// Copyright (c) 2009 by 3Dim Laboratory s.r.o.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __CASVOLUMESEGMENTATIONEM__
#define __CASVOLUMESEGMENTATIONEM__

#include <segmentation/CEM.h>
#include <segmentation/CASVolume.h>


namespace seg
{

    
	template < class _Sampler >
	class CVolumeEM : public seg::CATrainableSegmentation< typename _Sampler::tSample >
	{
	    public :
			VPL_SHAREDPTR( CVolumeEM );

		public :
			//! Sampler type
			typedef _Sampler						tSampler;

			//! Sample type
			typedef typename _Sampler::tSample		tSample;

			//! Training set type
			typedef CTrainingSet< tSample >			tSet;

			//! Vector
			typedef std::vector< tSample >			tSetVectorType;



			//! Segmentation object
			//typedef vpl::math::CMaxLikelihoodByEM< tSetVector, tSample::DIM >	tClustering;
            typedef vpl::math::CMaxLikelihoodByEM< vpl::math::CVector< double >, tSample::DIM > tClustering;

		protected :

			//!
			tClustering		mPositiveModel;

			//!
			bool			bPositiveTrained;

			//!
			tClustering		mNegativeModel;

			//!
			bool			bNegativeTrained;

			//!
			int				iGaussians;

			//!
			double			dPositiveMax;

			//!
			double			dNegativeMax;

			std::vector< int >	mTable;

		public :

			//!
			CVolumeEM( )// : CATrainableSegmentation()
			{
				iGaussians = 4;
				bPositiveTrained = false;
				bNegativeTrained = false;
			}

			//!
			void	setGaussians( int g )
			{
				iGaussians = g;
			}

			//!
			bool	hasPositiveModel()	
			{
				return bPositiveTrained;
			}

			//!
			bool	hasNegativeModel()
			{
				return bNegativeTrained;
			}

			//!
			virtual bool	train( CTrainingSet< tSample > & positive_set, CTrainingSet< tSample > & negative_set  )
			{
				int size;				

				size =		positive_set.size();
				if ( size > 0 )
				{
					//tSetVectorType	set_vector( size );
                    vpl::math::CVector< double >  set_vector( size );
                    
					for ( int i = 0; i < size; i++ ) set_vector(i) = positive_set.at(i).at( 0 );

					//tSetVector		container( set_vector );
                    
                    mPositiveModel.execute( set_vector, iGaussians );	
					//mPositiveModel.execute( container, iGaussians );	
					
					vpl::img::tDensityPixel		pix = vpl::img::CPixelTraits< vpl::img::tDensityPixel >::getMin();
					double max_prob	=	-1.0;
					double prob		=	0.0;
					tSample	v;
					while ( pix < vpl::img::CPixelTraits< vpl::img::tDensityPixel >::getMax() )
					{
						v(0) = pix;
						for ( int l = 0; l < iGaussians; l++ )
						{
							prob = prob + mPositiveModel.getComponent(l).getWeightedValue( v );
						}
						if ( prob > max_prob ) max_prob = prob;

						pix++;
					}
					dPositiveMax = max_prob;

					bPositiveTrained = true;
				}

				size =		negative_set.size();
				if ( size > 0 )
				{
					//tSetVectorType		set_vector( size );
					//
					//for ( int i = 0; i < size; i++ ) set_vector[ i ] = negative_set.get( i );

					//tSetVector		container( set_vector );
					//mNegativeModel.execute( container, iGaussians );

					//tSetVectorType	set_vector( size );
                    vpl::math::CVector< double >  set_vector( size );
                    
					for ( int i = 0; i < size; i++ ) set_vector(i) = negative_set.at(i).at(0);

					//tSetVector		container( set_vector );
                    
                    mNegativeModel.execute( set_vector, iGaussians );	
					//mPositiveModel.execute( container, iGaussians );	

					vpl::img::tDensityPixel		pix = vpl::img::CPixelTraits< vpl::img::tDensityPixel >::getMin();
					double max_prob	=	-1.0;
					double prob		=	0.0;
					tSample	v;
					while ( pix < vpl::img::CPixelTraits< vpl::img::tDensityPixel >::getMax() )
					{
						v(0) = pix;
						for ( int l = 0; l < iGaussians; l++ )
						{
							prob = prob + mNegativeModel.getComponent(l).getWeightedValue( v );
						}
						if ( prob > max_prob ) max_prob = prob;

						pix++;
					}
					dNegativeMax = max_prob;

					bNegativeTrained = true;
					
				}

				return true;
			}

			virtual bool	prepareTable()
			{
				mTable.clear();
				

				vpl::img::tDensityPixel max = vpl::img::CPixelTraits< vpl::img::tDensityPixel >::getMax();
				vpl::img::tDensityPixel min = vpl::img::CPixelTraits< vpl::img::tDensityPixel >::getMin();
				mTable.resize( max - min );

				vpl::img::tDensityPixel		pix = vpl::img::CPixelTraits< vpl::img::tDensityPixel >::getMin();

				tSample	v;
				while ( pix < vpl::img::CPixelTraits< vpl::img::tDensityPixel >::getMax() )
				{
					double pp = 0.0;
					double pn = 0.0;

					v(0) = pix;

					if ( hasPositiveModel() && hasNegativeModel() )
					{
						for ( int l = 0; l < iGaussians; l++ )
						{
							pp = pp + mPositiveModel.getComponent(l).getWeightedValue( v );
							pn = pn + mNegativeModel.getComponent(l).getWeightedValue( v );
						}
						double pprob = pp / dPositiveMax;
						double nprob = pn / dNegativeMax;

						if ( pprob < nprob ) mTable[ pix - min ] = 0;
						else mTable[ pix - min ] = 1;
					}
					else if ( hasPositiveModel() )
					{
						for ( int l = 0; l < iGaussians; l++ )
						{
							pp = pp + mPositiveModel.getComponent(l).getWeightedValue( v );
						}
						double pprob = pp / dPositiveMax;

						if ( pprob < 0.05 ) mTable[ pix - min ] = 1;
						else mTable[ pix - min ] = 0;
					}
					else if ( hasNegativeModel() )
					{
						for ( int l = 0; l < iGaussians; l++ )
						{
							pn = pn + mNegativeModel.getComponent(l).getWeightedValue( v );
						}

						double nprob = pn / dNegativeMax;

						if ( nprob < 0.05 ) mTable[ pix - min ] = 0;
						else mTable[ pix - min ] = 1;
					}
					else
					{
					}
					pix++;
				}
				return true;
			}

			virtual void	executeSliceOnly( data::CDensityData & density, data::CRegionData & region, data::COrthoSlice & slice, int currentRegion )
			{
				vpl::img::tDensityPixel min = vpl::img::CPixelTraits< vpl::img::tDensityPixel >::getMin();

				switch ( slice.getPlane() )
				{				
					case data::COrthoSlice::PLANE_XY :

                  // Comptue coloring
						prepareTable();

						for ( vpl::tSize j = 0; j < slice.getWidth(); j++ )
						{
							for ( vpl::tSize i = 0; i < slice.getHeight(); i++ )
							{
                        // Set region to the current active
                        int v( (mTable[ density( j, i, slice.getPosition() ) - min ] != 0 ) ? currentRegion : 0 );
								region.set( j, i, slice.getPosition(), v );
							}
						}
						break;

					case data::COrthoSlice::PLANE_XZ :
						break;

					case data::COrthoSlice::PLANE_YZ : 
						break;
				}
			}

			//!
			virtual	void	execute( data::CDensityData & density, data::CRegionData & region, int currentRegion, bool current_only = true )
			{	
				tSample		v;
				prepareTable();
            int value;

				vpl::img::tDensityPixel min = vpl::img::CPixelTraits< vpl::img::tDensityPixel >::getMin();
				for ( vpl::tSize k = 0; k <= region.getZSize(); k++ )
				{
					for ( vpl::tSize j = 0; j < region.getYSize(); j++ )
					{
						for ( vpl::tSize i = 0; i < region.getXSize(); i++ )
						{
                     value = (mTable[ density( i, j, k ) - min ] != 0 ) ? currentRegion : 0;
							region.set( i, j, k, value );				
						}
					}
				}		
				
			}

	};

	template < class _VoxelSampler >	
	class CVolumeEMHandler : public CVoxelSampler< _VoxelSampler >
	{
		public:
			typedef CVoxelSampler< _VoxelSampler > base;
			typedef typename base::tSampler tSampler;

		public:

			CVolumeEMHandler( OSGCanvas * canvas, scene::CSceneOSG * scene, tSampler * positive_sampler, tSampler * negative_sampler ) :
				CVoxelSampler< _VoxelSampler >( canvas, scene, positive_sampler, negative_sampler, true )
			{
			}

			virtual void doSegmentation()
			{
				typename seg::CATrainableSegmentation< typename tSampler::tSample >::tSmartPtr	segmentation( new CVolumeEM< tSampler >() );

				segmentation->train( this->pPositiveSampler->set(), this->pNegativeSampler->set() );
				
				data::CObjectPtr< data::CRegionData  > pRegion( APP_STORAGE.getEntry( data::Storage::RegionData::Id ) );
				data::CObjectPtr< data::CDensityData > pVolume( APP_STORAGE.getEntry(VPL_SIGNAL(SigGetActiveDataSet).invoke2()) );

				segmentation->execute( *pVolume, *pRegion );

				APP_STORAGE.invalidate( pRegion.getEntryPtr() );
			}

	};
}

#endif

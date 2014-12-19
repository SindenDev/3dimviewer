///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// Copyright (c) 2009 by 3Dim Laboratory s.r.o.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __CASVOLUMESAMPLER__
#define	__CASVOLUMESAMPLER__

#include <segmentation/CABase.h>
#include <segmentation/CLineRasterizer.h>

#include <data/CDensityData.h>
#include <data/CRegionData.h>
#include <data/COrthoSlice.h>


namespace seg
{
    ///////////////////////////////////////////////////////////////////////////////
    //! CVoxelValue
    //! - Samples only values of current voxel
	class CVoxelValue	: public CTrainingSample< double, 1 >
	{
		public :

            //! Empty constructor
			CVoxelValue( const double & value = 0 )
			{
				this->at( 0 ) = value;
			}

	};

    ///////////////////////////////////////////////////////////////////////////////
    //! CMean
    //! - Samples mean value from closest neigborhood
	class CMean	:	public	CTrainingSample< double, 1 >
	{
		public :

            //! Empty constructor
			CMean( const double & value = 0.0 )
			{
				this->at( 0 ) = value;
			}
	};

    ///////////////////////////////////////////////////////////////////////////////
    //! CSevenVoxels
    //! - Samples seven voxels from the neigborhood
	class CSevenVoxels	: public CTrainingSample< vpl::img::tDensityPixel, 7 >
	{
		public :

            //! Empty constructor
			CSevenVoxels()
			{
				this->at( 0 ) = 0;
				this->at( 1 ) = 0;
				this->at( 2 ) = 0;
				this->at( 3 ) = 0;
				this->at( 4 ) = 0;
				this->at( 5 ) = 0;
				this->at( 6 ) = 0;
			}

	};

    ///////////////////////////////////////////////////////////////////////////////
    //! CVoxelAverageVariance
    //! - Samples mean and variance from the neighborhood of current coordinate
	class CVoxelAverageVariance : public CTrainingSample< double, 2 >
	{
		public :

            //! Empty constructor
			CVoxelAverageVariance()
			{
				this->at( 0 ) = 0.0;
				this->at( 1 ) = 0.0;
			}
	};

    ///////////////////////////////////////////////////////////////////////////////
    //! CMeanSampler
    //! - Converts volume coordinate into CMean voxel value
	class CMeanSampler : public CSampler< CMean, osg::Vec3, data::CDensityData >
	{
		public :

			//! Smart pointer declaration
			VPL_SHAREDPTR( CMeanSampler );

		public :

            //! Load the sample
			virtual void loadSample( tSample & sample, const tCoord & coord, tData & data )
			{
				sample( 0 ) = 0.0;

				int x, y, z;
				x = static_cast< int >( coord[ 0 ] );
				y = static_cast< int >( coord[ 1 ] );
				z = static_cast< int >( coord[ 2 ] );

				sample( 0 ) = sample( 0 ) + data( x - 1, y - 1, z - 1 );
				sample( 0 ) = sample( 0 ) + data( x - 1, y - 1, z );
				sample( 0 ) = sample( 0 ) + data( x - 1, y - 1, z + 1 );
				sample( 0 ) = sample( 0 ) + data( x - 1, y, z - 1 );
				sample( 0 ) = sample( 0 ) + data( x - 1, y, z );
				sample( 0 ) = sample( 0 ) + data( x - 1, y, z + 1 );
				sample( 0 ) = sample( 0 ) + data( x - 1, y + 1, z - 1 );
				sample( 0 ) = sample( 0 ) + data( x - 1, y + 1, z );
				sample( 0 ) = sample( 0 ) + data( x - 1, y + 1, z + 1 );

				sample( 0 ) = sample( 0 ) + data( x, y - 1, z - 1 );
				sample( 0 ) = sample( 0 ) + data( x, y - 1, z );
				sample( 0 ) = sample( 0 ) + data( x, y - 1, z + 1 );
				sample( 0 ) = sample( 0 ) + data( x, y, z - 1 );
				sample( 0 ) = sample( 0 ) + data( x, y, z );
				sample( 0 ) = sample( 0 ) + data( x, y, z + 1 );
				sample( 0 ) = sample( 0 ) + data( x, y + 1, z - 1 );
				sample( 0 ) = sample( 0 ) + data( x, y + 1, z );
				sample( 0 ) = sample( 0 ) + data( x, y + 1, z + 1 );

				sample( 0 ) = sample( 0 ) + data( x + 1, y - 1, z - 1 );
				sample( 0 ) = sample( 0 ) + data( x + 1, y - 1, z );
				sample( 0 ) = sample( 0 ) + data( x + 1, y - 1, z + 1 );
				sample( 0 ) = sample( 0 ) + data( x + 1, y, z - 1 );
				sample( 0 ) = sample( 0 ) + data( x + 1, y, z );
				sample( 0 ) = sample( 0 ) + data( x + 1, y, z + 1 );
				sample( 0 ) = sample( 0 ) + data( x + 1, y + 1, z - 1 );
				sample( 0 ) = sample( 0 ) + data( x + 1, y + 1, z );
				sample( 0 ) = sample( 0 ) + data( x + 1, y + 1, z + 1 );

				sample( 0 ) = sample( 0 ) / 27;
			}

            //! Loads a sample and adds it into the training set
			virtual void	getSample( const osg::Vec3 & coord, data::CDensityData & data )
			{
				tSample	sample;

				loadSample( sample, coord, data );			
				addSample( sample );
			}

	};

    ///////////////////////////////////////////////////////////////////////////////
    //! CSevenVoxelsSampler
    //! - Converts volume coordinate into CSevenVoxels voxel value
	class CSevenVoxelsSampler : public CSampler< CSevenVoxels, osg::Vec3, data::CDensityData >
	{
		public :

		    //! Smart pointer declaration
			VPL_SHAREDPTR( CSevenVoxelsSampler );

		public :

            //! Load the sample ( instance of CSevenVoxels )
			virtual void	loadSample( tSample & sample, const tCoord & coord, tData & data )
			{
				double x, y, z;
				x = coord[ 0 ];
				y = coord[ 1 ];
				z = coord[ 2 ];

				sample( 0 ) = data( static_cast< vpl::tSize >( x ), 
											static_cast< vpl::tSize >( y ), 
											static_cast< vpl::tSize >( z ) );

				sample( 1 ) = data( static_cast< vpl::tSize >( x + 1 ), 
											static_cast< vpl::tSize >( y ), 
											static_cast< vpl::tSize >( z ) );

				sample( 2 ) = data( static_cast< vpl::tSize >( x - 1 ), 
											static_cast< vpl::tSize >( y ), 
											static_cast< vpl::tSize >( z ) );

				sample( 3 ) = data( static_cast< vpl::tSize >( x ), 
											static_cast< vpl::tSize >( y + 1 ), 
											static_cast< vpl::tSize >( z ) );

				sample( 4 ) = data( static_cast< vpl::tSize >( x ), 
											static_cast< vpl::tSize >( y - 1 ), 
											static_cast< vpl::tSize >( z ) );

				sample( 5 ) = data( static_cast< vpl::tSize >( x ), 
											static_cast< vpl::tSize >( y ), 
											static_cast< vpl::tSize >( z + 1 ) );

				sample( 6 ) = data( static_cast< vpl::tSize >( x ), 
											static_cast< vpl::tSize >( y ), 
											static_cast< vpl::tSize >( z - 1 ) );

			}

            //! Loads a sample and adds it into the training set
			virtual void	getSample( const osg::Vec3 & coord, data::CDensityData & data )
			{
				tSample	sample;

				loadSample( sample, coord, data );			
				addSample( sample );
			}

	};

    ///////////////////////////////////////////////////////////////////////////////
    //! CVoxelValueSampler
    //! - Converts volume coordinate into CVoxelValue voxel value
	class CVoxelValueSampler : public CSampler< CVoxelValue, osg::Vec3, data::CDensityData >
	{
		public :

			//! Smart pointer declaration
			VPL_SHAREDPTR( CVoxelValueSampler );

		public :

            //! Load the sample ( instance of CVoxelValue )
			virtual void	getSample( const osg::Vec3 & coord, data::CDensityData & data )
			{
				tSample	sample;
				loadSample( sample, coord, data );
				addSample( sample );
			}

            //! Loads a sample and adds it into the training set
			virtual void	loadSample( tSample & sample, const tCoord & coord, tData & data )
			{
				double x, y, z;
				x = coord[ 0 ];
				y = coord[ 1 ];
				z = coord[ 2 ];

				sample( 0 ) = data( static_cast< vpl::tSize >( x ), 
											static_cast< vpl::tSize >( y ), 
											static_cast< vpl::tSize >( z ) );
			}

	};


    ///////////////////////////////////////////////////////////////////////////////
    //! CVoxelValueSampler
    //! - Converts volume coordinate into seven instances of CVoxelValue 
	class CVoxelNeighbourhoodSampler : public CSampler< CVoxelValue, osg::Vec3, data::CDensityData >
	{
		public :

			//! Smart pointer declaration
			VPL_SHAREDPTR( CVoxelNeighbourhoodSampler );

		public :

            //! Load the sample ( instance of CVoxelValue )
			virtual void	loadSample( tSample & sample, const tCoord & coord, tData & data )
			{
				double x, y, z;
				x = coord[ 0 ];
				y = coord[ 1 ];
				z = coord[ 2 ];

				sample( 0 ) = data( static_cast< vpl::tSize >( x ), 
											static_cast< vpl::tSize >( y ), 
											static_cast< vpl::tSize >( z ) );
			}

            //! Loads 7 samples from neighborhood of current coordinate and adds them into the training set
			virtual void	getSample( const osg::Vec3 & coord, data::CDensityData & data )
			{
				tSample sample;

				loadSample( sample, coord, data );
				addSample( sample );
				loadSample( sample, coord + osg::Vec3( 1, 0, 0 ), data );
				addSample( sample );
				loadSample( sample, coord + osg::Vec3( -1, 0, 0 ), data );
				addSample( sample );
				loadSample( sample, coord + osg::Vec3( 0, 1, 0 ), data );
				addSample( sample );
				loadSample( sample, coord + osg::Vec3( 0, -1, 0 ), data );
				addSample( sample );
				loadSample( sample, coord + osg::Vec3( 0, 0, 1 ), data );
				addSample( sample );
				loadSample( sample, coord + osg::Vec3( 0, 0, -1 ), data );
				addSample( sample );
			}
	};

    ///////////////////////////////////////////////////////////////////////////////
    //! CVoxelValueTrainingSet
    //! - Training set for CVoxelValue sample    
	class CVoxelValueTrainingSet : public CTrainingSet< CVoxelValue >
	{
	};

    ///////////////////////////////////////////////////////////////////////////////
    //! CSevenVoxelTrainingSet
    //! - Training set for CSevenVoxels sample
	class CSevenVoxelTrainingSet : public CTrainingSet< CSevenVoxels >
	{
	};

    ///////////////////////////////////////////////////////////////////////////////
    //! CVoxelAverageVarianceTrainingSet
    //! - Training set for CVoxelAverageVariance sample
	class CVoxelAverageVarianceTrainingSet : public CTrainingSet< CVoxelAverageVariance >
	{
	};

    ///////////////////////////////////////////////////////////////////////////////
    //! CVoxelSampler
    //! - Segmentation handler subclassed for a particular volume sampler type
	template < class _VoxelSampler >
	class CVoxelSampler : public CASegmentationHandler< _VoxelSampler >
	{
		public:
			typedef CASegmentationHandler< _VoxelSampler > base;
			typedef typename base::tSampler tSampler;
			typedef typename base::tCoord tCoord;

		public:
            //! Constructor
			CVoxelSampler( OSGCanvas* canvas, scene::CSceneOSG * scene, tSampler * positive_sampler, tSampler * negative_sampler, bool retrain = true )
				: base( canvas, scene, positive_sampler, negative_sampler, retrain )
			{
			}

            //! Overriden sample collecting function
			virtual void collectSamples( tSampler * sampler )
			{
				typedef std::vector< osg::ref_ptr< osg::CLineGeode > >		tLineVector;
				typedef tLineVector::reverse_iterator						tLineRevIterator;

				for ( tLineRevIterator rit = this->m_linesVector.rbegin(); rit != this->m_linesVector.rend(); rit++ )
				{
					osg::Vec3Array * vertices = (*rit)->GetVertices();
					data::CObjectPtr< data::CDensityData >	volume( APP_STORAGE.getEntry( VPL_SIGNAL(SigGetActiveDataSet).invoke2() ) );

					data::CCoordinatesConv CoordConv = VPL_SIGNAL(SigGetActiveConvObject).invoke2();

					osg::Vec3Array::iterator	it1 = vertices->begin();
					osg::Vec3Array::iterator	it2 = vertices->begin();

					while ( it2 != vertices->end() )
					{
						if ( *it1 == *it2 ) it2++;
						else
						{					
							double x = CoordConv.fromSceneX( it1->operator [](0) );
							double y = CoordConv.fromSceneY( it1->operator [](1) );
							double z = CoordConv.fromSceneZ( it1->operator [](2) );

							osg::Vec3	coord( x, y, z );
							
							this->getSample( sampler, coord, *volume );							
							it1 = it2;
						}
					}
					
					volume.release();
				}
			}

            //! Overriden one sample collecting function
			virtual void getSample( tSampler * sampler, const tCoord & coord, typename tSampler::tData & data )
			{
				sampler->getSample( coord, data );
			}
	};


	//class CVoxelValueSegmentationHandler : public CASegmentationHandler< CVoxelValueSampler >
	//{
	//	public :

	//		CVoxelValueSegmentationHandler( scene::CSceneOSG * scene, tSampler * sampler, bool retrain = true ) :
	//			CASegmentationHandler( scene, sampler, retrain )
	//		{
	//		}

	//		virtual void collectSamples()
	//		{
	//			typedef std::vector< osg::ref_ptr< osg::CLineGeode > >		tLineVector;
	//			typedef tLineVector::reverse_iterator						tLineRevIterator;

	//			for ( tLineRevIterator rit = m_linesVector.rbegin(); rit != m_linesVector.rend(); rit++ )				
	//			{
	//				osg::Vec3Array * vertices = (*rit)->GetVertices();
	//				data::CObjectPtr< data::CDensityData >	volume	= APP_STORAGE.getEntryPtr( VPL_SIGNAL(SigGetActiveDataSet).invoke2() );

	//				osg::Vec3Array::iterator	it1 = vertices->begin();
	//				osg::Vec3Array::iterator	it2 = vertices->begin();

	//				while ( it2 != vertices->end() )
	//				{
	//					if ( *it1 == *it2 ) it2++;
	//					else
	//					{
	//						double x = ( it1->operator [](0) + ( volume->getDX() * volume->getXSize() ) / 2 ) / volume->getDX();
	//						double y = ( it1->operator [](1) + ( volume->getDY() * volume->getYSize() ) / 2 ) / volume->getDY();
	//						double z = ( it1->operator [](2) + ( volume->getDZ() * volume->getZSize() ) / 2 ) / volume->getDZ();

	//						osg::Vec3	coord( x, y, z );
	//						
	//						this->getSample( coord, *volume );							
	//						it1 = it2;
	//					}
	//				}
	//				
	//				volume.release();
	//			}
	//		}

	//		virtual void getSample( const tCoord & coord, tSampler::tData & data )
	//		{
	//			pSampler->getSample( coord, data );
	//		}


	//		virtual void doSegmentation()
	//		{

	//		}
	//};

}

#endif

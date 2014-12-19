///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// Copyright (c) 2009 by 3Dim Laboratory s.r.o.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __CABASE__
#define __CABASE__

#include <segmentation/CASTrainingSet.h>
#include <data/CDensityData.h>
#include <data/CRegionData.h>

namespace seg
{
    ///////////////////////////////////////////////////////////////////////////////
    //! CASSegmentation
    //! - Abstract automatic segmentation segmentation 
	class CASegmentation : public vpl::base::CObject
	{
		public :

			//! Smart pointer declaration
			VPL_SHAREDPTR( CASegmentation );

		public :
			
			//! Empty constructor
			CASegmentation()
			{
			}

			//! Virtual segmentation execution method
			virtual	void	execute( data::CDensityData & density, data::CRegionData & region, bool current_only = true )
            {
            };
	};

    ///////////////////////////////////////////////////////////////////////////////
    //! CATrainableSegmentation
    //! - Abstract automatic segmentation with supervised learning
	template < class _Sample >
	class CATrainableSegmentation : public CASegmentation
	{
		public :

			//! Smart pointer declaration
			VPL_SHAREDPTR( CATrainableSegmentation );

		public :

			//! Empty constructor
			CATrainableSegmentation()   {}

			//! Training method parametrized by positive and negative set
			virtual bool	train( CTrainingSet< _Sample > & positive_set, CTrainingSet< _Sample > & negetive_set  )
            {
                return false;
            };
	};

    ///////////////////////////////////////////////////////////////////////////////
    //! CASegmentationHandler
    //! - Template parametrized by a sampler, which collects voxel values from the volume
    //! - !! This class is obsolete, because of the new draw mode
	template < class _Sampler >
	class CASegmentationHandler : public osgGA::CSceneGeometryDrawEH
	{
		public :

			//! Sampler type
			typedef _Sampler	tSampler;

			//! Volume coordinate type
			typedef typename	tSampler::tCoord	tCoord;

		protected :

			//! Sampler instance for positive set
			typename _Sampler::tSmartPtr	 pPositiveSampler;

			//! Sampler instance for negative set
			typename _Sampler::tSmartPtr	 pNegativeSampler;

			//! Flag used when we want to retrain the whole segmentation
			bool							 bRetrain;

			//! Segmentation object pointer
			CASegmentation::tSmartPtr		 pSegmentation;

		public :

			//! Constructor parametrized by scene pointer and two samplers
			CASegmentationHandler( OSGCanvas * canvas, scene::CSceneOSG * scene, tSampler * positive_sampler, tSampler * negetive_sampler, bool retrain = true ) : 
			   CSceneGeometryDrawEH( canvas, scene ),
			   pPositiveSampler( positive_sampler ),
			   pNegativeSampler( negetive_sampler ),
			   bRetrain( retrain )
			{
			}

			//! Virtual destructor
		    virtual ~CASegmentationHandler()
			{

			}

			//! Turns retraining on/off
			void	setRetrain( bool state )
			{
				bRetrain = state;
			}

			//! Assigns segmentation object 
			void	setSegmentation( CASegmentation * seg )
			{
				pSegmentation = seg;
			}

			//! Overriden baseclass method which returns true if handler is active
			virtual bool UseHandler() 
			{ 				
				return ( APP_MODE.get() == scene::CAppMode::COMMAND_AUTO_SEGMENTATION );
			}

			//! Method called when mouse is released
			virtual void OnMouseRelease( const osgGA::CMousePoint & point )
			{				
				try 
				{
					if ( point.m_buttonEvent == 4 )		this->collectSamples( pNegativeSampler.get() );
					else								this->collectSamples( pPositiveSampler.get() );

					this->doSegmentation();
					this->clearData();
				}
				catch( ... )
				{
				}
			}

			//! Method called when mouse is clicked
			virtual void OnMousePush( const osgGA::CMousePoint & point )
			{
			}

			//! This method will do the segmentation and should be overriden
			virtual void doSegmentation()
			{
			}

			//! This method collects samples from mouse movement and should be overriden
			virtual void collectSamples( tSampler * sampler )
			{
			}

			//! Loads one sample from specified coordinates into the volume
			virtual void getSample( tSampler * sampler, const tCoord & coord, typename tSampler::tData & data )
			{
			}

			//! Clears the data from both samplers
			virtual void clearData()
			{
				if ( !bRetrain ) 
				{
					pPositiveSampler->set().clear();
					pNegativeSampler->set().clear();
				};

				this->ClearLines();
			}

	};

}

#endif
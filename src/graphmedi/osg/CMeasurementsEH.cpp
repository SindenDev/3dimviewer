///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// 3DimViewer
// Lightweight 3D DICOM viewer.
//
// Copyright 2008-2016 3Dim Laboratory s.r.o.
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

#include <osg/CMeasurementsEH.h>
#include <data/CMeasurementOptions.h>
#include <osg/Billboard>
#include <osgText/Text>
#include <osg/Version>
#include <sstream>
#include <app/Signals.h>
#include <osg/CThickLineMaterial.h>
#include <iomanip>

using namespace scene;

/******************************************************************************
	CLASS CMeasurementsEH
******************************************************************************/

//**************************************************
// Constructor
CMeasurementsEH::CMeasurementsEH( OSGCanvas * canvas,scene::CSceneBase * scene, bool handleDistance /* = true */, bool handleDensity /* = true */, bool handleDensityUnderCursor /* = false */ )
: CEventHandlerBase( canvas )
,  m_scene( scene )
,  m_handleDistance( handleDistance )
,  m_handleDensity( handleDensity )
,  m_handleDensityUnderCursor( handleDensityUnderCursor )
{
	// Initialize hit prospector
	m_desiredNodes = new osg::CNodeTypeIntersectionDesired<osgUtil::LineSegmentIntersector::Intersection, scene::CSliceGeode>;
    m_desiredList = new osg::CNodeListIntersectionDesired<osgUtil::LineSegmentIntersector::Intersection>;
	m_ip.addDesiredRule(m_desiredNodes);
    m_ip.addDesiredRule( m_desiredList );
	m_ip.useDesired(true);

    // Connect to the measurements parameter changed signal
    APP_MODE.getMeasuringParametersSignal().connect( this, &CMeasurementsEH::OnParametersChanged );

    m_gizmoLineMaterial = new osg::CMaterialLines(canvas->getView()->getCamera(), 2.0f);
}

//**************************************************
// Destructor
CMeasurementsEH::~CMeasurementsEH()
{
}

//**************************************************
// Handle operations
bool CMeasurementsEH::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa, osg::Object* o, osg::NodeVisitor* v)
{
	if ( ( APP_MODE.get() == scene::CAppMode::COMMAND_DENSITY_MEASURE ) && m_handleDensity )
        if (handleDensity(ea, aa, o, v))
            return true;

	if ( ( APP_MODE.get() == scene::CAppMode::COMMAND_DISTANCE_MEASURE ) && m_handleDistance )
        if (handleDistance(ea, aa, o, v))
            return true;

    if (m_handleDensityUnderCursor)
        return handleDensityUnderCursor(ea,aa,o,v);

	return false;
}

//*************************************************
// Handle distance measurement
bool CMeasurementsEH::handleDistance( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa, osg::Object*, osg::NodeVisitor* )
{
	// Get adapters
	osgGA::GUIEventAdapter * p_EventAdapter = const_cast< osgGA::GUIEventAdapter *>( &ea );
	osgGA::GUIActionAdapter * p_ActionAdapter = const_cast< osgGA::GUIActionAdapter *>( &aa );

	// get viewer pointer, if not possible, chicken out
	osg::ref_ptr<osgViewer::View> view = dynamic_cast<osgViewer::View *>( p_ActionAdapter );
	if ( !view )  return false;

	//intersections with interest geometry.
	osgUtil::LineSegmentIntersector::Intersection intersection;

	osg::Vec3 intersectionPoint;
	osg::Matrix unOrthoMatrix(osg::Matrix::inverse( m_scene->getOrthoTransformMatrix() ) );

	// decide what to do according to how mouse behaves
	switch ( p_EventAdapter->getEventType() )
	{
	case osgGA::GUIEventAdapter::PUSH :

		// get and store starting point
		if(!computeIntersections(ea, aa, m_scene.get(), intersection))
			return false;

		// Compute intersection as world coordinate
		m_start = intersection.getWorldIntersectPoint() * unOrthoMatrix;
		m_startN = intersection.getWorldIntersectNormal() * unOrthoMatrix;
        m_startN.normalize();
        modifyCreationCoordinates(m_start, m_startN);
		m_startN.normalize();

		// Create ruler geometry
		m_ruler = new CRulerGizmo;
        m_ruler->setMaterial(m_gizmoLineMaterial);

		// Add ruler to the scene
		m_scene->addGizmo( m_ruler.get() );

		// Modify ruler 
		m_ruler->update( m_start, m_startN, m_start, m_startN );
		m_ruler->show();

		break;

	case osgGA::GUIEventAdapter::DRAG :

		// get current mouse position
		if(computeIntersections(ea, aa, m_scene.get(), intersection)){
			m_end = intersection.getWorldIntersectPoint() * unOrthoMatrix;
			m_endN = intersection.getWorldIntersectNormal() * unOrthoMatrix;
            m_endN.normalize();
            modifyCreationCoordinates(m_end, m_endN);
			m_endN.normalize();

			// Modify ruler
            if (m_ruler)
                m_ruler->update( m_start, m_startN, m_end, m_endN );

			osg::Vec3d	distance = m_start - m_end;
			APP_MODE.getDistanceMeasureSignal().invoke( distance.length() );
		}

		break;

	case osgGA::GUIEventAdapter::RELEASE :


		// Switch distance measurement off (button too :))
		//APP_MODE.set(scene::CAppMode::DEFAULT_MODE);

		// Remove gizmos
		//m_scene->clearGizmos();

		break;

	default : 
		// This event is not for me...
		return false;

		break;
	}

	// All OK
	return true;
}

//*************************************************
// Handle density measurement
bool CMeasurementsEH::handleDensity( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa, osg::Object*, osg::NodeVisitor* )
{
	// computed density
	double density;

	// Density gizmo
	osg::ref_ptr< CDensityGizmo > gizmo;

	// Get adapters
	osgGA::GUIEventAdapter * p_EventAdapter = const_cast< osgGA::GUIEventAdapter *>( &ea );
	osgGA::GUIActionAdapter * p_ActionAdapter = const_cast< osgGA::GUIActionAdapter *>( &aa );

	// get viewer pointer, if not possible, chicken out
	osg::ref_ptr<osgViewer::View> view = dynamic_cast<osgViewer::View *>( p_ActionAdapter );
	if ( !view )  return false;

	//intersections with interest geometry.
	osgUtil::LineSegmentIntersector::Intersection intersection;

	osg::Vec3 intersectionPoint, volumePoint;
	osg::Matrix unOrthoMatrix(osg::Matrix::inverse( m_scene->getOrthoTransformMatrix() ) );

	// Get density volume from storage
    int datasetId = VPL_SIGNAL(SigGetActiveDataSet).invoke2();
    if (datasetId == data::CUSTOM_DATA)
    {
        return false;
    }
    data::CObjectPtr< data::CDensityData > pVolume( APP_STORAGE.getEntry(datasetId) );

	// Get CONVERSION OBJECT
	data::CCoordinatesConv CoordConv = VPL_SIGNAL(SigGetActiveConvObject).invoke2();

	// decide what to do according to how mouse behaves
	switch ( p_EventAdapter->getEventType() )
	{
	case osgGA::GUIEventAdapter::PUSH :
        {

		    // get and store starting point

		    if(!computeIntersections(ea, aa, m_scene.get(), intersection))
			    return false;

		    // Compute intersection as world coordinate
		    m_start = intersection.getWorldIntersectPoint()  * unOrthoMatrix;
            osg::Vec3 normal( intersection.getWorldIntersectNormal()  * unOrthoMatrix );
            normal.normalize();

            volumePoint[ 0 ] = CoordConv.fromSceneX( m_start.x() );
		    volumePoint[ 1 ] = CoordConv.fromSceneY( m_start.y() );
		    volumePoint[ 2 ] = CoordConv.fromSceneZ( m_start.z() );

            modifyCreationCoordinates(m_start, normal);
            normal.normalize();
    		
    		
		    

		    // Compute density 
		    density = m_collector.computeDensity( pVolume.get(), volumePoint );
		    APP_MODE.getDensityMeasureSignal().invoke( density );

		    // Create and add gizmo
		    gizmo = new CDensityGizmo( density, 1, m_start, normal , unOrthoMatrix );

		    m_scene->addGizmo( gizmo.get() );
        }
		break;

//	case osgGA::GUIEventAdapter::DRAG :
	case osgGA::GUIEventAdapter::RELEASE :

		// Switch distance measurement off (button too :))
		// APP_MODE.restore();

		// Remove gizmos
		//m_scene->clearGizmos();

		break;


	default : 
		// This event is not for me...
		return false;

		break;
	}

	// All OK
	return true;
}

void CMeasurementsEH::setHandleDensityUnderCursor(bool bSet)
{
    m_handleDensityUnderCursor=bSet;
}

//*************************************************
// Handle density measurement
bool CMeasurementsEH::handleDensityUnderCursor( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa, osg::Object*, osg::NodeVisitor* )
{
    // Get adapters
    osgGA::GUIEventAdapter * p_EventAdapter = const_cast< osgGA::GUIEventAdapter *>( &ea );
    osgGA::GUIActionAdapter * p_ActionAdapter = const_cast< osgGA::GUIActionAdapter *>( &aa );

    // get viewer pointer, if not possible, chicken out
    osg::ref_ptr<osgViewer::View> view = dynamic_cast<osgViewer::View *>( p_ActionAdapter );
    if ( !view )  return false;

    //intersections with interest geometry.
    osgUtil::LineSegmentIntersector::Intersection intersection;

    osg::Vec3 volumePoint;
    osg::Matrix unOrthoMatrix(osg::Matrix::inverse( m_scene->getOrthoTransformMatrix() ) );

    // Get density volume from storage
    int datasetId = VPL_SIGNAL(SigGetActiveDataSet).invoke2();
    if (datasetId == data::CUSTOM_DATA)
    {
        return false;
    }
    data::CObjectPtr< data::CDensityData > pVolume(APP_STORAGE.getEntry(datasetId));

    // Get CONVERSION OBJECT
    data::CCoordinatesConv CoordConv = VPL_SIGNAL(SigGetActiveConvObject).invoke2();

    // decide what to do according to how mouse behaves
    switch ( p_EventAdapter->getEventType() )
    {
    case osgGA::GUIEventAdapter::MOVE :
        {
            if(!computeIntersections(ea, aa, m_scene.get(), intersection, MASK_DRAGGABLE_SLICE_DRAGGER|MASK_VISIBLE_OBJECT))
            {
                APP_MODE.getContinuousDensityMeasureSignal().invoke( -1000 ); // air
                return true;
            }

            // Compute intersection as world coordinate
            osg::Vec3 start = intersection.getWorldIntersectPoint()  * unOrthoMatrix;

            volumePoint[ 0 ] = CoordConv.fromSceneX( start.x() );
            volumePoint[ 1 ] = CoordConv.fromSceneY( start.y() );
            volumePoint[ 2 ] = CoordConv.fromSceneZ( start.z() );

            // Compute density
            CDensitySolver::EMode prevMode = m_collector.getMode();
            m_collector.setMode( CDensitySolver::SINGLE );
            double density = m_collector.computeDensity( pVolume.get(), volumePoint );
            m_collector.setMode( prevMode );
            APP_MODE.getContinuousDensityMeasureSignal().invoke( density );
        }
        break;
    default :
        // This event is not for me...
        return false;

        break;
    }

    // All OK
    return true;
}

//***********************************************************
// Compute intersection
bool CMeasurementsEH::computeIntersections(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa, osg::Node * scene_root, osgUtil::LineSegmentIntersector::Intersection & intersection, osg::Node::NodeMask traversalMask)
{
	osgUtil::LineSegmentIntersector::Intersections allIntersections, usableIntersections;

	osgViewer::View * view = dynamic_cast<osgViewer::View*>( &aa );

    view->computeIntersections(ea.getX(), ea.getY(), allIntersections, traversalMask);

	// sort intersections
//	std::stable_sort(allIntersections.begin(), allIntersections.end());

	if(m_ip.prospect(allIntersections, usableIntersections) )
	{
		// intersection found
		intersection = *usableIntersections.begin();
		return true;
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   ! On parameters changed - signal response. 
//!
//!\param   tool    The tool. 
//!\param   flag    The flag. 
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMeasurementsEH::OnParametersChanged(int tool, int flag)
{
    if( tool == DENSITY )
    {
        // Set density measurement mode
        switch( flag )
        {
        case 0:
            m_collector.setMode( CDensitySolver::SINGLE );
            break;

        case 1:
            m_collector.setMode( CDensitySolver::AVERAGE );
            break;

        case 2:
            m_collector.setMode( CDensitySolver::MEDIAN );
            break;

        default:;

        }
    }
}


///////////////////////////////////////////////////////////////////////////////
// On app mode changed signal response
void CMeasurementsEH::OnModeChanged(scene::CAppMode::tMode mode)
{
	if( mode != scene::CAppMode::COMMAND_DENSITY_MEASURE && mode != scene::CAppMode::COMMAND_DISTANCE_MEASURE )
	{
		m_scene->clearGizmos();
	}
}

/**************************************************************************
	Density collector
**************************************************************************/
////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   ! Constructor. 
////////////////////////////////////////////////////////////////////////////////////////////////////
CDensitySolver::CDensitySolver(void)
    : m_mode( AVERAGE )
{
}
//-------------------------------------------------------------------------
// Test if position is inside volume
bool CDensitySolver::testPosition( data::CDensityData * volume, osg::Vec3 & position )
{
	// Get volume sizes
	osg::Vec3 size( volume->getXSize(), volume->getYSize(), volume->getZSize() );
/*
	bool retval;
	retval =	( position[0] < size[0] && position[0] >= 0.0 ) && 
				( position[1] < size[1] && position[1] >= 0.0 ) &&
				( position[2] < size[2] && position[2] >= 0.0 );

	// Check position
	return retval;
*/
	for( int i = 0; i < 3; ++i )
	{
		if( position[ i ] < 0.0 )
			position[ i ] = 0.0;
		if( position[ i ] >= size[ i ] )
			position[ i ] = size[ i ] - 1;
	}
	return true;
}

double roundToNearest(double num) {
    return (num > 0.0) ? floor(num + 0.5) : ceil(num - 0.5);
}


//-------------------------------------------------------------------------
// Compute density
vpl::img::tDensityPixel CDensitySolver::computeDensity(data::CDensityData *volume, osg::Vec3 position)
{
    double pixel( 0.0 );

    switch(m_mode)
    {
    case SINGLE:
        if( testPosition( volume, position ) )
        {
            pixel = volume->at( (vpl::tSize)position.x(), (vpl::tSize)position.y(), (vpl::tSize)position.z() );
        }
        break;

    case AVERAGE:
        // Test if position is within volume
        if( testPosition( volume, position ) )
        {
/*            for( int z = -1; z < 2; ++z )
                for( int y = -1; y < 2; ++y )
                    for( int x = -1; x <= 1; ++x)
                        pixel += volume->at( (vpl::tSize)position.x() + x, (vpl::tSize)position.y() + y, (vpl::tSize)position.z() + z ); 
            pixel /= (3*3*3);*/
            vpl::img::CVolumeAvg3Filter<vpl::img::CDensityVolume> Filter;
            pixel = Filter.getResponse(*volume, (vpl::tSize)position.x(), (vpl::tSize)position.y(), (vpl::tSize)position.z());
        }
        break;

    case MEDIAN:
        if( testPosition(volume, position ) )
        {
/*            count = 0;
            // Copy voxels from the window to the buffer
            for( int z = -1; z < 2; ++z )
                for( int y = -1; y < 2; ++y )
                    for( int x = -1; x <= 1; ++x)
                        m_buffer[ count++ ] = volume->at( (vpl::tSize)position.x() + x, (vpl::tSize)position.y() + y, (vpl::tSize)position.z() + z ); 

            pixel = vpl::img::median::findMedian( m_buffer );*/
            vpl::img::CVolumeMedianFilter<vpl::img::CDensityVolume> Filter(3);
            pixel = Filter.getResponse(*volume, (vpl::tSize)position.x(), (vpl::tSize)position.y(), (vpl::tSize)position.z());
        }
        break;
    
    default:
        pixel = 0.0;
    }

	return pixel;
}


/**************************************************************************
		Ruler 
**************************************************************************/

//***********************************************************
// Constructor
CRulerGizmo::CRulerGizmo()
{
	// Get colors from the storage
	data::CObjectPtr< data::CMeasurementOptions > ptrOptions( APP_STORAGE.getEntry(data::Storage::MeasurementOptions::Id) );
	m_lineColor = ptrOptions->GetRulerLineColor();
	m_textColor = ptrOptions->GetRulerFontColor();
	m_shadowColor = ptrOptions->GetRulerFontShadowColor();

	int i;

	// create geode
	m_lineGeode = new osg::Geode;
	{
		m_vertices = new osg::Vec3Array( 2 );

		for( i = 0; i < 2; ++i )
			m_vertices->operator [](0) = osg::Vec3(0.0, 0.0, 0.0);

		// Create line geometry
		m_lineGeometry = new osg::Geometry;

		m_lineGeometry->setVertexArray(m_vertices.get());
		
		m_lineGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES,0,2));

		// Line color
		osg::ref_ptr<osg::Vec4Array> color = new osg::Vec4Array;
		color->push_back( m_lineColor );
		m_lineGeometry->setColorArray(color.get(), osg::Array::BIND_OVERALL);

		m_lineGeode->addDrawable(m_lineGeometry.get());
	}

	// Crosses
	{
		osg::ref_ptr< osg::Geometry > crossGeometry[ 2 ];
		osg::ref_ptr< osg::Vec3Array > crossVertices[ 2 ];
		osg::ref_ptr< osg::Geode > crossGeodes[ 2 ];


		// Create and init cross geometries, geodes and mt.
		for( i = 0; i < 2; ++i )
		{
			m_mt[ i ] = new osg::MatrixTransform;

			crossGeometry[ i ] = new osg::Geometry;

			crossVertices[ i ] = new osg::Vec3Array( 4 );

			// Points
			crossVertices[ i ]->operator [](0) = osg::Vec3( 1.0, 0.0, 0.0 );
			crossVertices[ i ]->operator [](1) = osg::Vec3( -1.0, 0.0, 0.0 );
			crossVertices[ i ]->operator [](2) = osg::Vec3( 0.0, 1.0, 0.0 );
			crossVertices[ i ]->operator [](3) = osg::Vec3( 0.0, -1.0, 0.0 );

			crossGeometry[ i ]->setVertexArray( crossVertices[ i ].get() );
			crossGeometry[ i ]->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES,0,2));
			crossGeometry[ i ]->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES,2,2));

			// Geodes
			crossGeodes[ i ] = new osg::Geode;
			crossGeodes[ i ]->addDrawable( crossGeometry[ i ].get() );

			// Color
			osg::ref_ptr<osg::Vec4Array> color = new osg::Vec4Array;
			color->push_back( m_lineColor );
            crossGeometry[ i ]->setColorArray(color.get(), osg::Array::BIND_OVERALL);

			crossGeodes[ i ]->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON );

			// Disable depth testing so geometry is drawn regardless of depth values
			// of geometry already draw.
			crossGeodes[ i ]->getOrCreateStateSet()->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
			crossGeodes[ i ]->getOrCreateStateSet()->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );

			m_mt[ i ]->addChild( crossGeodes[ i ].get() );

			addChild( m_mt[ i ].get() );
		}
	}

	// Turn of lighting for line and set line width.
	{
		m_lineGeode->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON );

		// Disable depth testing so geometry is draw regardless of depth values
		// of geometry already draw.
		m_lineGeode->getOrCreateStateSet()->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
		m_lineGeode->getOrCreateStateSet()->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );

		// Disable culling
		setCullingActive(false);
	}

	// Add text
	m_text = new osgText::Text;
	osgText::Font* font = osgText::readFontFile( ptrOptions->GetRulerFontName().c_str() );
	m_text->setFont( font );


	m_text->setAutoRotateToScreen(true);
    m_text->setDataVariance(osg::Object::DYNAMIC);
	m_text->setCharacterSizeMode( osgText::Text::SCREEN_COORDS );
	m_text->setFontResolution( 20, 20 );
	m_text->setCharacterSize( ptrOptions->GetRulerFontSize() );

	m_text->setColor( m_textColor );
	m_text->setBackdropType( osgText::Text::OUTLINE );
	m_text->setBackdropOffset( 0.07f );
	m_text->setBackdropColor( m_shadowColor );
	m_text->setAlignment( osgText::Text::CENTER_CENTER );
	m_lineGeode->addDrawable( m_text.get() );


	// add line to the current dragger
	addChild(m_lineGeode.get());


	
}

//***********************************************************
// Destructor
CRulerGizmo::~CRulerGizmo()
{

}

//***********************************************************
// Show/hide
void CRulerGizmo::show( bool bShow /* = true */ )
{
	if(bShow)
		setAllChildrenOn();
	else
		setAllChildrenOff();
}

//***********************************************************
// Recompute geometry
void CRulerGizmo::update( const osg::Vec3 & spoint, const osg::Vec3 & snormal, const osg::Vec3 & epoint, const osg::Vec3 & enormal )
{
	osg::Vec3 point, normal;

	m_vertices->operator[](0) = spoint;
	m_vertices->operator[](1) = epoint;

	// Crosses
	m_mt[ 0 ]->setMatrix( osg::Matrix::rotate( osg::Vec3( 0.0, 0.0, 1.0 ), snormal ) * osg::Matrix::translate( spoint ) );
	m_mt[ 1 ]->setMatrix( osg::Matrix::rotate( osg::Vec3( 0.0, 0.0, 1.0 ), enormal ) * osg::Matrix::translate( epoint ) );

	// Ruler line
	osg::Vec3 vector( epoint[0] - spoint[0], epoint[1] - spoint[1], epoint[2] - spoint[2] );
	osg::Vec3 textPosition( (epoint[0] + spoint[0])/2, (epoint[1] + spoint[1])/2, (epoint[2] + spoint[2])/2 );
	
	// Set text
	std::ostringstream os;
	m_text->setPosition( textPosition );
	os << std::fixed << std::setprecision( 1 ) << vector.length();
	m_text->setText( os.str() );

	m_vertices->dirty();

	m_lineGeometry->setVertexArray(m_vertices.get());
}

void scene::CRulerGizmo::setMaterial(osg::CMaterialLines * material)
{
    m_gizmoLineMaterial = material;
    m_gizmoLineMaterial->apply(m_lineGeometry);
    m_gizmoLineMaterial->apply(m_mt[0]);
    m_gizmoLineMaterial->apply(m_mt[1]);
}

/******************************************************************************
	Density meter gizmo
******************************************************************************/

//-----------------------------------------------------------------------------
// Constructor
CDensityGizmo::CDensityGizmo( const double value, const double radius, const osg::Vec3 & position, const osg::Vec3 & normal, const osg::Matrix & unOrthoMatrix )
{
	// Get colors from the storage
	data::CObjectPtr< data::CMeasurementOptions > ptrOptions( APP_STORAGE.getEntry(data::Storage::MeasurementOptions::Id) );
	m_lineColor = ptrOptions->GetDropperLineColor();
	m_textColor = ptrOptions->GetDropperFontColor();
	m_shadowColor = ptrOptions->GetDropperFontShadowColor();

	float x, y;

	float angle = 2*osg::PI/16.0;
//	int n;

	// create geode
	m_geode = new osg::Geode;

	// Create text geode
	m_textGeode = new osg::Geode;

	{
		m_vertices = new osg::Vec3Array;

		for( int i = 0; i < 16; ++i )
		{
			x = radius*cos( i * angle );
			y = radius*sin( i * angle );

			m_vertices->push_back( osg::Vec3( x, y, 0 ) );
		}

		// Create line geometry
		m_geometry = new osg::Geometry;

		m_geometry->setVertexArray(m_vertices.get());
		m_geometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES,0, m_vertices->size()));

		// Line color
		osg::ref_ptr<osg::Vec4Array> color = new osg::Vec4Array;
		color->push_back( m_lineColor );
		m_geometry->setColorArray(color.get(), osg::Array::BIND_OVERALL);

		m_geode->addDrawable(m_geometry.get());
	}

	// Turn of lighting for line and set line width.
	{
		m_geode->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON );


		// Disable depth testing so geometry is draw regardless of depth values
		// of geometry already drawn.
		m_geode->getOrCreateStateSet()->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
		m_geode->getOrCreateStateSet()->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );

		// Disable culling
		setCullingActive(false);
	}

	// Set text geode parameters
	{
//		m_textGeode->getOrCreateStateSet()->setAttributeAndModes(linewidth, osg::StateAttribute::ON);
		m_textGeode->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON );
		m_textGeode->getOrCreateStateSet()->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
		m_textGeode->getOrCreateStateSet()->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
	}

	// Get volume size
	data::CCoordinatesConv CoordConv = VPL_SIGNAL(SigGetActiveConvObject).invoke2();
	

	// Value text
	osgText::Text * text = new osgText::Text;

	text = new osgText::Text;
	osgText::Font* font = osgText::readFontFile( ptrOptions->GetDropperFontName().c_str() );
	text->setFont( font );

	//text->setPosition( osg::Vec3( 1.5*radius, 1.5*radius, 0.0 ) );
	text->setAutoRotateToScreen(true);
	text->setCharacterSize( ptrOptions->GetDropperFontSize() );
	text->setFontResolution( 20, 20 );
	text->setCharacterSizeMode( osgText::Text::SCREEN_COORDS );
	text->setColor( m_textColor );
	text->setBackdropType( osgText::Text::OUTLINE );
	text->setBackdropColor( m_shadowColor );
	m_textGeode->addDrawable( text );


	// Set value as text
	std::ostringstream oss;
	oss << value;
	text->setText( oss.str() );

	// Ortho rotate transform
	osg::Matrix orthoRotate( osg::Matrix::rotate( unOrthoMatrix.getRotate() ) );

	// Matrix transform - circle
	osg::ref_ptr<osg::MatrixTransform > transformCircle = new osg::MatrixTransform;
	transformCircle->setMatrix(osg::Matrix::rotate( osg::Vec3( 0.0, 0.0, 1.0 ), normal )* osg::Matrix::translate( position ) );
	transformCircle->addChild( m_geode.get() );

	// Matrix transform - text
	osg::ref_ptr<osg::MatrixTransform > transformText = new osg::MatrixTransform;
	
	osg::Vec3 textShift( 1.5 * radius, 1.5 * radius, 1.5 * radius );
	textShift = textShift * orthoRotate;

	transformText->setMatrix( osg::Matrix::translate( position + textShift ) );
	transformText->addChild( m_textGeode.get() );

	addChild( transformText.get() );
	addChild( transformCircle.get() );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   ! Modify data used to create implant. 
//!
//!\param [in,out]  position    the position. 
//!\param [in,out]  direction   the direction. 
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMeasurementsEH::modifyCreationCoordinates(osg::Vec3 &position, osg::Vec3 &direction)
{
    // Get conversion object
    data::CCoordinatesConv CoordConv = VPL_SIGNAL(SigGetActiveConvObject).invoke2();	


    // Recalculate coordinates from scene to data space
    osg::Vec3d buf;
    CoordConv.unshift( position[0], position[1], position[2], buf[0], buf[1], buf[2] );
    position = buf;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   ! Add intersection desired node. 
//!
//!\param [in,out]  node    If non-null, the node. 
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMeasurementsEH::addDesiredNode(osg::Node *node)
{
    if( node != NULL )
        m_desiredList->addNode( node );
}

//! Constructor
CMeasurements3DEH::CMeasurements3DEH(OSGCanvas *canvas, scene::CSceneBase *scene)
    : CMeasurementsEH(canvas, scene)
{
    // Connect on dummy
    data::CGeneralObjectObserver<CMeasurements3DEH>::connect(APP_STORAGE.getEntry(data::Storage::SceneManipulatorDummy::Id).get(), data::CGeneralObjectObserver<CMeasurements3DEH>::tObserverHandler(this, &CMeasurements3DEH::sigSceneManipulatorDummyChanged));

    // Clear gizmos when slice moved
    m_conSliceXYMoved = VPL_SIGNAL(SigSetSliceXY).connect(this, &CMeasurements3DEH::SigSliceMoved);
    m_conSliceXZMoved = VPL_SIGNAL(SigSetSliceXZ).connect(this, &CMeasurements3DEH::SigSliceMoved);
    m_conSliceYZMoved = VPL_SIGNAL(SigSetSliceYZ).connect(this, &CMeasurements3DEH::SigSliceMoved);
}

//!
void CMeasurements3DEH::objectChanged(data::CStorageEntry *pEntry, const data::CChangedEntries &changes)
{ }

//! Clear gizmos when object has changed
void CMeasurements3DEH::sigSceneManipulatorDummyChanged(data::CStorageEntry *pEntry, const data::CChangedEntries &changes)
{
    m_scene->clearGizmos();
}

//! Slice moved signal answer
void CMeasurements3DEH::SigSliceMoved(int position)
{
    CMeasurementsEH::m_scene->clearGizmos();
}

//! Constructor
CMeasurementsRtgEH::CMeasurementsRtgEH(OSGCanvas *canvas, scene::CSceneBase *scene)
    : CMeasurementsEH(canvas, scene)
{
    m_ip.addDesiredRule(new osg::CNodeTypeIntersectionDesired<osgUtil::LineSegmentIntersector::Intersection, osg::Geode>);

    // Connect on dummy
    data::CGeneralObjectObserver<CMeasurementsRtgEH>::connect(APP_STORAGE.getEntry(data::Storage::SceneManipulatorDummy::Id).get(), data::CGeneralObjectObserver<CMeasurementsRtgEH>::tObserverHandler(this, &CMeasurementsRtgEH::sigSceneManipulatorDummyChanged));
}

//! Clear gizmos when object has changed
void CMeasurementsRtgEH::objectChanged(data::CStorageEntry *pData, const data::CChangedEntries &changes)
{ }

//! Clear gizmos when object has changed
void CMeasurementsRtgEH::sigSceneManipulatorDummyChanged(data::CStorageEntry *pData, const data::CChangedEntries &changes)
{
    m_scene->clearGizmos();
}

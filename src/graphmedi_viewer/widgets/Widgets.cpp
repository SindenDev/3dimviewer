///////////////////////////////////////////////////////////////////////////////
// $Id: Widgets.cpp 1916 2012-05-23 11:50:16Z tryhuk $
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

#include <widgets/Widgets.h>

#include <data/CSceneWidgetParameters.h>
#include <data/CMeasurementOptions.h>
#include <data/CSceneManipulatorDummy.h>

#include <osg/ShapeDrawable>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgWidget/WindowManager>
#include <osgText/Text>
#include <osgText/Font>
#include <osg/CullFace>
#include <osg/Shape>
#include <osg/Version>
#include <cmath>

#include <osg/CThickLineMaterial.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
//

osgWidget::Label* scene::createLabelWidget(const std::string& l, 
                                           unsigned int font_size, 
                                           const osgWidget::Color &fontColor, 
                                           const osgWidget::Color &backgroundColor
                                           )
{
    osgWidget::Label* label = new osgWidget::Label("", "");

    label->setFont( DEFAULT_FONT_NAME );
    label->setFontSize( font_size );
    label->setFontColor( fontColor );
    label->setColor( backgroundColor );
    label->setLabel( l );

    return label;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//

osgWidget::Widget* scene::createImageWidget(const std::string &filename)
{
    if( !osgDB::findDataFile(filename).size() )
    {
        // Cannot find image
        return NULL;
    }

    // Read the image
    osg::Image * image = osgDB::readImageFile(filename);
    if( !image )
    {
        return NULL;
    }

    osgWidget::Widget * widget = new osgWidget::Widget();
    if( widget->setImage( image ) )
    {
        // Set texturing coordinates
        widget->setTexCoord(0.0f, 0.0f, osgWidget::Widget::LOWER_LEFT);
        widget->setTexCoord(1.0f, 0.0f, osgWidget::Widget::LOWER_RIGHT);
        widget->setTexCoord(1.0f, 1.0f, osgWidget::Widget::UPPER_RIGHT);
        widget->setTexCoord(0.0f, 1.0f, osgWidget::Widget::UPPER_LEFT);

        // Set widget size to the image sizes
        widget->setSize( image->s(), image->t() );

        return widget;
    }

    // Unknown error 
    delete widget;
    return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//

scene::CSceneInfoWidget::CSceneInfoWidget(OSGCanvas *pCanvas)
    : osgWidget::Box("InfoBox", osgWidget::Box::VERTICAL)
{
    setCanvas(pCanvas);
    scene::CGeneralObjectObserverOSG<CSceneInfoWidget>::connect(APP_STORAGE.getEntry(data::Storage::SceneWidgetsParameters::Id).get());
    scene::CGeneralObjectObserverOSG<CSceneInfoWidget>::connect(APP_STORAGE.getEntry(data::Storage::PatientData::Id).get());
    this->setupObserver(this);

    // Get colors from the storage
    data::CObjectPtr< data::CSceneWidgetParameters > ptrOptions( APP_STORAGE.getEntry(data::Storage::SceneWidgetsParameters::Id) );
	osg::Vec4 color(ptrOptions->getTextColor());

    // Create labels
	m_labelPatientName = createLabel( "No data", color );
    m_labelPatientId = createLabel( "", color );
    m_labelDateTime = createLabel( "", color );

    // Create box
    this->attachMoveCallback();
    this->attachScaleCallback();
    this->attachRotateCallback();
    this->getBackground()->setColor( ptrOptions->getBackgroundColor() );
    this->setAnchorHorizontal( osgWidget::Window::HA_LEFT );
    this->setAnchorVertical( osgWidget::Window::VA_TOP );

    // Shift it
    this->addOrigin( 15, 15 );

    // Add labels
    this->addWidget( m_labelDateTime.get() );
	this->addWidget( m_labelPatientId.get() );
    this->addWidget( m_labelPatientName.get() );

    // Resize box
    this->resize();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//

void scene::CSceneInfoWidget::updateFromStorage()
{
    // Set output text
   data::CObjectPtr< data::CDensityData > data( APP_STORAGE.getEntry(data::Storage::PatientData::Id) );

   if( data->m_sPatientName.empty() )
   {
        m_labelPatientName->setLabel( "Anonymous patient" );
   }
   else
   {
        m_labelPatientName->setLabel( data->m_sModality + std::string(", ") + data->m_sPatientName );
   }

   if( data->m_sPatientId.empty() )
   {
        m_labelPatientId->setLabel( "" );
   }
   else
   {
        m_labelPatientId->setLabel( data->m_sPatientId );
   }

   if( data->m_sSeriesDate.empty() )
   {
        m_labelDateTime->setLabel( "" );
   }
   else
   {
        m_labelDateTime->setLabel( data->m_sSeriesDate + std::string(", ") + data->m_sSeriesTime );
   }

   this->resize();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//

scene::CSceneOrientationWidget::CSceneOrientationWidget(OSGCanvas *pCanvas,
                                                        int sx, int sy,
                                                        const osg::Matrix &orientation,
                                                        int Flags
                                                        )
    : osgWidget::Box("InfoBox", osgWidget::Box::VERTICAL)
    , m_Flags( Flags )
{
    setCanvas(pCanvas);
    scene::CGeneralObjectObserverOSG<CSceneOrientationWidget>::connect(APP_STORAGE.getEntry(data::Storage::PreviewModel::Id).get());
    this->setupObserver(this);

    VPL_ASSERT( pCanvas );
    m_viewer = pCanvas->getView();

    // Model view matrix
    osg::MatrixTransform * modelView = new osg::MatrixTransform( orientation );

    // Get colors from the storage
    data::CObjectPtr< data::CSceneWidgetParameters > ptrOptions( APP_STORAGE.getEntry(data::Storage::SceneWidgetsParameters::Id) );
    double scaleFactor = ptrOptions->getWidgetsScale();
    sx *= scaleFactor;
    sy *= scaleFactor;

    // Rotation matrix transform
    m_transform = new osg::MatrixTransform( osg::Matrix::identity() );
    m_shift = new osg::MatrixTransform( osg::Matrix::translate( sx / 2, sy / 2, 0 ) );
    modelView->addChild( m_transform.get() );
    m_shift->addChild( modelView );
    this->addChild( m_shift );

    // Scene scaling transform
    m_scaleCenter = new osg::MatrixTransform( osg::Matrix::identity() );
    m_transform->addChild( m_scaleCenter.get() );

    // Add null widget to the box
    osgWidget::NullWidget * nullw = new osgWidget::NullWidget( "Null", sx, sy );
    nullw->setColor( 0.0f, 0.0f, 0.0f, 0.0f );
    this->addWidget( nullw );

    // Set alignment and background color
    this->getBackground()->setColor( ptrOptions->getBackgroundColor() );
    this->setAnchorHorizontal( osgWidget::Window::HA_LEFT );
    this->setAnchorVertical( osgWidget::Window::VA_BOTTOM );

    // Release options - are used in the create scene method
    ptrOptions.release();

    m_materialBodyRegular = new osg::CPseudoMaterial;
    m_materialBodyRegular->uniform("Shininess")->set(0.0f);
    m_materialBodyRegular->uniform("Specularity")->set(0.0f);
    m_materialBodyRegular->uniform("Diffuse")->set(osg::Vec3(0.5f, 0.5f, 0.5f));
    m_materialBodyRegular->uniform("Emission")->set(osg::Vec3(0.4f, 0.4f, 0.4f));

    // Add orientation marker scene centered and scaled
	createScene();
	m_scaleCenter->addChild( m_sceneGeometry );
	m_scaleCenter->addChild( m_sceneLabels );
    scaleScene( m_sceneGeometry );

    // Set update callback
    this->addUpdateCallback( new COrientationUpdateCallback() );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//

void scene::CSceneOrientationWidget::createScene(void)
{
#ifdef USE_BODY
    m_sceneGeometry = new osg::Geode;
#else
    m_sceneGeometry = new osg::Group;
#endif // USE_BODY
	m_sceneLabels = new osg::Group;

    // Get colors from the storage
    data::CObjectPtr< data::CSceneWidgetParameters > ptrCOptions( APP_STORAGE.getEntry(data::Storage::SceneWidgetsParameters::Id) );
    double scaleFactor = ptrCOptions->getWidgetsScale();

    // Axes of the coordinate system
#ifndef USE_BODY
    m_sceneGeometry->addChild( createArrow( osg::Vec3( 0.0, 0.0, 0.0 ), osg::Vec3( 1.0, 0.0, 0.0 ), ptrCOptions->getOrthoSliceColor( 1 ) ) );
    m_sceneGeometry->addChild( createArrow( osg::Vec3( 0.0, 0.0, 0.0 ), osg::Vec3( 0.0, 1.0, 0.0 ), ptrCOptions->getOrthoSliceColor( 2 ) ) );
    m_sceneGeometry->addChild( createArrow( osg::Vec3( 0.0, 0.0, 0.0 ), osg::Vec3( 0.0, 0.0, 1.0 ), ptrCOptions->getOrthoSliceColor( 0 ) ) );
#endif // USE_BODY

    // Release color options
    ptrCOptions.release();

    // Surface model
#ifndef USE_BODY
    m_model = new tModelVisualizer( data::Storage::PreviewModel::Id );
	m_model->dirtyBound();
    m_sceneGeometry->addChild( m_model );
#endif // USE_BODY

    // Get options from the storage
    data::CObjectPtr< data::CMeasurementOptions > ptrOptions( APP_STORAGE.getEntry(data::Storage::MeasurementOptions::Id) );
    osg::Vec4 textColor = ptrOptions->GetDropperFontColor();
    osg::Vec4 shadowColor = ptrOptions->GetDropperFontShadowColor();
    std::string fontName = ptrOptions->GetDropperFontName();
//    unsigned int fontSize = ptrOptions->GetDropperFontSize();
    
    unsigned int fontSize = 24 * scaleFactor;
    ptrOptions.release();

    // Assistant model
#ifdef USE_BODY
    vpl::base::CScopedPtr< geometry::CMesh > pMesh(new geometry::CMesh);
    OpenMesh::IO::Options ropt;
    ropt += OpenMesh::IO::Options::Binary;
    if( OpenMesh::IO::read_mesh(*pMesh, "models/assistant_body.stl", ropt) )
    {
        m_model = convertOpenMesh2OSGGeometry(pMesh, osg::Vec4(1.0, 1.0, 1.0, 1.0));
        m_sceneGeometry->addDrawable( m_model.get() );

        m_materialBodyRegular->apply(m_model);
    }
#endif // USE_BODY

    // Create labels
    if( m_Flags & SW_LEFT_LABEL )
    {
#ifndef USE_BODY
        m_sceneLabels->addChild( createLabel("L", osg::Vec3(1.0, 0.0, 0.0), fontName, fontSize, textColor, shadowColor) );
#else
        m_sceneLabels->addChild( createLabel("L", osg::Vec3(0.6, 0.0, 0.0), fontName, fontSize, textColor, shadowColor) );
        m_sceneLabels->addChild( createLabel("R", osg::Vec3(-0.8, 0.0, 0.0), fontName, fontSize, textColor, shadowColor) );
/*        if( OpenMesh::IO::read_mesh(*pMesh, "models/assistant_axis_x.stl", ropt) )
        {
            m_axisx = convertOpenMesh2OSGGeometry(pMesh, osg::Vec4(1.0, 0.0, 0.0, 1.0));
            m_sceneGeometry->addDrawable( m_axisx );
        }*/
#endif // USE_BODY
    }
    if( m_Flags & SW_POST_LABEL )
    {
#ifndef USE_BODY
        m_sceneLabels->addChild( createLabel("P", osg::Vec3(0.0, 1.0, 0.0), fontName, fontSize, textColor, shadowColor) );
#else
        m_sceneLabels->addChild( createLabel("P", osg::Vec3(0.0, 0.6, 0.0), fontName, fontSize, textColor, shadowColor) );
        m_sceneLabels->addChild( createLabel("A", osg::Vec3(0.0, -0.6, 0.0), fontName, fontSize, textColor, shadowColor) );
/*        if( OpenMesh::IO::read_mesh(*pMesh, "models/assistant_axis_y.stl", ropt) )
        {
            m_axisy = convertOpenMesh2OSGGeometry(pMesh, osg::Vec4(0.0, 1.0, 0.0, 1.0));
            m_sceneGeometry->addDrawable( m_axisy );
        }*/
#endif // USE_BODY
    }
    if( m_Flags & SW_SUP_LABEL )
    {
#ifndef USE_BODY
        m_sceneLabels->addChild( createLabel("S", osg::Vec3(0.0, 0.0, 1.0), fontName, fontSize, textColor, shadowColor) );
#else
        m_sceneLabels->addChild( createLabel("S", osg::Vec3(-0.1, 0.0, 0.7), fontName, fontSize, textColor, shadowColor) );
        m_sceneLabels->addChild( createLabel("I", osg::Vec3(-0.1, 0.0, -0.9), fontName, fontSize, textColor, shadowColor) );
/*        if( OpenMesh::IO::read_mesh(*pMesh, "models/assistant_axis_z.stl", ropt) )
        {
            m_axisz = convertOpenMesh2OSGGeometry(pMesh, osg::Vec4(0.0, 0.0, 1.0, 1.0));
            m_sceneGeometry->addDrawable( m_axisz );
        }*/
#endif // USE_BODY
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//

osg::MatrixTransform * scene::CSceneOrientationWidget::createLabel( const std::string & label,
                                                                    const osg::Vec3 & position,
                                                                    const std::string & fontname,
                                                                    unsigned int fontsize,
                                                                    const osg::Vec4 & fontcolor,
                                                                    const osg::Vec4 & shadowcolor
                                                                    )
{
    // Create text geode
    osg::Geode * textGeode = new osg::Geode;

    // Value text
    osgText::Text * text = new osgText::Text;
    osgText::Font * font = osgText::readFontFile( fontname.c_str() );
    text->setFont( font );
    text->setAutoRotateToScreen( true );
    text->setCharacterSize( fontsize );
    text->setFontResolution( 20, 20 );
    text->setCharacterSizeMode( osgText::Text::SCREEN_COORDS );
    text->setColor( fontcolor );
    text->setBackdropType( osgText::Text::OUTLINE );
    text->setBackdropColor( shadowcolor );
    text->setText( label );

    textGeode->addDrawable( text );

    // Positioning
    osg::MatrixTransform * mat = new osg::MatrixTransform( osg::Matrix::translate(position) );
    mat->addChild( textGeode );
    return mat;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//

osg::MatrixTransform * scene::CSceneOrientationWidget::createArrow( const osg::Vec3 &start,
                                                                    const osg::Vec3 &end,
                                                                    const osg::Vec4 &color
                                                                    )
{
    // Compute arrow parameters
    float length = (end - start).length();
    osg::Vec3 endPoint( length, 0.0, 0.0 );
    osg::Vec3 vec( end - start );
    vec.normalize();

    // Create geode 
    osg::Geode * geode = new osg::Geode;

    // Line
    {
        // Create geometry
        osg::Geometry * lineGeometry = new osg::Geometry;

        // Create vertices
        osg::Vec3Array * lineVertices = new osg::Vec3Array;
        lineGeometry->setVertexArray( lineVertices );
        lineVertices->push_back( osg::Vec3(0.0, 0.0, 0.0) );
        lineVertices->push_back( endPoint );

        // Draw arrays
        osg::DrawArrays * drawArrays = new osg::DrawArrays( osg::PrimitiveSet::LINES, 0, 2);
        lineGeometry->addPrimitiveSet( drawArrays );

        // Color array
        osg::Vec4Array * colors = new osg::Vec4Array;
        colors->push_back( color );

		lineGeometry->setColorArray(colors, osg::Array::BIND_OVERALL);

        // Add line to the geode
        geode->addDrawable( lineGeometry );
    }

    // Create matrix transform to rotate arrow to the position
    osg::MatrixTransform * mt = new osg::MatrixTransform( osg::Matrix::rotate( osg::Vec3( 1.0, 0.0, 0.0 ), vec ) );
    mt->addChild( geode );

    return mt;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//

void scene::CSceneOrientationWidget::scaleScene(osg::Node *scene)
{
	if( !scene )
    {
		return;
    }

	osg::BoundingSphere sphere( scene->getBound() );
	float scale( 0.9 * 0.5 * vpl::math::getMin( this->getWidth(), this->getHeight() ) / sphere.radius() );
	m_scaleCenter->setMatrix( osg::Matrix::translate( -sphere.center() ) * osg::Matrix::scale( scale, scale, scale ) );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//

void scene::CSceneOrientationWidget::updateFromStorage()
{
    // Get current rotation matrix - as in ruller widget we must use manipulator to get realy current position
    osg::Matrix m( m_viewer->getCameraManipulator()->getInverseMatrix() );
    osg::Matrix rotation( m.getRotate() );
    m_transform->setMatrix( rotation );

    // Refresh view - scene has changed
    m_pCanvas->Refresh();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//

void scene::CSceneOrientationWidget::COrientationUpdateCallback::operator()(osg::Node* node, osg::NodeVisitor* nv)
{
    CSceneOrientationWidget * overlay = dynamic_cast< CSceneOrientationWidget * >( node );
    if( overlay && (overlay->m_Flags & SW_MOVABLE) && overlay->m_viewer.get() )
    {
        // Get current rotation matrix - as in ruller widget we must use manipulator to get realy current position
        osg::Matrix m( overlay->m_viewer->getCameraManipulator()->getInverseMatrix() );
        osg::Matrix rotation( m.getRotate() );
        overlay->setMatrix( rotation );
    }

    traverse( node, nv );
}


////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//

scene::CRulerWidget::CRulerWidget( OSGCanvas * pCanvas, int sx, int sy, const osg::Vec4 & color /* = osg::Vec4( 1.0, 0.0, 0.0, 1.0 ) */ )
	: osgWidget::Box("RulerBox", osgWidget::Box::VERTICAL)
	, m_scenePadding( 8 )
	, m_rulerWidth( 15 )
	, m_scale( 0.0f )
	, m_distance( 0.0f )
    , m_usedColor( color )
{
    VPL_ASSERT( pCanvas );
	m_pCanvas = pCanvas;
    m_viewer = pCanvas->getView();

    // Get colors from the storage
    data::CObjectPtr< data::CSceneWidgetParameters > ptrOptions( APP_STORAGE.getEntry(data::Storage::SceneWidgetsParameters::Id) );
    double scaleFactor = ptrOptions->getWidgetsScale();
    m_rulerWidth *= scaleFactor;
 
	// Create scale label 
	m_labelScale = new osgWidget::Label();
	m_labelScale->setFont( DEFAULT_FONT_NAME );
	m_labelScale->setFontSize( 16.0 * scaleFactor );
	m_labelScale->setFontColor( m_usedColor );
	m_labelScale->setColor( osgWidget::Color( 0.0, 0.0, 0.0, 0.0 ) );
	m_labelScale->setLabel("[mm]");

	this->addWidget( m_labelScale.get() );

	// Get label height
	int labelHeight = m_labelScale->getHeightTotal();

	// Shift scene to the center of the null widget
	m_shift = new osg::MatrixTransform( osg::Matrix::translate( sx-m_rulerWidth, (sy + labelHeight + m_scenePadding)/2, 0 ) );

	// Scene scaling transform
	m_scaleCenter = new osg::MatrixTransform( osg::Matrix::identity() );
	
	m_nullWidget = new osgWidget::NullWidget( "RulerScene", m_rulerWidth, sy - labelHeight ); 
	m_nullWidget->setColor( osgWidget::Color( 0.0, 0.0, 0.0, 0.0 ) );
	m_nullWidget->setAlignHorizontal( osgWidget::Widget::HA_RIGHT );
	m_nullWidget->setAlignVertical( osgWidget::Widget::VA_TOP );
	this->addWidget( m_nullWidget.get() );

	// Move box center
	osgWidget::XYCoord coord;
	coord = this->getOrigin( );
	this->setPosition( -sx/2, -sy/2, 0 );

	//! Set alignment and background color
	this->getBackground()->setColor( ptrOptions->getBackgroundColor() );
	
	this->setWindowPosition( 100, 50 );
	this->setWindowOrigin( -100, -50 );
	this->setWindowSize( sx, 100 );

	this->setPositionType( scene::CPositionedWindow::PROPORTIONAL_WM, scene::CPositionedWindow::PROPORTIONAL_WM );
	this->setOriginType( scene::CPositionedWindow::PROPORTIONAL_WINDOW, scene::CPositionedWindow::PROPORTIONAL_WINDOW );
	this->setSizeType( scene::CPositionedWindow::FIXED, scene::CPositionedWindow::PROPORTIONAL_WM );

	// Construct scene
	m_scaleCenter->addChild( createRulerScene() );
	m_shift->addChild( m_scaleCenter );
	this->addChild( m_shift );

	// Set update callback
	this->setUpdateCallback( new CRulerUpdateCallback );

	// Connect to the invalidation signal
    scene::CGeneralObjectObserverOSG<CRulerWidget>::connect(APP_STORAGE.getEntry(data::Storage::SceneManipulatorDummy::Id).get());
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//

osg::Group * scene::CRulerWidget::createRulerScene()
{
	osg::Geode * geode = new osg::Geode;

	// Get colors from the storage
    data::CObjectPtr< data::CSceneWidgetParameters > ptrOptions( APP_STORAGE.getEntry(data::Storage::SceneWidgetsParameters::Id) );

	// Line
	{
		// Create geometry
		m_lineGeometry = new osg::Geometry;

		// Create vertices
		m_vertexArray = new osg::Vec3Array;
		m_vertexArray->setDataVariance( osg::Object::DYNAMIC );
		m_lineGeometry->setVertexArray( m_vertexArray );

		// Draw arrays
		m_drawArrays = new osg::DrawArrays( osg::PrimitiveSet::LINES, 0, 0);
		m_drawArrays->setDataVariance( osg::Object::DYNAMIC );
		m_lineGeometry->addPrimitiveSet( m_drawArrays.get() );

		// Color array
		osg::Vec4Array * colors = new osg::Vec4Array;
		colors->push_back( m_usedColor );

		m_lineGeometry->setColorArray(colors, osg::Array::BIND_OVERALL);

        m_lineMaterial = new osg::CMaterialLines(m_pCanvas->getView()->getCamera(), 1.25f);
        m_lineMaterial->apply(m_lineGeometry);

		// Add line to the geode
		geode->addDrawable( m_lineGeometry.get() );	
	}

	// Create group that holds all geometry
	osg::Group * group = new osg::Group;
	group->addChild( geode );

	return group;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//

void scene::CRulerWidget::CRulerUpdateCallback::operator()(osg::Node* node, osg::NodeVisitor* nv)
{
	CRulerWidget * widget = dynamic_cast< CRulerWidget * >( node );
	if( widget && widget->calcScaling() )
	{
		widget->onWMSizeChanged( widget->getWindowManager()->getWidth(), widget->getWindowManager()->getHeight() );
	}

    traverse(node, nv);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//

void scene::CRulerWidget::updateRulerGeometry( int windowWidth, int windowHeight )
{
	// Get label size
	int labelWidth( this->m_labelScale->getWidth() );
	int labelHeight( this->m_labelScale->getHeight() );

	// compute new ruler width
	float rulerWidth(vpl::math::getMin( (int)m_nullWidget->getWidth(), m_rulerWidth ));

	// Modify ruler scene shift transform
	m_shift->setMatrix( osg::Matrix::translate( windowWidth - rulerWidth - 1, (windowHeight + labelHeight + m_scenePadding)/2, 0 ) );

	// Update widget size
	m_nullWidget->setHeight( vpl::math::getMax(0, windowHeight - labelHeight) );
	m_nullWidget->setWidth( labelWidth );

	// Create geometry
	modifyLine( rulerWidth, m_nullWidget->getHeight() - m_scenePadding * 2, m_distance );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//

void scene::CRulerWidget::modifyLine(float w, float h, float marksDistance)
{
	// Clear old
	m_drawArrays->setCount( 0 );
	m_vertexArray->clear();

	
	// Insert first line - vertical
	m_vertexArray->push_back( osg::Vec3( w, -h/2, 0.0 ) );	
	m_vertexArray->push_back( osg::Vec3( w, h/2, 0.0 ) );

	// Create markers
	int mCount( h / marksDistance );
	float pos(h/2);

	int t( 0 );
	for( int i = 0; i < mCount; ++i )
	{
		float sub( t == 0 ? 0 : w/2 );
		m_vertexArray->push_back( osg::Vec3( w, pos, 0.0 ) );
		m_vertexArray->push_back( osg::Vec3( sub, pos, 0.0 ) );

		pos -= marksDistance;

		++t;
		if( t >= 10 ) t = 0;
	}

	// Modify primitive set
	m_drawArrays->setCount( 2 + mCount*2 );
	m_drawArrays->dirty();
	m_vertexArray->dirty();

	m_lineGeometry->dirtyGLObjects();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//

void scene::CRulerWidget::getDistances(float & dx, float & dy)
{
	// get viewport
	osg::Viewport* viewport = m_viewer->getCamera()->getViewport();

	// Compute transformation matrix
	osg::Matrixd ipm, pm( m_viewer->getCamera()->getProjectionMatrix() );
	ipm.invert(pm);
	
	osg::Matrixd iwm, wm( viewport->computeWindowMatrix() );
	iwm.invert(wm);

	/*
		This is really ugly and dirty hack. Update traversal is called BEFORE
		the viewer manipulator updates the camera view matrix. So if we use
		current view matrix, we are using one step backward matrix in fact. 
		This hack gets around this harsh reality.
	*/
	osg::Matrixd ivm, vm( m_viewer->getCameraManipulator()->getMatrix() );
	ivm.invert(vm);

	osg::Matrixd matrix = iwm*ipm*ivm;

	// Compute transformations of the unit vectors
	osg::Vec3 vdx( 1.0, 0.0, 0.5 );
	osg::Vec3 vdy( 0.0, 1.0, 0.5 );
	osg::Vec3 vo( 0.0, 0.0, 0.5 );

	vdx = vdx * matrix;
	vdy = vdy * matrix;
	vo = vo * matrix;

	vdx = vdx - vo;
	vdy = vdy - vo;

	dx = vdx.length();
	dy = vdy.length();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//

void scene::CRulerWidget::updateFromStorage()
{
	// Recompute scaling
	calcScaling( );

	// Redraw all
	updateLabel( );
	updateRulerGeometry( getWidth(), getHeight() );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//

void scene::CRulerWidget::updateWindowSizePosition(int width, int height, int x, int y)
{
	// Update ruler geometry
	updateRulerGeometry( width, height );

	// Set window position
	this->setPosition( x, y, 0 );

	// Resize box
	this->resize();

	// Resize all objects
	this->getWindowManager()->resizeAllWindows();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//

void scene::CRulerWidget::recomputeSizes(void)
{
	updateLabel();
	int labelWidth( this->m_labelScale->getWidth() );
	this->setWindowSize( labelWidth, 100 );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//

bool scene::CRulerWidget::calcScaling()
{
	// Viewer is not set
	if( m_viewer.get() == 0 )
		return false;

	float dx, dy;
	// Get pixel projected distance
	getDistances( dx, dy );

	float d10( 10.0 * dy ); // Ten pixels distance
	float l10( log10( d10 ) ); // Log of this distance
	int trunc( l10 + ( l10 > 0 ? 0.5 : -0.5 ) );	// Truncatenated log

	m_scale = pow( 10.0, trunc ); // Scale value
	float newdistance = m_scale / dy; // How many pixels is between scal distance - distance between ticks.

	m_distance = newdistance;
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//

void scene::CRulerWidget::updateLabel()
{
    // Set label
    std::stringstream ss;
    ss << m_scale << "mm";
    this->m_labelScale->setLabel(ss.str());

    // Modify label size
    osg::Vec2f xy = this->m_labelScale->getTextSize();
    this->m_labelScale->setSize( xy );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//

osgWidget::Label * scene::CSceneInfoWidget::createLabel( const std::string & text, const osg::Vec4 & color	)
{
   // correctly resize 
	data::CObjectPtr< data::CSceneWidgetParameters > ptrOptions( APP_STORAGE.getEntry(data::Storage::SceneWidgetsParameters::Id) );
	double scaleFactor = ptrOptions->getWidgetsScale();

	// create and init label
	osgWidget::Label * label = new osgWidget::Label();
    label->setFont( DEFAULT_FONT_NAME );
    label->setFontSize( 14 * scaleFactor );
    label->setFontColor( color );
    label->setColor( osgWidget::Color( 0.0, 0.0, 0.0, 0.0 ) );
    label->setLabel( text );
	label->setAlignHorizontal( osgWidget::Widget::HA_LEFT );

	return(label);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//

scene::CDescriptionWidget::CDescriptionWidget( OSGCanvas * pCanvas, EViewType type )
    : m_colorDefault( 0.0, 1.0, 0.0, 1.0 )
    , m_viewType( type )
    , m_padding_vertical( 0 )
    , m_padding_horizontal( 0 )
{
    // Get colors from the storage
    data::CObjectPtr< data::CSceneWidgetParameters > ptrOptions( APP_STORAGE.getEntry(data::Storage::SceneWidgetsParameters::Id) );

    // Create top label
    {
        m_labelT = new osgWidget::Label;
        if (SLICE_AXIAL == m_viewType)
            m_labelT->setLabel("F");
        else
            m_labelT->setLabel("T");

        // Modify label size
        osg::Vec2f xy = m_labelT->getTextSize();
        m_labelT->setSize( xy );

        // Set label parameters
        m_labelT->setColor( osg::Vec4(0.0, 0.0, 0.0, 0.0 ) );
        m_labelT->setFontColor( m_colorDefault );
        m_labelT->setFont( DEFAULT_FONT_NAME );
        m_labelT->setFontSize( 10.0 );

        this->addWidget( m_labelT, 0, 0 );
    }


    // Create horizontal label
    {
        m_labelH = new osgWidget::Label;
        if (SLICE_SAGITTAL == m_viewType)
            m_labelH->setLabel("F");
        else
            m_labelH->setLabel("R");

        // Modify label size
        osg::Vec2f xy = m_labelH->getTextSize();
        m_labelH->setSize( xy );

        // Set label parameters
        m_labelH->setColor( osg::Vec4(0.0, 0.0, 0.0, 0.0 ) );
        m_labelH->setFontColor( m_colorDefault );
        m_labelH->setFont( DEFAULT_FONT_NAME );
        m_labelH->setFontSize( 10.0 );

        this->addWidget( m_labelH, 0, 0 );
    }


    // Set window parameters
    this->setWindowPosition( 0, 0 );
    this->setWindowOrigin( 0, 0 );
    this->setWindowSize( 100, 100 );
    this->getBackground()->setColor( ptrOptions->getBackgroundColor() );

    this->setSizeType( scene::CPositionedWindow::PROPORTIONAL_WM, scene::CPositionedWindow::PROPORTIONAL_WM );
//            this->setPositionType(scene::CPositionedWindow::PROPORTIONAL_WM, scene::CPositionedWindow::PROPORTIONAL_WM);

}

void scene::CDescriptionWidget::setColor( const osg::Vec4 & color )
{
    m_labelH->setFontColor( color );
    m_labelT->setFontColor( color );
}


void scene::CDescriptionWidget::updateWindowSizePosition( int width, int height, int x, int y )
{
    // Modify labels size
    osg::Vec2f xy = m_labelH->getTextSize();
    m_labelH->setSize( xy );
    xy = m_labelT->getTextSize();
    m_labelT->setSize( xy );

    if (SLICE_SAGITTAL == m_viewType)
        m_labelH->setOrigin( width - m_labelH->getTextSize().x() - m_padding_horizontal, height/2 - m_labelH->getTextSize().y() );
    else
        m_labelH->setOrigin( m_labelH->getTextSize().x() + m_padding_horizontal, height/2 - m_labelH->getTextSize().y() );
    m_labelT->setOrigin( width/2 + m_labelT->getTextSize().x(), height - m_padding_vertical - m_labelT->getTextSize().y() / 2 );

    // Set window position
    this->setPosition( 0, 0, 0 );

    // Resize box
    this->resize();

    // Resize all objects
    this->getWindowManager()->resizeAllWindows();
}

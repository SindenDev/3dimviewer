///////////////////////////////////////////////////////////////////////////////
// $Id: CDraggableSlice.cpp 1939 2012-05-28 08:00:09Z tryhuk $
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

#include <base/Defs.h>
#include <osg/CDraggableSlice.h>
#include <osg/CTranslateOtherLineDragger.h>
#include <osg/OSGCanvas.h>
#include <osg/NodeMasks.h>
#include <osg/COnOffNode.h>
#include <osg/Version>

#include <osg/LineWidth>
#include <app/Signals.h>
#include <graph/osg/NodeMasks.h>

#define SCALE_FRACTION 0.5

//====================================================================================================================
scene::CFrameGeode::CFrameGeode() : m_FrameGeometry(new osg::Geometry)
{
	osg::Vec4Array * plane_color = new osg::Vec4Array;
//	plane_color->push_back( osg::Vec4( 1.0, 1.0, 1.0, 1.0 ) );
	plane_color->push_back( osg::Vec4( 0.0, 0.0, 0.0, 1.0 ) );

#if OSG_VERSION_GREATER_OR_EQUAL(3,1,10)
	m_FrameGeometry->setColorArray( plane_color, osg::Array::BIND_OVERALL );
#else
	m_FrameGeometry->setColorArray( plane_color );
#endif
	m_FrameGeometry->setColorBinding( osg::Geometry::BIND_OVERALL );
//	m_FrameGeometry.get()->setColorArray( plane_color );

	this->addDrawable( m_FrameGeometry.get() );

	// setup polygon offset
	osg::StateSet * state_set = new osg::StateSet();
	osg::PolygonOffset * offset = new osg::PolygonOffset( -2.0f, -2.0f );
	osg::LineWidth * line_width = new osg::LineWidth( 2.0f );

	state_set->setAttributeAndModes( offset, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );
	state_set->setAttributeAndModes( line_width, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );	

    m_FrameGeometry->setStateSet( state_set );

}

//====================================================================================================================
void scene::CFrameGeode::setColor( float r, float g, float b )
{
	osg::Vec4Array * color = dynamic_cast< osg::Vec4Array* >( m_FrameGeometry->getColorArray() );
	(*color)[0][0] = r;
	(*color)[0][1] = g;
	(*color)[0][2] = b;
	(*color)[0][3] = 1.0;
	m_FrameGeometry->dirtyDisplayList();
}


//====================================================================================================================
//====================================================================================================================
scene::CFrameXYGeode::CFrameXYGeode()
{
	osg::Vec3Array * plane_vertices = new osg::Vec3Array;
    plane_vertices->push_back( osg::Vec3( 0.0, 0.0, 0.0 ) );
    plane_vertices->push_back( osg::Vec3( 1.0, 0.0, 0.0 ) );
    plane_vertices->push_back( osg::Vec3( 1.0, 1.0, 0.0 ) );
    plane_vertices->push_back( osg::Vec3( 0.0, 1.0, 0.0 ) );
    plane_vertices->push_back( osg::Vec3( 0.0, 0.0, 0.001 ) );

	// set slice plane geometry
	m_FrameGeometry.get()->setVertexArray( plane_vertices );
	
	// create primitive set
	osg::DrawElementsUInt * plane_ps = new osg::DrawElementsUInt( osg::PrimitiveSet::LINES, 0 );

    plane_ps->push_back( 0 );
	plane_ps->push_back( 1 );
	plane_ps->push_back( 1 );
	plane_ps->push_back( 2 );
	plane_ps->push_back( 2 );
	plane_ps->push_back( 3 );
	plane_ps->push_back( 3 );
	plane_ps->push_back( 0 );
	plane_ps->push_back( 0 );
	plane_ps->push_back( 4 );

    m_FrameGeometry.get()->addPrimitiveSet( plane_ps );	
}


//====================================================================================================================
//====================================================================================================================
scene::CFrameXZGeode::CFrameXZGeode()
{
	osg::Vec3Array * plane_vertices = new osg::Vec3Array;

    plane_vertices->push_back( osg::Vec3( 0.0, 0.0, 0.0 ) );
    plane_vertices->push_back( osg::Vec3( 1.0, 0.0, 0.0 ) );
    plane_vertices->push_back( osg::Vec3( 1.0, 0.0, 1.0 ) );
    plane_vertices->push_back( osg::Vec3( 0.0, 0.0, 1.0 ) );
    plane_vertices->push_back( osg::Vec3( 0.0, 0.001, 0.0 ) );

	// set slice plane geometry
	m_FrameGeometry.get()->setVertexArray( plane_vertices );
	
	// create primitive set
	osg::DrawElementsUInt * plane_ps = new osg::DrawElementsUInt( osg::PrimitiveSet::LINES, 0 );

    plane_ps->push_back( 0 );
	plane_ps->push_back( 1 );
	plane_ps->push_back( 1 );
	plane_ps->push_back( 2 );
	plane_ps->push_back( 2 );
	plane_ps->push_back( 3 );
	plane_ps->push_back( 3 );
	plane_ps->push_back( 0 );
	plane_ps->push_back( 0 );
	plane_ps->push_back( 4 );

    m_FrameGeometry.get()->addPrimitiveSet( plane_ps );	
}

//====================================================================================================================
scene::CFrameYZGeode::CFrameYZGeode()
{
	osg::Vec3Array * plane_vertices = new osg::Vec3Array;

    plane_vertices->push_back( osg::Vec3( 0.0, 0.0, 0.0 ) );
    plane_vertices->push_back( osg::Vec3( 0.0, 1.0, 0.0 ) );
    plane_vertices->push_back( osg::Vec3( 0.0, 1.0, 1.0 ) );
    plane_vertices->push_back( osg::Vec3( 0.0, 0.0, 1.0 ) );
    plane_vertices->push_back( osg::Vec3( 0.001, 0.0, 0.0 ) );

	// set slice plane geometry
	m_FrameGeometry.get()->setVertexArray( plane_vertices );
	
	// create primitive set
	osg::DrawElementsUInt * plane_ps = new osg::DrawElementsUInt( osg::PrimitiveSet::LINES, 0 );

    plane_ps->push_back( 0 );
	plane_ps->push_back( 1 );
	plane_ps->push_back( 1 );
	plane_ps->push_back( 2 );
	plane_ps->push_back( 2 );
	plane_ps->push_back( 3 );
	plane_ps->push_back( 3 );
	plane_ps->push_back( 0 );
	plane_ps->push_back( 0 );
	plane_ps->push_back( 4 );

    m_FrameGeometry.get()->addPrimitiveSet( plane_ps );	
}


//====================================================================================================================
//====================================================================================================================
scene::CSliceGeode::CSliceGeode() : 
	m_SliceGeometry( new osg::Geometry() ),
	p_StateSet( new osg::StateSet )
{
}

void scene::CSliceGeode::setupScene()
{
	//osg::Vec4Array * plane_color = new osg::Vec4Array;
//	plane_color->push_back( osg::Vec4( 1.0, 1.0, 1.0, 1.0 ) );
	//plane_color->push_back( osg::Vec4( 0.0, 0.0, 0.0, 1.0 ) );

	//m_SliceGeometry.get()->setColorArray( plane_color );
	//m_SliceGeometry.get()->setColorBinding( osg::Geometry::BIND_OVERALL );
    m_SliceGeometry.get()->setDataVariance(osg::Object::DYNAMIC);

	osg::Vec2Array * plane_ta = new osg::Vec2Array;
	plane_ta->push_back( osg::Vec2( 0.0, 0.0 ) );
	plane_ta->push_back( osg::Vec2( 1.0, 0.0 ) );
	plane_ta->push_back( osg::Vec2( 1.0, 1.0 ) );
	plane_ta->push_back( osg::Vec2( 0.0, 1.0 ) );

	m_SliceGeometry.get()->setTexCoordArray( 0, plane_ta );
    m_SliceGeometry.get()->setStateSet( p_StateSet.get() );

    p_StateSet->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);
    p_StateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

	this->addDrawable( m_SliceGeometry.get() );

    // Set node mask to visible object (see NodeMasks.h)
    this->setNodeMask( MASK_VISIBLE_OBJECT );
}

//====================================================================================================================
void scene::CSliceGeode::setTextureAndCoordinates( osg::Texture2D * texture, float x_min, float x_max, float y_min, float y_max )
{
	osg::Vec2Array * coords = dynamic_cast<osg::Vec2Array *>(m_SliceGeometry.get()->getTexCoordArray(0));
    if( !coords )
    {
        return;
    }
    
    if( texture == p_StateSet->getTextureAttribute(0, osg::StateAttribute::TEXTURE)
        && (*coords)[2][0] == x_max
        && (*coords)[2][1] == y_max )
    {
        return;
    }

    osg::Vec4Array * plane_color = new osg::Vec4Array;
	plane_color->push_back( osg::Vec4(1.0, 1.0, 1.0, 1.0) );
#if OSG_VERSION_GREATER_OR_EQUAL(3,1,10)
	m_SliceGeometry.get()->setColorArray( plane_color, osg::Array::BIND_OVERALL );
#else
	m_SliceGeometry.get()->setColorArray( plane_color );
#endif
	m_SliceGeometry.get()->setColorBinding( osg::Geometry::BIND_OVERALL );

	(*coords)[0] = osg::Vec2(x_min, y_min);
	(*coords)[1] = osg::Vec2(x_max, y_min);
	(*coords)[2] = osg::Vec2(x_max, y_max);
	(*coords)[3] = osg::Vec2(x_min, y_max);
	m_SliceGeometry.get()->setTexCoordArray( 0, coords );
    m_SliceGeometry.get()->getTexCoordArray( 0 )->dirty();

	p_StateSet->setTextureAttributeAndModes( 0, texture, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );
	m_SliceGeometry->dirtyDisplayList();
}


//====================================================================================================================
//====================================================================================================================
scene::CSliceXYGeode::CSliceXYGeode()
{
	osg::Vec3Array * plane_vertices = new osg::Vec3Array;

    plane_vertices->push_back( osg::Vec3( 0.0, 0.0, 0.0 ) );
    plane_vertices->push_back( osg::Vec3( 1.0, 0.0, 0.0 ) );
    plane_vertices->push_back( osg::Vec3( 1.0, 1.0, 0.0 ) );
    plane_vertices->push_back( osg::Vec3( 0.0, 1.0, 0.0 ) );

	// set slice plane geometry
	m_SliceGeometry.get()->setVertexArray( plane_vertices );

	// create primitive set
	osg::DrawElementsUInt* plane_ps = new osg::DrawElementsUInt( osg::PrimitiveSet::QUADS, 0 );

	plane_ps->push_back( 0 );
	plane_ps->push_back( 1 );
	plane_ps->push_back( 2 );
	plane_ps->push_back( 3 );

    // assign primitive set to geometry
	m_SliceGeometry.get()->addPrimitiveSet( plane_ps );

    setupScene();
}


//====================================================================================================================
//====================================================================================================================
scene::CSliceXZGeode::CSliceXZGeode()
{
	osg::Vec3Array * plane_vertices = new osg::Vec3Array;

    plane_vertices->push_back( osg::Vec3( 0.0, 0.0, 0.0 ) );
    plane_vertices->push_back( osg::Vec3( 1.0, 0.0, 0.0 ) );
    plane_vertices->push_back( osg::Vec3( 1.0, 0.0, 1.0 ) );
    plane_vertices->push_back( osg::Vec3( 0.0, 0.0, 1.0 ) );

	// set slice plane geometry
	m_SliceGeometry.get()->setVertexArray( plane_vertices );

	// create primitive set
	osg::DrawElementsUInt* plane_ps = new osg::DrawElementsUInt( osg::PrimitiveSet::QUADS, 0 );

	plane_ps->push_back( 0 );
	plane_ps->push_back( 1 );
	plane_ps->push_back( 2 );
	plane_ps->push_back( 3 );

	// assign primitive set to geometry
	m_SliceGeometry.get()->addPrimitiveSet( plane_ps );	

    setupScene();
}


//====================================================================================================================
//====================================================================================================================
scene::CSliceYZGeode::CSliceYZGeode()
{
	osg::Vec3Array * plane_vertices = new osg::Vec3Array;

    plane_vertices->push_back( osg::Vec3( 0.0, 0.0, 0.0 ) );
    plane_vertices->push_back( osg::Vec3( 0.0, 1.0, 0.0 ) );
    plane_vertices->push_back( osg::Vec3( 0.0, 1.0, 1.0 ) );
    plane_vertices->push_back( osg::Vec3( 0.0, 0.0, 1.0 ) );

	// set slice plane geometry
	m_SliceGeometry.get()->setVertexArray( plane_vertices );

	// create primitive set
	osg::DrawElementsUInt* plane_ps = new osg::DrawElementsUInt( osg::PrimitiveSet::QUADS, 0 );

	plane_ps->push_back( 0 );
	plane_ps->push_back( 1 );
	plane_ps->push_back( 2 );
	plane_ps->push_back( 3 );

	// assign primitive set to geometry
	m_SliceGeometry.get()->addPrimitiveSet( plane_ps );

    setupScene();
}


//====================================================================================================================
//====================================================================================================================
scene::CDraggableSlice::CDraggableSlice(bool isOrtho) :
	p_Slice(),
	p_Dragger(),
	p_Dummy( new CDummyDraggableGeode ),
	p_Frame(),
	p_Constraint(),
	p_Selection(),
	p_Anchor( new osg::Group ),
    m_Ortho(isOrtho)
{
}

scene::CDraggableSlice::~CDraggableSlice()
{

}

//====================================================================================================================
void scene::CDraggableSlice::setupScene()
{
    // Set dragger mask
    p_Dragger->setNodeMask( MASK_DRAGGABLE_SLICE_DRAGGER );
    p_Dragger->setName ( "Slice Dragger" );

	this->addChild( p_Dragger.get() );
	this->addChild( p_Selection.get() );

	p_Dragger->addChild( p_Dummy.get() );
    if (p_DraggerHandle.get())
        p_Dragger->addChild( p_DraggerHandle.get() );
	p_Selection->addChild( p_Anchor.get() );

//    this->dummyVisible();
    this->dummyInvisible();

    p_Selection->addChild( p_Slice.get() );
    p_Selection->addChild( p_Frame.get() );
    p_Selection->setConstraintLink( p_Constraint.get() );

	p_Dragger->addDraggerCallback( p_Selection->getDraggerCommand() );
	p_Dragger->addConstraint( p_Constraint.get() );
//	p_Dragger->setHandleEvents(true);
}

//====================================================================================================================
void scene::CDraggableSlice::setTextureAndCoordinates( osg::Texture2D * texture, float x_min, float x_max, float y_min, float y_max )
{
	p_Slice->setTextureAndCoordinates( texture, x_min, x_max, y_min, y_max );
}

//====================================================================================================================
int scene::CDraggableSlice::getVoxelDepth()
{
	return p_Selection->getVoxelDepth();
}

//====================================================================================================================
float scene::CDraggableSlice::getVoxelSize()
{
	return p_Selection->getVoxelSize();
}

//====================================================================================================================
void scene::CDraggableSlice::setSignal(tSignal *pSignal)
{
    p_Selection->setSignal(pSignal);
}

//====================================================================================================================
void scene::CDraggableSlice::moveInDepth( int iPosition )
{
    if( iPosition == p_Selection->getIntPosition() )
    {
        return;
    }

	osg::Matrix	translation = p_Selection->getTranslationToPosition( iPosition );

	p_Selection->manualTranslation( translation );

	p_Dragger->setMatrix( p_Selection->getMatrix() );
}

//====================================================================================================================
void scene::CDraggableSlice::anchorToSlice( osg::Node * node )
{
	p_Anchor->addChild( node );
}

//====================================================================================================================
void scene::CDraggableSlice::scaleScene( float dx, float dy, float dz, int sx, int sy, int sz )
{
    data::CCoordinatesConv CoordConv(dx, dy, dz, sx, sy, sz);

    osg::Matrix m;
    m.makeScale( osg::Vec3d(CoordConv.getSceneMaxX(), CoordConv.getSceneMaxY(), CoordConv.getSceneMaxZ()) );
    m = m * osg::Matrix::translate( osg::Vec3d(CoordConv.getSceneShiftX(), CoordConv.getSceneShiftY(), CoordConv.getSceneShiftZ()) );

    this->setMatrix(m);

    if (p_DraggerHandle.get())
    {
        int sceneMin = CoordConv.getSceneMaxX();
        if (CoordConv.getSceneMaxY()<sceneMin)
            sceneMin=CoordConv.getSceneMaxY();
        if (CoordConv.getSceneMaxZ()<sceneMin)
            sceneMin=CoordConv.getSceneMaxZ();
        if (sceneMin>0)
        {
            osg::Matrix im;
            // undo effect of above scaling and rescale accordin to min. size
            im.makeScale( osg::Vec3d(sceneMin/CoordConv.getSceneMaxX(), sceneMin/CoordConv.getSceneMaxY(), sceneMin/CoordConv.getSceneMaxZ()) );
            // we know that the scale matrix of dragger handle is first child
            osg::ref_ptr<osg::MatrixTransform> mt=p_DraggerHandle->asGroup()->getChild(0)->asTransform()->asMatrixTransform();
            mt->setMatrix(im);
        }
    }
}

//====================================================================================================================
//====================================================================================================================

osg::Node* scene::CDraggableSlice::createDraggerHandle(osg::Vec3 center, osg::Vec3 color) const
{
    // create handle geometry
    osg::Geode *geode = new osg::Geode;
    osg::Box* box = new osg::Box(osg::Vec3(0,0,0),0.06);
    osg::ShapeDrawable* boxDrawable = new osg::ShapeDrawable(box);
    boxDrawable->setColor(osg::Vec4( color.x(), color.y(), color.z(), 1 ));
    geode->addDrawable(boxDrawable);
    // setup scale transform
    osg::MatrixTransform *mt = new osg::MatrixTransform;
    mt->setMatrix(osg::Matrix::identity());
    mt->addChild(geode);
    // create translate transform
    osg::MatrixTransform *mtt = new osg::MatrixTransform;
    mtt->setMatrix(osg::Matrix::translate(center));
    mtt->addChild(mt);
    return mtt;
}

/**
 * \brief    Accept node visitors - used to compute current view matrix.
 *
 * \param [in,out]  nv  The node visitor.
 */
void scene::CDraggableSlice::accept(osg::NodeVisitor& nv)
{
    if (nv.validNodeMask(*this))
    {
        // If this is cull visitor...
        if (nv.getVisitorType()==osg::NodeVisitor::CULL_VISITOR)
        {
            osgUtil::CullVisitor *cv(dynamic_cast<osgUtil::CullVisitor*>(&nv));
            if(cv != 0)
            {
                m_viewMatrix = cv->getCurrentCamera()->getViewMatrix()*_matrix;

                // Compute scale factor given by view matrix (we want to scale independently on zoom)
                osg::Vec3 scale(m_viewMatrix.getScale());

                m_scaleFactor = SCALE_FRACTION*std::min(scale[0], scale[1]);

                // Set scale factor to the dragger if possible
                if(p_Dragger != 0)
                {
                    osgManipulator::CTranslateOtherLineDragger * dragger(dynamic_cast<osgManipulator::CTranslateOtherLineDragger*>(p_Dragger.get()));
                    if(dragger != 0)
                        dragger->setScaleFactor(m_scaleFactor);
                }
            }
            osg::CullStack* cs = dynamic_cast<osg::CullStack*>(&nv);
            if(cs)
            {
                
//                 m_viewMatrix = *cs->getModelViewMatrix();
// 
//                 // Compute scale factor given by view matrix (we want to scale independently on zoom)
//                 osg::Vec3 scale(m_viewMatrix.getScale());
// 
//                 m_scaleFactor = std::min(scale[0], scale[1]);
// 
//                 // Set scale factor to the dragger if possible
//                 if(p_Dragger != 0)
//                 {
//                     osgManipulator::CTranslateOtherLineDragger * dragger(dynamic_cast<osgManipulator::CTranslateOtherLineDragger*>(p_Dragger.get()));
//                     if(dragger != 0)
//                         dragger->setScaleFactor(m_scaleFactor);
//                 }
            }
        }
    }

    osg::MatrixTransform::accept(nv);
}

//====================================================================================================================
//====================================================================================================================
scene::CDraggableSliceXY::CDraggableSliceXY( OSGCanvas * pCanvas, bool isOrtho, int SliceId, bool draggerHandle )
    : m_SliceId(SliceId)
{
    setCanvas(pCanvas);

	p_Slice = new CSliceXYGeode();

    p_Frame = new CFrameXYGeode();
    p_Frame->setColor( 0.0, 0.0, 1.0 );

    p_Selection = new scene::CPlaneXYUpdateSelection();
    
    // set up dummy draggable geometry
    p_Dummy->setUpSquarePlaneXY(false);
    p_Dummy->setColor( 0.0, 0.0, 1.0 );

    if (draggerHandle)
        p_DraggerHandle = createDraggerHandle(osg::Vec3(0.5,0.5,0),osg::Vec3(1.0, 1.0, 0.0));

	if( isOrtho )
	{
		p_Dragger = new	osgManipulator::CTranslateOtherLineDragger( osg::Vec3(0.0, 0.0, 0.0), osg::Vec3(0.0, 0.0, 1.0), osg::Vec3(0.0, 0.0, 0.0), osg::Vec3(0.0, -1.0, 0.0));
	    p_Dragger->setDraggerActive( true );
    }
	else	
	{
        //p_Dragger = new osgManipulator::Translate1DDragger( osg::Vec3( 0.0, 0.0, 0.0 ), osg::Vec3(0.0, 0.0, 1.0 ) );
        p_Dragger = new osgManipulator::CSliceDragger( osg::Vec3( 0.0, 0.0, 0.0 ), osg::Vec3(0.0, 0.0, 1.0 ) );
	}
 
    p_Constraint = new manipul::CSliceXYConstraint( *p_Slice.get() );

	// Set slice type
	m_Plane = data::COrthoSlice::PLANE_XY;

    // initialize the tree
    setupScene();

    // associate dummy geometry with slice according to its index
    p_Dummy->setID( static_cast<unsigned>(data::COrthoSlice::PLANE_XY) );

    // Set the update callback
    if( m_SliceId != data::Storage::UNKNOWN )
    {
        scene::CGeneralObjectObserverOSG<CDraggableSliceXY>::connect(APP_STORAGE.getEntry(m_SliceId).get(), tObserverHandler(this, &CDraggableSliceXY::objectChanged));
        this->setupObserver(this);
    }
}

scene::CDraggableSliceXY::~CDraggableSliceXY()
{
    if( m_SliceId != data::Storage::UNKNOWN )
    {
        this->freeObserver(this);
        scene::CGeneralObjectObserverOSG<CDraggableSliceXY>::disconnect(APP_STORAGE.getEntry(m_SliceId).get());
    }
}


//====================================================================================================================
void scene::CDraggableSliceXY::scaleScene(float dx, float dy, float dz, int sx, int sy, int sz)
{
	p_Selection->setVoxelDepth(sz, dz);
    
    scene::CDraggableSlice::scaleScene(dx, dy, dz, sx, sy, sz);
}

//====================================================================================================================
void scene::CDraggableSliceXY::updateFromStorage()
{
    data::CObjectPtr<data::COrthoSliceXY> spSlice( APP_STORAGE.getEntry(data::Storage::SliceXY::Id) );

    moveInDepth( spSlice->getPosition() );
    if( !m_Ortho )
    {
	    setTextureAndCoordinates( spSlice->getTexturePtr(), 0.0, spSlice->getTextureWidth(), 0.0, spSlice->getTextureHeight() );
    }
}

//====================================================================================================================
void scene::CDraggableSliceXY::objectChanged(data::CStorageEntry *pEntry, const data::CChangedEntries &changes)
{
    // Invalidate OpenGL canvas
    if( m_pCanvas)
    {
        // if parent is switch, check its state
        const osg::Group* parent = (1==getNumParents())? this->getParent(0) : NULL;
        if (NULL!=parent)
        {
            const osg::Switch* pSwitch=parent->asSwitch();
            if (pSwitch)
            {
                const osg::COnOffNode* pOnOffNode=dynamic_cast<const osg::COnOffNode*>(pSwitch);
                if (NULL!=pOnOffNode)
                {
                    if (pOnOffNode->isVisible())
                    {
                        CGeneralObjectObserverOSG<scene::CDraggableSliceXY>::objectChanged(pEntry, changes);
                    }
                    return;
                }
            }
        }
        // if it is not a switch, refresh always
        CGeneralObjectObserverOSG<scene::CDraggableSliceXY>::objectChanged(pEntry, changes);
    }                            
}

//====================================================================================================================
//====================================================================================================================
scene::CDraggableSliceXZ::CDraggableSliceXZ( OSGCanvas * pCanvas, bool isOrtho, int SliceId, bool draggerHandle )
    : m_SliceId(SliceId)
{
    setCanvas(pCanvas);

	p_Slice = new CSliceXZGeode();

    p_Frame = new CFrameXZGeode();
    p_Frame->setColor( 0.0, 1.0, 0.0 );

    p_Selection = new scene::CPlaneXZUpdateSelection();
    
    // set up dummy draggable geometry
    p_Dummy->setUpSquarePlaneXZ(false);
    p_Dummy->setColor( 0.0, 1.0, 0.0 );

    if (draggerHandle)
        p_DraggerHandle = createDraggerHandle(osg::Vec3(0.5,0,0.5),osg::Vec3(1.0, 1.0, 0.0));

	if( isOrtho )
	{
		p_Dragger = new	osgManipulator::CTranslateOtherLineDragger( osg::Vec3(0.0, 0.0, 0.0), osg::Vec3(0.0, 1.0, 0.0), osg::Vec3(0.0, 0.0, 0.0), osg::Vec3(0.0, 0.0, 1.0));
        p_Dragger->setDraggerActive( true );
	}
	else
	{
		//p_Dragger = new osgManipulator::Translate1DDragger( osg::Vec3( 0.0, 0.0, 0.0 ), osg::Vec3(0.0, 1.0, 0.0 ) );
		p_Dragger = new osgManipulator::CSliceDragger( osg::Vec3( 0.0, 0.0, 0.0 ), osg::Vec3(0.0, 1.0, 0.0 ) );
	}

    p_Constraint = new manipul::CSliceXZConstraint( *p_Slice.get() );

	// Set slice type
	m_Plane = data::COrthoSlice::PLANE_XZ;

    // initialize the tree
    setupScene();

	// associate dummy geometry with slice according to its index
    p_Dummy->setID( static_cast<unsigned>(data::COrthoSlice::PLANE_XZ) );

    // Set the update callback
    if( m_SliceId != data::Storage::UNKNOWN )
    {
        scene::CGeneralObjectObserverOSG<CDraggableSliceXZ>::connect(APP_STORAGE.getEntry(m_SliceId).get(), tObserverHandler(this, &CDraggableSliceXZ::objectChanged));
        this->setupObserver(this);
    }
}

scene::CDraggableSliceXZ::~CDraggableSliceXZ()
{
    if( m_SliceId != data::Storage::UNKNOWN )
    {
        this->freeObserver(this);
        scene::CGeneralObjectObserverOSG<CDraggableSliceXZ>::disconnect(APP_STORAGE.getEntry(m_SliceId).get());
    }
}

//====================================================================================================================
void scene::CDraggableSliceXZ::scaleScene(float dx, float dy, float dz, int sx, int sy, int sz)
{
	p_Selection->setVoxelDepth(sy, dy);

    scene::CDraggableSlice::scaleScene(dx, dy, dz, sx, sy, sz);
}

//====================================================================================================================
void scene::CDraggableSliceXZ::updateFromStorage()
{
    data::CObjectPtr<data::COrthoSliceXZ> spSlice( APP_STORAGE.getEntry(m_SliceId) );

    moveInDepth( spSlice->getPosition() );
    if( !m_Ortho )
    {
	    setTextureAndCoordinates( spSlice->getTexturePtr(), 0.0, spSlice->getTextureWidth(), 0.0, spSlice->getTextureHeight() );
    }
}

//====================================================================================================================
void scene::CDraggableSliceXZ::objectChanged(data::CStorageEntry *pEntry, const data::CChangedEntries &changes)
{
    // Invalidate OpenGL canvas
    if( m_pCanvas)
    {
        // if parent is switch, check its state
        const osg::Group* parent = (1==getNumParents())? this->getParent(0) : NULL;
        if (NULL!=parent)
        {
            const osg::Switch* pSwitch=parent->asSwitch();
            if (pSwitch)
            {
                const osg::COnOffNode* pOnOffNode=dynamic_cast<const osg::COnOffNode*>(pSwitch);
                if (NULL!=pOnOffNode)
                {
                    if (pOnOffNode->isVisible())
                    {
                        CGeneralObjectObserverOSG<scene::CDraggableSliceXZ>::objectChanged(pEntry, changes);
                    }
                    return;
                }
            }
        }
        // if it is not a switch, refresh always
        CGeneralObjectObserverOSG<scene::CDraggableSliceXZ>::objectChanged(pEntry, changes);
    }
}

//====================================================================================================================
//====================================================================================================================
scene::CDraggableSliceYZ::CDraggableSliceYZ( OSGCanvas * pCanvas, bool isOrtho, int SliceId, bool draggerHandle )
    : m_SliceId(SliceId)
{
    setCanvas(pCanvas);

	p_Slice = new CSliceYZGeode();

    p_Frame = new CFrameYZGeode();
    p_Frame->setColor( 1.0, 0.0, 0.0 );

    p_Selection = new scene::CPlaneYZUpdateSelection();
    
    // set up dummy draggable geometry
    p_Dummy->setUpSquarePlaneYZ(false);
    p_Dummy->setColor( 1.0, 0.0, 0.0 );

    if (draggerHandle)
        p_DraggerHandle = createDraggerHandle(osg::Vec3(0,0.5,0.5),osg::Vec3( 1.0, 1.0, 0.0));

	if( isOrtho )
	{
		p_Dragger = new	osgManipulator::CTranslateOtherLineDragger( osg::Vec3(0.0, 0.0, 0.0), osg::Vec3(1.0, 0.0, 0.0), osg::Vec3(0.0, 0.0, 0.0), osg::Vec3(0.0, 0.0, 1.0));
        p_Dragger->setDraggerActive( true );
	}
	else
	{
		//p_Dragger = new osgManipulator::Translate1DDragger( osg::Vec3( 0.0, 0.0, 0.0 ), osg::Vec3(1.0, 0.0, 0.0 ) );
		p_Dragger = new osgManipulator::CSliceDragger( osg::Vec3( 0.0, 0.0, 0.0 ), osg::Vec3(1.0, 0.0, 0.0 ) );
	}

    p_Constraint = new manipul::CSliceYZConstraint( *p_Slice.get() );

	// Set slice type
	m_Plane = data::COrthoSlice::PLANE_YZ;

    // initialize the tree
    setupScene();

	// associate dummy geometry with slice according to its index
    p_Dummy->setID( static_cast<unsigned>(data::COrthoSlice::PLANE_YZ) );	

    // Set the update callback
    if( m_SliceId != data::Storage::UNKNOWN )
    {
        scene::CGeneralObjectObserverOSG<CDraggableSliceYZ>::connect(APP_STORAGE.getEntry(m_SliceId).get(), tObserverHandler(this, &CDraggableSliceYZ::objectChanged));
        this->setupObserver(this);
    }
}

scene::CDraggableSliceYZ::~CDraggableSliceYZ()
{
    if( m_SliceId != data::Storage::UNKNOWN )
    {
        this->freeObserver(this);
        scene::CGeneralObjectObserverOSG<CDraggableSliceYZ>::disconnect(APP_STORAGE.getEntry(m_SliceId).get());
    }
}

//====================================================================================================================
void scene::CDraggableSliceYZ::scaleScene(float dx, float dy, float dz, int sx, int sy, int sz)
{
	p_Selection->setVoxelDepth(sx, dx);

    scene::CDraggableSlice::scaleScene(dx, dy, dz, sx, sy, sz);
}

//====================================================================================================================
void scene::CDraggableSliceYZ::updateFromStorage()
{
    data::CObjectPtr<data::COrthoSliceYZ> spSlice( APP_STORAGE.getEntry(m_SliceId) );

    moveInDepth( spSlice->getPosition() );
    if( !m_Ortho )
    {
	    setTextureAndCoordinates( spSlice->getTexturePtr(), 0.0, spSlice->getTextureWidth(), 0.0, spSlice->getTextureHeight() );
    }
}

//====================================================================================================================
void scene::CDraggableSliceYZ::objectChanged(data::CStorageEntry *pEntry, const data::CChangedEntries &changes)
{
    // Invalidate OpenGL canvas
    if( m_pCanvas)
    {
        // if parent is switch, check its state
        const osg::Group* parent = (1==getNumParents())? this->getParent(0) : NULL;
        if (NULL!=parent)
        {
            const osg::Switch* pSwitch=parent->asSwitch();
            if (pSwitch)
            {
                const osg::COnOffNode* pOnOffNode=dynamic_cast<const osg::COnOffNode*>(pSwitch);
                if (NULL!=pOnOffNode)
                {
                    if (pOnOffNode->isVisible())
                    {
                        CGeneralObjectObserverOSG<scene::CDraggableSliceYZ>::objectChanged(pEntry, changes);
                    }
                    return;
                }
            }
        }
        // if it is not a switch, refresh always
        CGeneralObjectObserverOSG<scene::CDraggableSliceYZ>::objectChanged(pEntry, changes);
    }
}

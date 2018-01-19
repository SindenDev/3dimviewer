///////////////////////////////////////////////////////////////////////////////
// $Id: Widgets.h 1865 2012-05-14 10:01:32Z tryhuk $
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

#ifndef Widgets_H_included
#define Widgets_H_included

#include <osg/StateSet>
#include <osgText/Text>
#include <osgWidget/Util>
#include <osgWidget/Box>
#include <osgWidget/Label>
#include <osgGA/GUIEventHandler>

#include <osg/CGeneralObjectObserverOSG.h>
#include <osg/CModelVisualizer.h>

#include <data/CActiveDataSet.h>
#include <data/CSceneWidgetParameters.h>
#include <data/CDensityData.h>
#include <data/CPreviewModel.h>

#include <widgets/CPositionedWindow.h>

namespace scene
{

////////////////////////////////////////////////////////////////////////////////////////////////////
// Global functions and definitions

//! Default text font
#define DEFAULT_FONT_NAME "fonts/Vera.ttf"

//! Basic flags describing scene widgets.
enum ESceneWidgetsFlags
{
    //! Creates an info widget.
    SW_INFO         = 1 << 0,

    //! Creates an orientation widget.
    SW_ORIENT       = 1 << 1,

    //! Creates a ruler widget.
    SW_RULER        = 1 << 2,

    //! Creates a letters (T,F,R) widget.
    SW_DESCRIPTION  = 1 << 3,

    //! All basic widgets.
    SW_ALL_WIDGETS  = SW_INFO | SW_ORIENT | SW_RULER
};

//! Creates a simple label widget.
osgWidget::Label* createLabelWidget(const std::string& label, 
                                    unsigned int font_size = 13, 
                                    const osgWidget::Color & fontColor = osgWidget::Color( 1.0, 1.0, 1.0, 1.0 ), 
                                    const osgWidget::Color & backgroundColor = osgWidget::Color( 0.0, 0.0, 0.0, 0.0 )
                                    );

//! Creates an image widget.
osgWidget::Widget* createImageWidget(const std::string & filename);


////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Scene information widget. 

class CSceneInfoWidget
    : public osgWidget::Box
    , public scene::CGeneralObjectObserverOSG<CSceneInfoWidget>
{
public:
    //! Constructor
    CSceneInfoWidget(OSGCanvas * pCanvas);

    //! Update from storage
    virtual void updateFromStorage();

protected:
	//! Create label and set parameters
	osgWidget::Label * createLabel( const std::string & label, const osg::Vec4 & color );

protected:
	//! Patient name
	osg::ref_ptr< osgWidget::Label > m_labelPatientName;

	// Other lines
	osg::ref_ptr< osgWidget::Label > m_labelPatientId;
	osg::ref_ptr< osgWidget::Label > m_labelDateTime;
};


////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Scene orientation widget. 

class CSceneOrientationWidget
    : public osgWidget::Box
    , public scene::CGeneralObjectObserverOSG<CSceneOrientationWidget>
{
public:
    //! Additional flags modifying the widget behaviour.
    enum EFlags
    {
        //! Adds L-left label close to the end of x-axis.
        SW_LEFT_LABEL   = 1 << 4,

        //! Adds P-posterior label close to the end of y-axis.
        SW_POST_LABEL   = 1 << 5,

        //! Adds S-superior label close to the end of z-axis.
        SW_SUP_LABEL    = 1 << 6,

        //! Adds all axis labels to the scene.
        SW_ALL_LABELS   = SW_LEFT_LABEL | SW_POST_LABEL | SW_SUP_LABEL,

        //! Allows automatic view changes according to orientation of the 3D scene.
        SW_MOVABLE      = 1 << 7
    };

public:
    //! Constructor
    CSceneOrientationWidget(OSGCanvas * pCanvas,
                            int sx, int sy,
                            const osg::Matrix & orientation,
                            int Flags = SW_ALL_LABELS | SW_MOVABLE
                            );

    //! Set matrix
    void setMatrix(const osg::Matrix & matrix) { m_transform->setMatrix( matrix ); }

    //! Changes the internal flags.
    void setFlags(int Flags) { m_Flags = Flags; }

    //! Update from storage.
    virtual void updateFromStorage();

protected:
    //! Creates simple orientation scene.
    void createScene();

    //! Creates a subgraph for an arrow.
    osg::MatrixTransform * createArrow(const osg::Vec3 & start,
                                       const osg::Vec3 & end,
                                       const osg::Vec4 & color
                                       );

    //! Creates a subgraph for a simple text label.
    osg::MatrixTransform * createLabel(const std::string & label,
                                       const osg::Vec3 & position,
                                       const std::string & fontname,
                                       unsigned int fontsize,
                                       const osg::Vec4 & fontcolor,
                                       const osg::Vec4 & shadowcolor
                                       );

    //! Scale scene
    void scaleScene(osg::Node * scene);

protected:
    //! Scene orientation matrix transform
    osg::ref_ptr< osg::MatrixTransform > m_transform;

    //! Move scene to the center of the window
    osg::ref_ptr< osg::MatrixTransform > m_shift;

    //! Scaling and centering matrix transform
    osg::ref_ptr< osg::MatrixTransform > m_scaleCenter;

    //! Model shown in the scene
#ifndef USE_BODY
    typedef osg::CAnyModelVisualizer<data::CPreviewModel> tModelVisualizer;
    osg::ref_ptr< tModelVisualizer > m_model;
#else
    osg::ref_ptr< osg::Geometry > m_model;
//    osg::ref_ptr< osg::Geometry > m_axisx;
//    osg::ref_ptr< osg::Geometry > m_axisy;
//    osg::ref_ptr< osg::Geometry > m_axisz;
#endif // USE_BODY

    //! Scene flags.
    int m_Flags;

    //! Viewer
    osg::ref_ptr< osgViewer::Viewer > m_viewer;

	//! Scene geometry
#ifndef USE_BODY
	osg::ref_ptr< osg::Group > m_sceneGeometry;
#else
	osg::ref_ptr< osg::Geode > m_sceneGeometry;
#endif // USE_BODY

	//! Scene labels
	osg::ref_ptr< osg::Group > m_sceneLabels;

protected:
    //! Update orientation callback.
    struct COrientationUpdateCallback : public osg::NodeCallback
    {
	    virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);
    };
};


////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Ruler widget. 

class CRulerWidget : public osgWidget::Box, public scene::CPositionedWindow, public scene::CGeneralObjectObserverOSG<CRulerWidget>
{
public:
    //! Constructor
    CRulerWidget( OSGCanvas *pCanvas, int sx, int sy, const osg::Vec4 & color = osg::Vec4( 1.0, 0.0, 0.0, 1.0 ) );

protected:
    //! Create ruler scene
    osg::Group * createRulerScene( );

    //! Recompute scaling
    bool calcScaling( );

    //! Update ruler geometry
    void updateRulerGeometry( int windowWidth, int windowHeight );

    //! Update label
    void updateLabel( );

    //! Modify ruler line
    void modifyLine( float w, float h, float marksDistance );

    //! Compute pixel distances
    void getDistances( float & dx, float & dy);

    //! Update window parameters implementation
    virtual void updateWindowSizePosition( int width, int height, int x, int y );

    //! Set new window origin based on label size...
    virtual void recomputeSizes();

    virtual void updateFromStorage();

protected:
    //! Scale label
    osg::ref_ptr< osgWidget::Label > m_labelScale;

    //! Null label
    osg::ref_ptr< osgWidget::NullWidget > m_nullWidget;

    //! Move scene to the top right of the window
    osg::ref_ptr< osg::MatrixTransform > m_shift;

    //! Scaling and centering matrix transform
    osg::ref_ptr< osg::MatrixTransform > m_scaleCenter;

    //! Viewer
    osg::ref_ptr< osgViewer::Viewer > m_viewer;

    //! Line draw arrays
    osg::ref_ptr< osg::DrawArrays > m_drawArrays;

    //! Line vertex array
    osg::ref_ptr< osg::Vec3Array > m_vertexArray;

    //! Ruler geometry
    osg::ref_ptr< osg::Geometry > m_lineGeometry;

    //! Canvas
    OSGCanvas * m_pCanvas;

    //! Scene padding
    int m_scenePadding;

    //! Default ruler width
    int m_rulerWidth;

    //! Data has changed signal connection
    vpl::mod::tSignalConnection m_conDataChanged;

    //! Current scaling
    float m_scale;

    //! Current marks distance
    float m_distance;

    //! Used color
    osg::Vec4 m_usedColor;

protected:
    //! Update callback.
    struct CRulerUpdateCallback : public osg::NodeCallback
    {
        virtual void operator() (osg::Node* node, osg::NodeVisitor* nv);
    };
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Letter description widget.

class CDescriptionWidget : public osgWidget::Canvas, public scene::CPositionedWindow
{
public:
    enum EViewType
    {
        NONE = 0,
        SLICE_AXIAL = 1,
        SLICE_CORONAL = 2,
        SLICE_SAGITTAL = 4
    };

    /// Constructor
    CDescriptionWidget( OSGCanvas * pCanvas, EViewType type );

    /// Set color of all elements
    void setColor( const osg::Vec4 & color );

    // Set horizontal text padding
    void setHorizontalPadding( int padding ){ m_padding_horizontal = padding; }

    // Set vertical text padding
    void setVerticalPadding( int padding ){ m_padding_vertical = padding; }

protected:
    //! Update window parameters implementation
    virtual void updateWindowSizePosition( int width, int height, int x, int y );

protected:
    /// Default color
    osg::Vec4 m_colorDefault;

    /// View type
    EViewType m_viewType;

    //! Labels
    osg::ref_ptr< osgWidget::Label > m_labelT, m_labelH;

    //! Padding of label
    int m_padding_vertical;

    int m_padding_horizontal;

}; // class CDescriptionWidget


} // namespace scene

#endif // Widgets_H_included

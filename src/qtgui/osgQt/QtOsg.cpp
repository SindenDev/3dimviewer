/* NOTE from http://developer.qt.nokia.com/doc/qt-4.8/qglwidget.html
Note that under Windows, the QGLContext belonging to a QGLWidget has to be recreated
when the QGLWidget is reparented. This is necessary due to limitations on the Windows
platform. This will most likely cause problems for users that have subclassed and
installed their own QGLContext on a QGLWidget. It is possible to work around this
issue by putting the QGLWidget inside a dummy widget and then reparenting the dummy
widget, instead of the QGLWidget. This will side-step the issue altogether, and is
what we recommend for users that need this kind of functionality.

On Mac OS X, when Qt is built with Cocoa support, a QGLWidget can't have any sibling
widgets placed ontop of itself. This is due to limitations in the Cocoa API and is not
supported by Apple.
*/
#ifdef __APPLE__
    #include <glew.h>
#else
    #include <GL/glew.h>
#endif

#include "render/cvolumerendererwindow.h"

//#include <VPL/Base/Logging.h>

#include <base/Macros.h>

#include <osg/CSceneManipulator.h>
#include <osg/CSceneOSG.h>
#include <osg/CScreenshot.h>
#include <osg/Version>

#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>
#include <osgDB/ReadFile>
#include <osgViewer/Renderer>

#include <QMouseEvent>
#include <QApplication>
#include <QThread>
#include <QDebug>

#include <osgQt/QtOsg.h>

///////////////////////////////////////////////////////////////////////////////
// mapping of special keys

class QtKeyboardMap
{

public:
    QtKeyboardMap()
    {
        mKeyMap[Qt::Key_Escape     ] = osgGA::GUIEventAdapter::KEY_Escape;
        mKeyMap[Qt::Key_Delete   ] = osgGA::GUIEventAdapter::KEY_Delete;
        mKeyMap[Qt::Key_Home       ] = osgGA::GUIEventAdapter::KEY_Home;
        mKeyMap[Qt::Key_Enter      ] = osgGA::GUIEventAdapter::KEY_KP_Enter;
        mKeyMap[Qt::Key_End        ] = osgGA::GUIEventAdapter::KEY_End;
        mKeyMap[Qt::Key_Return     ] = osgGA::GUIEventAdapter::KEY_Return;
        mKeyMap[Qt::Key_PageUp     ] = osgGA::GUIEventAdapter::KEY_Page_Up;
        mKeyMap[Qt::Key_PageDown   ] = osgGA::GUIEventAdapter::KEY_Page_Down;
        mKeyMap[Qt::Key_Left       ] = osgGA::GUIEventAdapter::KEY_Left;
        mKeyMap[Qt::Key_Right      ] = osgGA::GUIEventAdapter::KEY_Right;
        mKeyMap[Qt::Key_Up         ] = osgGA::GUIEventAdapter::KEY_Up;
        mKeyMap[Qt::Key_Down       ] = osgGA::GUIEventAdapter::KEY_Down;
        mKeyMap[Qt::Key_Backspace  ] = osgGA::GUIEventAdapter::KEY_BackSpace;
        mKeyMap[Qt::Key_Tab        ] = osgGA::GUIEventAdapter::KEY_Tab;
        mKeyMap[Qt::Key_Space      ] = osgGA::GUIEventAdapter::KEY_Space;
        mKeyMap[Qt::Key_Delete     ] = osgGA::GUIEventAdapter::KEY_Delete;
        mKeyMap[Qt::Key_Alt      ] = osgGA::GUIEventAdapter::KEY_Alt_L;
        mKeyMap[Qt::Key_Shift    ] = osgGA::GUIEventAdapter::KEY_Shift_L;
        mKeyMap[Qt::Key_Control  ] = osgGA::GUIEventAdapter::KEY_Control_L;
        mKeyMap[Qt::Key_Meta     ] = osgGA::GUIEventAdapter::KEY_Meta_L;

        mKeyMap[Qt::Key_F1             ] = osgGA::GUIEventAdapter::KEY_F1;
        mKeyMap[Qt::Key_F2             ] = osgGA::GUIEventAdapter::KEY_F2;
        mKeyMap[Qt::Key_F3             ] = osgGA::GUIEventAdapter::KEY_F3;
        mKeyMap[Qt::Key_F4             ] = osgGA::GUIEventAdapter::KEY_F4;
        mKeyMap[Qt::Key_F5             ] = osgGA::GUIEventAdapter::KEY_F5;
        mKeyMap[Qt::Key_F6             ] = osgGA::GUIEventAdapter::KEY_F6;
        mKeyMap[Qt::Key_F7             ] = osgGA::GUIEventAdapter::KEY_F7;
        mKeyMap[Qt::Key_F8             ] = osgGA::GUIEventAdapter::KEY_F8;
        mKeyMap[Qt::Key_F9             ] = osgGA::GUIEventAdapter::KEY_F9;
        mKeyMap[Qt::Key_F10            ] = osgGA::GUIEventAdapter::KEY_F10;
        mKeyMap[Qt::Key_F11            ] = osgGA::GUIEventAdapter::KEY_F11;
        mKeyMap[Qt::Key_F12            ] = osgGA::GUIEventAdapter::KEY_F12;
        mKeyMap[Qt::Key_F13            ] = osgGA::GUIEventAdapter::KEY_F13;
        mKeyMap[Qt::Key_F14            ] = osgGA::GUIEventAdapter::KEY_F14;
        mKeyMap[Qt::Key_F15            ] = osgGA::GUIEventAdapter::KEY_F15;
        mKeyMap[Qt::Key_F16            ] = osgGA::GUIEventAdapter::KEY_F16;
        mKeyMap[Qt::Key_F17            ] = osgGA::GUIEventAdapter::KEY_F17;
        mKeyMap[Qt::Key_F18            ] = osgGA::GUIEventAdapter::KEY_F18;
        mKeyMap[Qt::Key_F19            ] = osgGA::GUIEventAdapter::KEY_F19;
        mKeyMap[Qt::Key_F20            ] = osgGA::GUIEventAdapter::KEY_F20;

        mKeyMap[Qt::Key_hyphen         ] = '-';
        mKeyMap[Qt::Key_Equal         ] = '=';

        mKeyMap[Qt::Key_division      ] = osgGA::GUIEventAdapter::KEY_KP_Divide;
        mKeyMap[Qt::Key_multiply      ] = osgGA::GUIEventAdapter::KEY_KP_Multiply;
        mKeyMap[Qt::Key_Minus         ] = '-';
        mKeyMap[Qt::Key_Plus          ] = '+';
        //mKeyMap[Qt::Key_H              ] = osgGA::GUIEventAdapter::KEY_KP_Home;
        //mKeyMap[Qt::Key_                    ] = osgGA::GUIEventAdapter::KEY_KP_Up;
        //mKeyMap[92                    ] = osgGA::GUIEventAdapter::KEY_KP_Page_Up;
        //mKeyMap[86                    ] = osgGA::GUIEventAdapter::KEY_KP_Left;
        //mKeyMap[87                    ] = osgGA::GUIEventAdapter::KEY_KP_Begin;
        //mKeyMap[88                    ] = osgGA::GUIEventAdapter::KEY_KP_Right;
        //mKeyMap[83                    ] = osgGA::GUIEventAdapter::KEY_KP_End;
        //mKeyMap[84                    ] = osgGA::GUIEventAdapter::KEY_KP_Down;
        //mKeyMap[85                    ] = osgGA::GUIEventAdapter::KEY_KP_Page_Down;
        mKeyMap[Qt::Key_Insert        ] = osgGA::GUIEventAdapter::KEY_KP_Insert;
        //mKeyMap[Qt::Key_Delete        ] = osgGA::GUIEventAdapter::KEY_KP_Delete;
    }

    ~QtKeyboardMap()
    {
    }

    int remapKey(QKeyEvent* event)
    {
        KeyMap::iterator itr = mKeyMap.find(event->key());
        if (itr == mKeyMap.end())
        {
#if QT_VERSION < 0x050000
            return int(*(event->text().toAscii().data()));
#else
            return int(*(event->text().toLatin1().data()));
#endif
        }
        else
            return itr->second;
    }

    private:
    typedef std::map<unsigned int, int> KeyMap;
    KeyMap mKeyMap;
};

static QtKeyboardMap s_QtKeyboardMap;

/////////////////////////////////////////////////////////////////////////////////////////////////////
// source code taken from osg 3.3.2
// modifications based on http://forum.openscenegraph.org/viewtopic.php?t=15097
// and on http://polar.inria.fr/downloadfaq/download/

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)) // needed for QOpenGLWidget
#include <QOpenGLContext>
#include <osgUtil/CullVisitor>

using namespace osg;

// Custom FrameBufferObject set via camera default stateset, restores fbo to the one used by QOpenGLWidget when necessary
class CExtraFBO : public osg::FrameBufferObject
{
public:
    virtual void apply(State &state) const
    {
        apply(state, READ_DRAW_FRAMEBUFFER);
    }
    virtual void apply(State &state, BindTarget target) const
    {
        if (getAttachmentMap().empty())
        {
            //osg::FrameBufferObject::apply(state, target);
#if OSG_VERSION_GREATER_OR_EQUAL(3,4,0)
            osg::GLExtensions* ext = state.get<osg::GLExtensions>();
            bool fbo_supported = ext && ext->isFrameBufferObjectSupported;
            if (NULL != ext)
                ext->glBindFramebuffer(GL_FRAMEBUFFER_EXT, ((StateEx*)(&state))->getDefaultFbo());
#else
            osg::FBOExtensions* fbo_ext = osg::FBOExtensions::instance(state.getContextID(), true);
            bool fbo_supported = fbo_ext && fbo_ext->isSupported();
            if (NULL != fbo_ext)
                fbo_ext->glBindFramebuffer(/*osg::FrameBufferObject::READ_DRAW_FRAMEBUFFER*/target, ((StateEx*)(&state))->getDefaultFbo());
#endif
        }
    }
};


// RenderStage override based on OSG 3.3.2 code 
// Restores fbo in some situations to the one used by QOpenGlWidget
// Customized RenderStage is set via customized cullvisitor prototype and to renderer sceneview

namespace osgUtil
{
    // RenderStageCache is direct copy from osgUtil with no modifications
    class RenderStageCache : public osg::Object
    {
    public:

        RenderStageCache() {}
        RenderStageCache(const RenderStageCache&, const osg::CopyOp&) {}

        META_Object(osgUtil, RenderStageCache);

        void setRenderStage(CullVisitor* cv, RenderStage* rs)
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
            _renderStageMap[cv] = rs;
        }

        RenderStage* getRenderStage(osgUtil::CullVisitor* cv)
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
            return _renderStageMap[cv].get();
        }

        typedef std::map<CullVisitor*, osg::ref_ptr<RenderStage> > RenderStageMap;

        /** Resize any per context GLObject buffers to specified size. */
        virtual void resizeGLObjectBuffers(unsigned int maxSize)
        {
            for (RenderStageMap::const_iterator itr = _renderStageMap.begin(); itr != _renderStageMap.end(); ++itr)
            {
                itr->second->resizeGLObjectBuffers(maxSize);
            }
        }

        /** If State is non-zero, this function releases any associated OpenGL objects for
        * the specified graphics context. Otherwise, releases OpenGL objexts
        * for all graphics contexts. */
        virtual void releaseGLObjects(osg::State* state = 0) const
        {
            for (RenderStageMap::const_iterator itr = _renderStageMap.begin(); itr != _renderStageMap.end(); ++itr)
            {
                itr->second->releaseGLObjects(state);
            }
        }

        OpenThreads::Mutex  _mutex;
        RenderStageMap      _renderStageMap;
    };

    // RenderStageEx overrides drawInner and changes fbo switching
    class RenderStageEx : public RenderStage
    {
    public:
        RenderStageEx() :
            RenderStage()
        {}
        virtual const char* className() const
        {
            return "RenderStageEx";
        }
        virtual void drawInner(osg::RenderInfo& renderInfo, RenderLeaf*& previous, bool& doCopyTexture) override
        {
            struct SubFunc
            {
                static void applyReadFBO(bool& apply_read_fbo,
                    const FrameBufferObject* read_fbo, osg::State& state)
                {
                    if (read_fbo->isMultisample())
                    {
                        OSG_WARN << "Attempting to read from a"
                            " multisampled framebuffer object. Set a resolve"
                            " framebuffer on the RenderStage to fix this." << std::endl;
                    }

                    if (apply_read_fbo)
                    {
                        // Bind the monosampled FBO to read from
                        read_fbo->apply(state, FrameBufferObject::READ_FRAMEBUFFER);
                        apply_read_fbo = false;
                    }
                }
            };

            osg::State& state = *renderInfo.getState();

#if 1
    #if OSG_MIN_VERSION_REQUIRED(3,3,7)
            osg::GLExtensions* fbo_ext = state.get<osg::GLExtensions>();
            bool fbo_supported = fbo_ext && fbo_ext->isFrameBufferObjectSupported;
            bool using_multiple_render_targets = fbo_supported && _fbo.valid() && _fbo->hasMultipleRenderingTargets();
    #else
            osg::FBOExtensions* fbo_ext = osg::FBOExtensions::instance(state.getContextID(), true);
            bool fbo_supported = fbo_ext && fbo_ext->isSupported();
            bool using_multiple_render_targets = fbo_supported && _fbo.valid() && _fbo->hasMultipleRenderingTargets();
    #endif

            if (fbo_supported)
            {
                if (_fbo.valid())
                {
                    if (!_fbo->hasMultipleRenderingTargets())
                    {
    #if !defined(OSG_GLES1_AVAILABLE) && !defined(OSG_GLES2_AVAILABLE)

                        if (getDrawBufferApplyMask())
                            glDrawBuffer(_drawBuffer);

                        if (getReadBufferApplyMask())
                            glReadBuffer(_readBuffer);
    #endif
                    }

                    _fbo->apply(state);
                }
                else
                {
                    fbo_ext->glBindFramebuffer(osg::FrameBufferObject::READ_DRAW_FRAMEBUFFER, static_cast<StateEx *>(&state)->getDefaultFbo()); // the MOST IMPORTANT line
                }
            }
#else
            osg::FBOExtensions* fbo_ext = _fbo.valid() ? osg::FBOExtensions::instance(state.getContextID(), true) : 0;
            bool fbo_supported = fbo_ext && fbo_ext->isSupported();

            bool using_multiple_render_targets = fbo_supported && _fbo->hasMultipleRenderingTargets();

            if (!using_multiple_render_targets)
            {
#if !defined(OSG_GLES1_AVAILABLE) && !defined(OSG_GLES2_AVAILABLE)

                if (getDrawBufferApplyMask())
                    glDrawBuffer(_drawBuffer);

                if (getReadBufferApplyMask())
                    glReadBuffer(_readBuffer);

#endif
            }

            if (fbo_supported)
            {
                _fbo->apply(state);
            }
#endif
            //  GLint fboId;
            //  glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING_EXT, &fboId);
            //  std::cout << fboId << std::endl;

            // do the drawing itself.
            RenderBin::draw(renderInfo, previous);

            if (state.getCheckForGLErrors() != osg::State::NEVER_CHECK_GL_ERRORS)
            {
                if (state.checkGLErrors("after RenderBin::draw(..)"))
                {
                    if (fbo_ext)
                    {
                        GLenum fbstatus = fbo_ext->glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT);
                        if (fbstatus != GL_FRAMEBUFFER_COMPLETE_EXT)
                        {
                            OSG_NOTICE << "RenderStage::drawInner(,) FBO status = 0x" << std::hex << fbstatus << std::dec << std::endl;
                        }
                    }
                }
            }

            const FrameBufferObject* read_fbo = fbo_supported ? _fbo.get() : 0;
            bool apply_read_fbo = false;

            if (fbo_supported && _resolveFbo.valid() && fbo_ext->glBlitFramebuffer)
            {
                GLbitfield blitMask = 0;
                bool needToBlitColorBuffers = false;

                //find which buffer types should be copied
                for (FrameBufferObject::AttachmentMap::const_iterator
                    it = _resolveFbo->getAttachmentMap().begin(),
                    end = _resolveFbo->getAttachmentMap().end(); it != end; ++it)
                {
                    switch (it->first)
                    {
                    case Camera::DEPTH_BUFFER:
                        blitMask |= GL_DEPTH_BUFFER_BIT;
                        break;
                    case Camera::STENCIL_BUFFER:
                        blitMask |= GL_STENCIL_BUFFER_BIT;
                        break;
                    case Camera::PACKED_DEPTH_STENCIL_BUFFER:
                        blitMask |= GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
                        break;
                    case Camera::COLOR_BUFFER:
                        blitMask |= GL_COLOR_BUFFER_BIT;
                        break;
                    default:
                        needToBlitColorBuffers = true;
                        break;
                    }
                }

                // Bind the resolve framebuffer to blit into.
                _fbo->apply(state, FrameBufferObject::READ_FRAMEBUFFER);
                _resolveFbo->apply(state, FrameBufferObject::DRAW_FRAMEBUFFER);

                if (blitMask)
                {
                    // Blit to the resolve framebuffer.
                    // Note that (with nvidia 175.16 windows drivers at least) if the read
                    // framebuffer is multisampled then the dimension arguments are ignored
                    // and the whole framebuffer is always copied.
                    fbo_ext->glBlitFramebuffer(
                        0, 0, static_cast<GLint>(_viewport->width()), static_cast<GLint>(_viewport->height()),
                        0, 0, static_cast<GLint>(_viewport->width()), static_cast<GLint>(_viewport->height()),
                        blitMask, GL_NEAREST);
                }

#if !defined(OSG_GLES1_AVAILABLE) && !defined(OSG_GLES2_AVAILABLE)
                if (needToBlitColorBuffers)
                {
                    for (FrameBufferObject::AttachmentMap::const_iterator
                        it = _resolveFbo->getAttachmentMap().begin(),
                        end = _resolveFbo->getAttachmentMap().end(); it != end; ++it)
                    {
                        osg::Camera::BufferComponent attachment = it->first;
                        if (attachment >= osg::Camera::COLOR_BUFFER0)
                        {
                            glReadBuffer(GL_COLOR_ATTACHMENT0_EXT + (attachment - osg::Camera::COLOR_BUFFER0));
                            glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT + (attachment - osg::Camera::COLOR_BUFFER0));

                            fbo_ext->glBlitFramebuffer(
                                0, 0, static_cast<GLint>(_viewport->width()), static_cast<GLint>(_viewport->height()),
                                0, 0, static_cast<GLint>(_viewport->width()), static_cast<GLint>(_viewport->height()),
                                GL_COLOR_BUFFER_BIT, GL_NEAREST);
                        }
                    }
                    // reset the read and draw buffers?  will comment out for now with the assumption that
                    // the buffers will be set explictly when needed elsewhere.
                    // glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
                    // glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
                }
#endif

                apply_read_fbo = true;
                read_fbo = _resolveFbo.get();

                using_multiple_render_targets = read_fbo->hasMultipleRenderingTargets();
            }

            // now copy the rendered image to attached texture.
            if (doCopyTexture)
            {
                if (read_fbo) SubFunc::applyReadFBO(apply_read_fbo, read_fbo, state);
                copyTexture(renderInfo);
            }

            std::map< osg::Camera::BufferComponent, Attachment>::const_iterator itr;
            for (itr = _bufferAttachmentMap.begin();
                itr != _bufferAttachmentMap.end();
                ++itr)
            {
                if (itr->second._image.valid())
                {
                    if (read_fbo) SubFunc::applyReadFBO(apply_read_fbo, read_fbo, state);

#if !defined(OSG_GLES1_AVAILABLE) && !defined(OSG_GLES2_AVAILABLE)

                    if (using_multiple_render_targets)
                    {
                        int attachment = itr->first;
                        if (attachment == osg::Camera::DEPTH_BUFFER || attachment == osg::Camera::STENCIL_BUFFER) {
                            // assume first buffer rendered to is the one we want
                            glReadBuffer(read_fbo->getMultipleRenderingTargets()[0]);
                        }
                        else {
                            glReadBuffer(GL_COLOR_ATTACHMENT0_EXT + (attachment - osg::Camera::COLOR_BUFFER0));
                        }
                    }
                    else {
                        if (_readBuffer != GL_NONE)
                        {
                            glReadBuffer(_readBuffer);
                        }
                    }

#endif

                    GLenum pixelFormat = itr->second._image->getPixelFormat();
                    if (pixelFormat == 0) pixelFormat = _imageReadPixelFormat;
                    if (pixelFormat == 0) pixelFormat = GL_RGB;

                    GLenum dataType = itr->second._image->getDataType();
                    if (dataType == 0) dataType = _imageReadPixelDataType;
                    if (dataType == 0) dataType = GL_UNSIGNED_BYTE;

                    itr->second._image->readPixels(static_cast<int>(_viewport->x()),
                        static_cast<int>(_viewport->y()),
                        static_cast<int>(_viewport->width()),
                        static_cast<int>(_viewport->height()),
                        pixelFormat, dataType);
                }
            }

            if (fbo_supported)
            {
                if (getDisableFboAfterRender())
                {
                    // switch off the frame buffer object
                    GLuint fboId = state.getGraphicsContext() ? state.getGraphicsContext()->getDefaultFboId() : 0;
                    fbo_ext->glBindFramebuffer(GL_FRAMEBUFFER_EXT, fboId);
                }

                doCopyTexture = true;
            }

            if (fbo_supported && _camera.valid())
            {
                // now generate mipmaps if they are required.
                const osg::Camera::BufferAttachmentMap& bufferAttachments = _camera->getBufferAttachmentMap();
                for (osg::Camera::BufferAttachmentMap::const_iterator itr = bufferAttachments.begin();
                    itr != bufferAttachments.end();
                    ++itr)
                {
                    if (itr->second._texture.valid() && itr->second._mipMapGeneration)
                    {
                        state.setActiveTextureUnit(0);
                        state.applyTextureAttribute(0, itr->second._texture.get());
                        fbo_ext->glGenerateMipmap(itr->second._texture->getTextureTarget());
                    }
                }
            }
        }

    };

}

//! custom cull visitor to be able to replace RenderStage with RenderStageEx in camera apply
class CullVisitorEx : public osgUtil::CullVisitor
{
public:
    META_NodeVisitor(Ex, CullVisitorEx);

    CullVisitorEx() : osgUtil::CullVisitor()
    {
        setRenderStage(new osgUtil::RenderStageEx());
    }
    CullVisitorEx(const CullVisitorEx& cv) : osgUtil::CullVisitor(cv) 
    { 
        setRenderStage(new osgUtil::RenderStageEx());
    }
    CullVisitorEx* clone() const { return new CullVisitorEx(*this); }

    virtual void apply(osg::Camera& camera);
};


void CullVisitorEx::apply(osg::Camera& camera)
{
    // push the node's state.
    StateSet* node_state = camera.getStateSet();
    if (node_state) pushStateSet(node_state);

    //#define DEBUG_CULLSETTINGS

#ifdef DEBUG_CULLSETTINGS
    if (osg::isNotifyEnabled(osg::NOTICE))
    {
        OSG_NOTICE << std::endl << std::endl << "CullVisitor, before : ";
        write(osg::notify(osg::NOTICE));
    }
#endif

    // Save current cull settings
    CullSettings saved_cull_settings(*this);

#ifdef DEBUG_CULLSETTINGS
    if (osg::isNotifyEnabled(osg::NOTICE))
    {
        OSG_NOTICE << "CullVisitor, saved_cull_settings : ";
        saved_cull_settings.write(osg::notify(osg::NOTICE));
    }
#endif

#if 1
    // set cull settings from this Camera
    setCullSettings(camera);

#ifdef DEBUG_CULLSETTINGS
    OSG_NOTICE << "CullVisitor, after setCullSettings(camera) : ";
    write(osg::notify(osg::NOTICE));
#endif
    // inherit the settings from above
    inheritCullSettings(saved_cull_settings, camera.getInheritanceMask());

#ifdef DEBUG_CULLSETTINGS
    OSG_NOTICE << "CullVisitor, after inheritCullSettings(saved_cull_settings," << camera.getInheritanceMask() << ") : ";
    write(osg::notify(osg::NOTICE));
#endif

#else
    // activate all active cull settings from this Camera
    inheritCullSettings(camera);
#endif

    // set the cull mask.
    unsigned int savedTraversalMask = getTraversalMask();
    bool mustSetCullMask = (camera.getInheritanceMask() & osg::CullSettings::CULL_MASK) == 0;
    if (mustSetCullMask) setTraversalMask(camera.getCullMask());

    RefMatrix& originalModelView = *getModelViewMatrix();

    osg::RefMatrix* projection = 0;
    osg::RefMatrix* modelview = 0;

    if (camera.getReferenceFrame() == osg::Transform::RELATIVE_RF)
    {
        if (camera.getTransformOrder() == osg::Camera::POST_MULTIPLY)
        {
            projection = createOrReuseMatrix(*getProjectionMatrix()*camera.getProjectionMatrix());
            modelview = createOrReuseMatrix(*getModelViewMatrix()*camera.getViewMatrix());
        }
        else // pre multiply
        {
            projection = createOrReuseMatrix(camera.getProjectionMatrix()*(*getProjectionMatrix()));
            modelview = createOrReuseMatrix(camera.getViewMatrix()*(*getModelViewMatrix()));
        }
    }
    else
    {
        // an absolute reference frame
        projection = createOrReuseMatrix(camera.getProjectionMatrix());
        modelview = createOrReuseMatrix(camera.getViewMatrix());
    }


    if (camera.getViewport()) pushViewport(camera.getViewport());

    // record previous near and far values.
    value_type previous_znear = _computed_znear;
    value_type previous_zfar = _computed_zfar;

    // take a copy of the current near plane candidates
    DistanceMatrixDrawableMap  previousNearPlaneCandidateMap;
    previousNearPlaneCandidateMap.swap(_nearPlaneCandidateMap);

    DistanceMatrixDrawableMap  previousFarPlaneCandidateMap;
    previousFarPlaneCandidateMap.swap(_farPlaneCandidateMap);

    _computed_znear = FLT_MAX;
    _computed_zfar = -FLT_MAX;

    pushProjectionMatrix(projection);
    pushModelViewMatrix(modelview, camera.getReferenceFrame());


    if (camera.getRenderOrder() == osg::Camera::NESTED_RENDER)
    {
        handle_cull_callbacks_and_traverse(camera);
    }
    else
    {
        // set up lighting.
        // currently ignore lights in the scene graph itself..
        // will do later.
        osgUtil::RenderStage* previous_stage = getCurrentRenderBin()->getStage();

        // use render to texture stage.
        // create the render to texture stage.
        osg::ref_ptr<osgUtil::RenderStageCache> rsCache = dynamic_cast<osgUtil::RenderStageCache*>(camera.getRenderingCache());
        if (!rsCache)
        {
            rsCache = rsCache = new osgUtil::RenderStageCache;
            camera.setRenderingCache(rsCache.get());
        }

        osg::ref_ptr<osgUtil::RenderStage> rtts = rsCache->getRenderStage(this);
        if (!rtts)
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(*(camera.getDataChangeMutex()));

            rtts = new osgUtil::RenderStageEx;
            rsCache->setRenderStage(this, rtts.get());

            rtts->setCamera(&camera);

            if (camera.getInheritanceMask() & DRAW_BUFFER)
            {
                // inherit draw buffer from above.
                rtts->setDrawBuffer(previous_stage->getDrawBuffer(), previous_stage->getDrawBufferApplyMask());
            }
            else
            {
                rtts->setDrawBuffer(camera.getDrawBuffer());
            }

            if (camera.getInheritanceMask() & READ_BUFFER)
            {
                // inherit read buffer from above.
                rtts->setReadBuffer(previous_stage->getReadBuffer(), previous_stage->getReadBufferApplyMask());
            }
            else
            {
                rtts->setReadBuffer(camera.getReadBuffer());
            }
        }
        else
        {
            // reusing render to texture stage, so need to reset it to empty it from previous frames contents.
            rtts->reset();
        }

        // set up clera masks/values
        rtts->setClearDepth(camera.getClearDepth());
        rtts->setClearAccum(camera.getClearAccum());
        rtts->setClearStencil(camera.getClearStencil());
        rtts->setClearMask(camera.getClearMask());


        // set up the background color and clear mask.
        if (camera.getInheritanceMask() & CLEAR_COLOR)
        {
            rtts->setClearColor(previous_stage->getClearColor());
        }
        else
        {
            rtts->setClearColor(camera.getClearColor());
        }
        if (camera.getInheritanceMask() & CLEAR_MASK)
        {
            rtts->setClearMask(previous_stage->getClearMask());
        }
        else
        {
            rtts->setClearMask(camera.getClearMask());
        }


        // set the color mask.
        osg::ColorMask* colorMask = camera.getColorMask() != 0 ? camera.getColorMask() : previous_stage->getColorMask();
        rtts->setColorMask(colorMask);

        // set up the viewport.
        osg::Viewport* viewport = camera.getViewport() != 0 ? camera.getViewport() : previous_stage->getViewport();
        rtts->setViewport(viewport);

        // set initial view matrix
        rtts->setInitialViewMatrix(modelview);

        // set up to charge the same PositionalStateContainer is the parent previous stage.
        osg::Matrix inheritedMVtolocalMV;
        inheritedMVtolocalMV.invert(originalModelView);
        inheritedMVtolocalMV.postMult(*getModelViewMatrix());
        rtts->setInheritedPositionalStateContainerMatrix(inheritedMVtolocalMV);
        rtts->setInheritedPositionalStateContainer(previous_stage->getPositionalStateContainer());

        // record the render bin, to be restored after creation
        // of the render to text
        osgUtil::RenderBin* previousRenderBin = getCurrentRenderBin();

        // set the current renderbin to be the newly created stage.
        setCurrentRenderBin(rtts.get());

        // traverse the subgraph
        {
            handle_cull_callbacks_and_traverse(camera);
        }

        // restore the previous renderbin.
        setCurrentRenderBin(previousRenderBin);


        if (rtts->getStateGraphList().size() == 0 && rtts->getRenderBinList().size() == 0)
        {
            // getting to this point means that all the subgraph has been
            // culled by small feature culling or is beyond LOD ranges.
        }


        // and the render to texture stage to the current stages
        // dependancy list.
        switch (camera.getRenderOrder())
        {
        case osg::Camera::PRE_RENDER:
            getCurrentRenderBin()->getStage()->addPreRenderStage(rtts.get(), camera.getRenderOrderNum());
            break;
        default:
            getCurrentRenderBin()->getStage()->addPostRenderStage(rtts.get(), camera.getRenderOrderNum());
            break;
        }

    }

    // restore the previous model view matrix.
    popModelViewMatrix();

    // restore the previous model view matrix.
    popProjectionMatrix();


    // restore the original near and far values
    _computed_znear = previous_znear;
    _computed_zfar = previous_zfar;

    // swap back the near plane candidates
    previousNearPlaneCandidateMap.swap(_nearPlaneCandidateMap);
    previousFarPlaneCandidateMap.swap(_farPlaneCandidateMap);


    if (camera.getViewport()) popViewport();

    // restore the previous traversal mask settings
    if (mustSetCullMask) setTraversalMask(savedTraversalMask);

    // restore the previous cull settings
    setCullSettings(saved_cull_settings);

    // pop the node's state off the render graph stack.
    if (node_state) popStateSet();
}

#endif // QT_VERSION

///////////////////////////////////////////////////////////////////////////////
// Graphics window for single threaded use

QtOSGGraphicsWindow::QtOSGGraphicsWindow(Traits *traits, BASEGLWidget *widget) :
    osgViewer::GraphicsWindowEmbedded(traits)
{
    m_glCanvas = widget;
    m_bContextX = false;
    // set osgViewer::GraphicsWindow traits
    _traits = traits;    
    init();
}

QtOSGGraphicsWindow::~QtOSGGraphicsWindow()
{
    close();
}

void QtOSGGraphicsWindow::init()
{
    if (valid())
    {
        setState( new StateEx);
        getState()->setGraphicsContext(this);

#if OSG_VERSION_GREATER_OR_EQUAL(3,1,10)
		if (_traits.valid() && _traits->sharedContext.get())
#else
        if (_traits.valid() && _traits->sharedContext)
#endif
        {
            getState()->setContextID( _traits->sharedContext->getState()->getContextID() );
            incrementContextIDUsageCount( getState()->getContextID() );
        }
        else
        {
            getState()->setContextID( osg::GraphicsContext::createNewContextID() );
        }
    }
}

// dummy implementations, assume that graphics context is *always* current and valid.
bool QtOSGGraphicsWindow::valid() const 
{ 
    return true; 
}

bool QtOSGGraphicsWindow::realizeImplementation() 
{ 
    return true; 
}

bool QtOSGGraphicsWindow::isRealizedImplementation() const  
{ 
    return true; 
}

void QtOSGGraphicsWindow::closeImplementation() 
{
}

//#define DEBUG_CONTEXT_SWITCH

bool QtOSGGraphicsWindow::makeCurrentImplementation() 
{ 
#ifdef DEBUG_CONTEXT_SWITCH
    if (dynamic_cast<CVolumeRendererWindow *>(m_glCanvas) != NULL)
        qDebug() << "makeCurrent " << QThread::currentThreadId() << " " << wglGetCurrentContext();
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    if (m_glCanvas->context()!=QOpenGLContext::currentContext())
    {
        m_glCanvas->makeCurrent();
        m_bContextX = true;
        qDebug() << "QtOSGGraphicsWindow::makeCurrentImplementation: fixing bad context";
    }
#else
    if (m_glCanvas->context()!=QGLContext::currentContext())
    {
        m_glCanvas->makeCurrent();
        m_bContextX = true;
        qDebug() << "QtOSGGraphicsWindow::makeCurrentImplementation: fixing bad context";
    }
#endif
    return true; 
}

bool QtOSGGraphicsWindow::releaseContextImplementation()
{ 
#ifdef DEBUG_CONTEXT_SWITCH
    if (dynamic_cast<CVolumeRendererWindow *>(m_glCanvas) != NULL)
        qDebug() << "doneCurrent " << QThread::currentThreadId();
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    if (m_bContextX)
        m_glCanvas->doneCurrent();
#else
    if (m_bContextX)
        m_glCanvas->doneCurrent();
#endif
    m_bContextX = false;
    return true; 
}

void QtOSGGraphicsWindow::swapBuffersImplementation() 
{
}

void QtOSGGraphicsWindow::grabFocus() 
{
}

void QtOSGGraphicsWindow::grabFocusIfPointerInWindow() 
{
}

void QtOSGGraphicsWindow::raiseWindow() 
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Graphics window for multithreaded opengl with known issues on AMD cards

QtOSGGraphicsWindowMT::QtOSGGraphicsWindowMT(Traits *traits, BASEGLWidget *widget) :
    osgQt::GraphicsWindowQt(traits)
{
    m_glCanvas = widget;
    // set osgViewer::GraphicsWindow traits
    _traits = traits;
    init();
}

QtOSGGraphicsWindowMT::~QtOSGGraphicsWindowMT()
{
}

void QtOSGGraphicsWindowMT::init()
{
    if (valid())
    {
        setState( new StateEx );
        getState()->setGraphicsContext(this);

#if OSG_VERSION_GREATER_OR_EQUAL(3,1,10)
		if (_traits.valid() && _traits->sharedContext.get())
#else
        if (_traits.valid() && _traits->sharedContext)
#endif
        {
            getState()->setContextID( _traits->sharedContext->getState()->getContextID() );
            incrementContextIDUsageCount( getState()->getContextID() );
        }
        else
        {
            getState()->setContextID( osg::GraphicsContext::createNewContextID() );
        }
    }
}

bool QtOSGGraphicsWindowMT::valid() const 
{ 
    if (m_glCanvas && m_glCanvas->isValid())
        return true;
    return false; 
}

bool QtOSGGraphicsWindowMT::makeCurrentImplementation()
{
#ifdef DEBUG_CONTEXT_SWITCH
    if (dynamic_cast<CVolumeRendererWindow *>(m_glCanvas) != NULL)
        qDebug() << "makeCurrent " << QThread::currentThreadId() << " " << wglGetCurrentContext();
#endif
    if (m_glCanvas && m_glCanvas->isVisible())
    {
        lock();
        m_glCanvas->makeCurrent();
        return true;
    }
    return false;
}

void QtOSGGraphicsWindowMT::swapBuffersImplementation()
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    QGLWidget* pQt4GlWidget = dynamic_cast<QGLWidget*>(m_glCanvas);
    if (pQt4GlWidget && pQt4GlWidget->doubleBuffer())
    {
#ifndef __APPLE__
        pQt4GlWidget->swapBuffers();
#endif
    }
#else
    if (m_glCanvas && m_glCanvas->doubleBuffer())
    {
  #ifndef __APPLE__
        m_glCanvas->swapBuffers();
  #endif
    }
#endif
}

bool QtOSGGraphicsWindowMT::releaseContextImplementation()
{
#ifdef DEBUG_CONTEXT_SWITCH
    if (dynamic_cast<CVolumeRendererWindow *>(m_glCanvas) != NULL)
        qDebug() << "doneCurrent " << QThread::currentThreadId();
#endif
    if (m_glCanvas)
    {
        m_glCanvas->doneCurrent();
        unlock();
        return true;
    }
    return false;
}

void QtOSGGraphicsWindowMT::closeImplementation()
{
    if (m_glCanvas)
        m_glCanvas->hide();
}

bool QtOSGGraphicsWindowMT::realizeImplementation()
{
    if( m_glCanvas )
    {
        m_glCanvas->show();
        return true;
    }
    return false;
}

bool QtOSGGraphicsWindowMT::isRealizedImplementation() const
{
    return (m_glCanvas) ? m_glCanvas->isVisible() : false;
}

///////////////////////////////////////////////////////////////////////////////
// Viewer implementation

void QtOSGViewer::setUpThreading()
{
    // override default implementation which sets thread affinity
    // osgViewer::Viewer::setUpThreading();
    Contexts contexts;
    getContexts(contexts);
    if(_threadingModel == SingleThreaded)
    {
        if(_threadsRunning)
            stopThreading();
    }
    else 
    {
        if(!_threadsRunning)
            startThreading();
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Qt-OSG GLWidget implementation

QGLOSGWidget::QGLOSGWidget(QWidget *parent) :
    BASEGLWidget(parent)
    //BASEGLWidget(QGLFormat(true ? QGL::SampleBuffers : (QGL::FormatOptions)0), parent)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    // We're gonna need custom cullvisitor 
    if (NULL == dynamic_cast<CullVisitorEx*>(osgUtil::CullVisitor::prototype().get()))
    {
        CullVisitorEx* cve = new CullVisitorEx();
        osgUtil::CullVisitor::prototype() = cve;
    }
#endif
    m_lastRenderingTime = 0;
    m_bAntialiasing = false;
    osg::Vec4 color( 0,0,0,0 );
    init(parent,color);
}

QGLOSGWidget::QGLOSGWidget(QWidget *parent, const osg::Vec4 &bgColor, bool bAntialiasing):
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    //BASEGLWidget(QGLFormat(bAntialiasing ? QGL::SampleBuffers : (QGL::FormatOptions)0), parent)
    BASEGLWidget(parent) // for QOpenGLWidget
#else
    BASEGLWidget(QGLFormat(bAntialiasing?QGL::SampleBuffers:(QGL::FormatOptions)0),parent)
#endif
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    // We're gonna need custom cullvisitor 
    if (NULL == dynamic_cast<CullVisitorEx*>(osgUtil::CullVisitor::prototype().get()))
    {
        CullVisitorEx* cve = new CullVisitorEx();
        osgUtil::CullVisitor::prototype() = cve;
    }
#endif
    m_lastRenderingTime = 0;
    m_bAntialiasing = bAntialiasing;
    init(parent,bgColor);
}

void QGLOSGWidget::init(QWidget *parent, const osg::Vec4 &bgColor)
{    
    QSize size(100,100);
    // creating graphics window object for scene OpenGL display
    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits(osg::DisplaySettings::instance().get());
    traits->width = size.width();
    traits->height = size.height();
    traits->doubleBuffer = true;
    traits->sharedContext = 0;
//	m_graphic_window = new osgViewer::GraphicsWindowEmbedded(0, 0, size.width(), size.height());
    m_graphic_window = new QtOSGGraphicsWindow(traits,this);
    //m_graphic_window = new QtOSGGraphicsWindowMT(traits,this);

    // creating osgViewer::Viewer object for scene manipulation controling
    m_view = new QtOSGViewer;
    m_view->setRunFrameScheme(osgViewer::ViewerBase::ON_DEMAND);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)) // needed for QOpenGLWidget
    osgViewer::Renderer* renderer = dynamic_cast<osgViewer::Renderer*>(m_view->getCamera()->getRenderer());
    if (NULL != renderer)
    {
        if (NULL != renderer->getSceneView(0))
            renderer->getSceneView(0)->setRenderStage(new osgUtil::RenderStageEx());
        if (NULL != renderer->getSceneView(1))
            renderer->getSceneView(1)->setRenderStage(new osgUtil::RenderStageEx());
    }
#endif

    // setting necessary viewer attributes
    m_view->getCamera()->setClearColor( osg::Vec4(0.2, 0.2, 0.6, 1.0) );
    m_view->getCamera()->setViewport(0, 0, size.width(), size.height());
    m_view->getCamera()->setProjectionMatrixAsPerspective( 30.0f, static_cast<double>( size.width() )/ static_cast<double>( size.height() ), 1.0f, 10000.0f );
    m_view->getCamera()->setGraphicsContext(m_graphic_window.get());

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)) // needed for QOpenGLWidget
    // to correctly restore fbo after osg::FrameBufferObject was set via stateset change during draw we change default camera stateset to a new one with our customized 
    // fbo which restores fbo to the one used by QOpenGLWidget
    osg::StateSet *ssc = m_view->getCamera()->getOrCreateStateSet();
    ssc->setGlobalDefaults();
    ssc->setAttributeAndModes(new CExtraFBO);
#endif    

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)) // needed for QOpenGLWidget
    //m_view->getCamera()->setPreDrawCallback(new BindFboPreDrawCallbackX());
    m_view->getCamera()->setFinalDrawCallback(new UnBindFboPostDrawCallback);
#endif

    // monitor gpu to adjust frame rate
    osg::ref_ptr<osg::Stats> pStats = m_view->getCamera()->getStats();
    pStats = m_view->getCamera()->getStats();    
    pStats->collectStats("gpu",true);

    //m_view->addEventHandler(new osgViewer::StatsHandler);
    m_view->setThreadingModel(osgViewer::Viewer::SingleThreaded);
    m_view->setCameraManipulator(new osgGA::TrackballManipulator); // should be osg::SceneManipulator, but because implementation differs between apps, their osgcanvas handles it
    m_view->setKeyEventSetsDone(0); // if not called then escape key "breaks" view    

    // Sets the widget's clear color
    setBackgroundColor(bgColor);

    // TODO: fix garbage in background when switching tabs with GLWidgets
    setAttribute(Qt::WA_OpaquePaintEvent );
    //setAutoFillBackground(false);

    // we need to set focus policy to be able to handle keyboard input (Mouse mode modifiers)
    setFocusPolicy(Qt::StrongFocus);
    // set mouse tracking so we can extract continously density from a point under cursor
    setMouseTracking(true);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    if (m_bAntialiasing)
    {
        QSurfaceFormat format = QSurfaceFormat::defaultFormat();
        format.setSamples(4);
        QOpenGLWidget* pQt5GlWidget = dynamic_cast<QOpenGLWidget*>(this);
        if (NULL != pQt5GlWidget)
            pQt5GlWidget->setFormat(format);
    }
    if (0) // enable this to be able to use QOpenGLDebugLogger or glDebugMessageCallback
    {
        QSurfaceFormat format = QSurfaceFormat::defaultFormat();
        format.setProfile(QSurfaceFormat::CoreProfile);
        format.setOption(QSurfaceFormat::DebugContext);
        QOpenGLWidget* pQt5GlWidget = dynamic_cast<QOpenGLWidget*>(this);
        if (NULL != pQt5GlWidget)
            pQt5GlWidget->setFormat(format);
    }
#endif
}

QGLOSGWidget::~QGLOSGWidget()
{
}

void QGLOSGWidget::initializeGL()
{
    /*
    m_graphic_window->setClearMask( GL_COLOR_BUFFER_BIT |
                                    GL_DEPTH_BUFFER_BIT );
    m_graphic_window->clear();
    m_graphic_window->setClearMask( 0 );
    */
    if (glewInit() != GLEW_OK)
    {
        bool damn= true;
    }
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    QOpenGLWidget* pQt5GlWidget = dynamic_cast<QOpenGLWidget*>(this);
    if (NULL != pQt5GlWidget && m_bAntialiasing)
        glEnable(GL_MULTISAMPLE);
#endif
}

void QGLOSGWidget::resizeGL(int width, int height)
{
    if (m_graphic_window.valid())
    {
        getEventQueue()->windowResize(/*m_graphic_window->getTraits()*/x(),y(), width, height);
        m_graphic_window->resized(x(), y(), width, height);        
    }
    onResize(width,height);
}

void QGLOSGWidget::frame()
{
    if (m_view.valid())
    {
        makeCurrent();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
        static_cast<StateEx *>(m_graphic_window->getState())->setDefaultFbo(defaultFramebufferObject());
#endif
        m_view->frame();
        doneCurrent();
    }
}

void QGLOSGWidget::paintGL()
{
    if (m_view.valid()
            /*&& m_view->checkNeedToDoFrame()*/ // this doesn't work properly with canvases in volume crop
            )
    {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
        static_cast<StateEx *>(m_graphic_window->getState())->setDefaultFbo(defaultFramebufferObject());        
#endif
        m_view->frame();
        // get statistics
        double value = 0;
        osg::ref_ptr<osg::Stats> pStats = m_view->getCamera()->getStats();
        int frame = pStats->getLatestFrameNumber();
        // do not take last two frames as these might not be finished yet
        // when gpu is overloaded it might be even more - that's why getAveragedAttribute is better
        if (frame>5)
            pStats->getAveragedAttribute(frame-5, frame-2, "GPU draw time taken",value);
        else if (frame>2)
            pStats->getAttribute(frame-2,"GPU draw time taken",value);
        m_lastRenderingTime = value*1000;
    }
}

void QGLOSGWidget::glDraw()
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    /*QGLWidget* pQt4GlWidget = dynamic_cast<QGLWidget*>(this);
    if (NULL != pQt4GlWidget)
        QGLWidget::glDraw();*/
#else
    BASEGLWidget::glDraw();
#endif
}

void QGLOSGWidget::glInit()
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    /*QGLWidget* pQt4GlWidget = dynamic_cast<QGLWidget*>(this);
    if (NULL != pQt4GlWidget)
        QGLWidget::glInit();*/
#else
    BASEGLWidget::glInit();
#endif
}

void QGLOSGWidget::setScene(osg::Node * scene)
{
    if( m_view.get() )
        m_view->setSceneData(scene);
}

osg::Node *	QGLOSGWidget::getScene() 
{
    if( m_view.get() )
        return m_view->getSceneData();
	return NULL;
}

void QGLOSGWidget::addEventHandler(osgGA::GUIEventHandler * handler)
{
    if( !handler || !(m_view.get()) ) return;

    m_view->addEventHandler(handler);
}

void QGLOSGWidget::addEventHandlerFront(osgGA::GUIEventHandler * handler)
{
    if( !handler || !(m_view.get()) ) return;

    m_view->getEventHandlers().push_front(handler);
}

bool QGLOSGWidget::findEventHandler(osgGA::GUIEventHandler * handler)
{
    if( !handler || !(m_view.get()) ) return false;
    osgViewer::View::EventHandlers ehs = m_view->getEventHandlers();
    osgViewer::View::EventHandlers::iterator findIter = std::find(ehs.begin(), ehs.end(), handler);
    return findIter!=ehs.end();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   Removes the event handler described by handler.
//!
//!\param [in,out]  handler If non-null, the handler. 
////////////////////////////////////////////////////////////////////////////////////////////////////
void QGLOSGWidget::removeEventHandler( osgGA::GUIEventHandler *handler )
{
    if(handler != 0)
        m_view->removeEventHandler(handler);
}

void QGLOSGWidget::centerAndScale()
{
    m_view->getCameraManipulator()->computeHomePosition();
    m_view->getCameraManipulator()->home(0.0);
    Refresh(false);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   Center and scale. 
//!
//!\param   box The box. 
////////////////////////////////////////////////////////////////////////////////////////////////////
void QGLOSGWidget::centerAndScale( const osg::BoundingBox & box )
{
    // Try to get camera manipulator
    osg::CSceneManipulator * sm( dynamic_cast< osg::CSceneManipulator * >( m_view->getCameraManipulator() ) );

    if( sm != 0 )
    {
        sm->storeBB( box );
        sm->useStoredBox( true );

        m_view->getCameraManipulator()->home(0.0);
        Refresh(false);
    }
}

void QGLOSGWidget::Refresh(bool bEraseBackground)
{
    update(); // or repaint for immediate repaint or UpdateGL?
}


void QGLOSGWidget::setBackgroundColor(const osg::Vec4 &color)
{
    m_view->getCamera()->setClearColor( color );
    //setBackgroundRole(QPalette::Mid);
    //setForegroundRole(QPalette::Mid);
}


void QGLOSGWidget::keyPressEvent( QKeyEvent* event )
{
    setKeyboardModifiers(event);
#if(0) // doesn't handle correctly special keys like arrow keys
    QString keyString   = event->text();
    QByteArray arrKey = keyString.toLocal8Bit();
    const char* keyData = arrKey.data();
    this->getEventQueue()->keyPress( osgGA::GUIEventAdapter::KeySymbol( *keyData ) );
#else
    int value = s_QtKeyboardMap.remapKey( event );
	getEventQueue()->keyPress( value );
#endif
}

void QGLOSGWidget::keyReleaseEvent( QKeyEvent* event )
{
    setKeyboardModifiers(event);
#if(0)
    QString keyString   = event->text();
    QByteArray arrKey = keyString.toLocal8Bit();
    const char* keyData = arrKey.data();
    this->getEventQueue()->keyRelease( osgGA::GUIEventAdapter::KeySymbol( *keyData ) );
#else
    int value = s_QtKeyboardMap.remapKey( event );
	getEventQueue()->keyRelease( value );
#endif
}

void QGLOSGWidget::wheelEvent ( QWheelEvent * event )
{
    setKeyboardModifiers(event);
    /*event->accept();
    int delta = event->delta();
    osgGA::GUIEventAdapter::ScrollingMotion motion = delta > 0 ?   osgGA::GUIEventAdapter::SCROLL_UP
                                                                : osgGA::GUIEventAdapter::SCROLL_DOWN;
    this->getEventQueue()->mouseScroll( motion );
    Refresh(false);*/
	getEventQueue()->mouseScroll(
        event->orientation() == Qt::Vertical ?
            (event->delta()>0 ? osgGA::GUIEventAdapter::SCROLL_UP : osgGA::GUIEventAdapter::SCROLL_DOWN) :
            (event->delta()>0 ? osgGA::GUIEventAdapter::SCROLL_LEFT : osgGA::GUIEventAdapter::SCROLL_RIGHT) );
}

osgGA::EventQueue* QGLOSGWidget::getEventQueue() const
{
    osgGA::EventQueue* eventQueue = m_graphic_window->getEventQueue();
    if( eventQueue )
        return eventQueue;
    else
        throw std::runtime_error( "Unable to obtain valid event queue");
} 

void QGLOSGWidget::paintEvent( QPaintEvent* event )
{
    BASEGLWidget::paintEvent(event);
}

void QGLOSGWidget::onHome()
{
    m_view->home();
}

void QGLOSGWidget::onResize( int width, int height )
{
	std::vector<osg::Camera*> cameras;
	m_view->getCameras( cameras );
	if ( cameras.size() == 1 )
		cameras.front()->setViewport( 0, 0, width, height );
}

void QGLOSGWidget::setKeyboardModifiers( QInputEvent* event )
{
    int modkey = event->modifiers() & (Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier);
    unsigned int mask = 0;
    if ( modkey & Qt::ShiftModifier ) mask |= osgGA::GUIEventAdapter::MODKEY_SHIFT;
    if ( modkey & Qt::ControlModifier ) mask |= osgGA::GUIEventAdapter::MODKEY_CTRL;
    if ( modkey & Qt::AltModifier ) mask |= osgGA::GUIEventAdapter::MODKEY_ALT;
    getEventQueue()->getCurrentEventState()->setModKeyMask( mask );
}

void QGLOSGWidget::mousePressEvent ( QMouseEvent * event )
{
    // 1 = left mouse button
    // 2 = middle mouse button
    // 3 = right mouse button
    unsigned int button = 0;
    switch( event->button() )
    {
    case Qt::LeftButton:      button = 1;      break;
    case Qt::MiddleButton:    button = 2;      break;
    case Qt::RightButton:     button = 3;      break;
    default:      break;
    }
    setKeyboardModifiers( event );
    getEventQueue()->mouseButtonPress( static_cast<float>( event->x() ),
                                             static_cast<float>( event->y() ),
                                             button );
}

void QGLOSGWidget::mouseReleaseEvent ( QMouseEvent * event )
{    
    // 1 = left mouse button
    // 2 = middle mouse button
    // 3 = right mouse button

    unsigned int button = 0;

    switch( event->button() )
    {
    case Qt::LeftButton:      button = 1;      break;
    case Qt::MiddleButton:    button = 2;      break;
    case Qt::RightButton:     button = 3;      break;
    default:      break;
    }
    setKeyboardModifiers(event);
    getEventQueue()->mouseButtonRelease( static_cast<float>( event->x() ),
                                               static_cast<float>( event->y() ),
                                               button );
}

void QGLOSGWidget::mouseDoubleClickEvent( QMouseEvent* event )
{
    int button = 0;
    switch ( event->button() )
    {
        case Qt::LeftButton: button = 1; break;
        case Qt::MidButton: button = 2; break;
        case Qt::RightButton: button = 3; break;
        case Qt::NoButton: button = 0; break;
        default: button = 0; break;
    }
    setKeyboardModifiers( event );
    getEventQueue()->mouseDoubleButtonPress( event->x(), event->y(), button );
}

void QGLOSGWidget::mouseMoveEvent ( QMouseEvent * event )
{
    setKeyboardModifiers( event );
    this->getEventQueue()->mouseMotion( static_cast<float>( event->x() ),
                                        static_cast<float>( event->y() ) );
    //if (Qt::NoButton!=event->buttons())
    //    Refresh(false);
}

void QGLOSGWidget::enterEvent ( QEvent * event )
{
    BASEGLWidget::enterEvent(event);
    if (!hasFocus())
        setFocus(Qt::MouseFocusReason);
    // call refresh so the osg items can receive the information too
    Refresh(false);
}

void QGLOSGWidget::leaveEvent ( QEvent * event )
{
    BASEGLWidget::leaveEvent(event);
    // call refresh so the osg items can receive the information too
    Refresh(false);
}

bool QGLOSGWidget::event(QEvent *event)
{
    bool handled = BASEGLWidget::event(event);

    // This ensures that the OSG widget is always going to be repainted after the
    // user performed some interaction. Doing this in the event handler ensures
    // that we don't forget about some event and prevents duplicate code.
    switch( event->type() )
    {
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseMove:
    case QEvent::Wheel:
            update();
            break;
    default:
            break;
    }
    return handled;
}

void QGLOSGWidget::enableMultiThreaded()
{
    if (!m_view.get() || !m_graphic_window.get()) return;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    return; // Qt5 multithreaded support is broken
    // http://osg-users.openscenegraph.narkive.com/yR4B7TDq/qt-5-4-osg-3-2-1-multithreading
    // http://pastebin.com/W2QJj8bQ#
#endif
    {   // replace safer single threaded graphics window with the multithreaded one
        m_graphic_window = new QtOSGGraphicsWindowMT(const_cast<osg::GraphicsContext::Traits*>(m_graphic_window->getTraits()),this);
        m_view->getCamera()->setGraphicsContext(m_graphic_window.get());
    }
    m_view->setThreadingModel(osgViewer::Viewer::CullDrawThreadPerContext);
#if OSG_VERSION_LESS_THAN(3,5,0)
    m_view->setThreadSafeReferenceCounting(true); // sets the default but doesn't enable it for the object (static function)
#endif
    m_view->setThreadSafeRefUnref(true); // enables thread safety for the object
    m_view->setEndBarrierPosition(osgViewer::ViewerBase::AfterSwapBuffers);
}

////////////////////////////////////////////////////////////
//

void QGLOSGWidget::screenShot( osg::Image * img, unsigned int scalePercent, bool bWantWidgets )
{
    osg::ref_ptr< osg::Image > p_Image = img;
    osg::ref_ptr< osg::Camera > p_OrigCam = m_view->getCamera();
    osg::ref_ptr< osg::Viewport > p_OrigView = p_OrigCam->getViewport();

    osg::Matrix m = m_view->getCameraManipulator()->getMatrix();

//    double ratio = p_OrigView->width() / (double) p_OrigView->height();

    int w, h;
    w = p_OrigView->width();
    h = p_OrigView->height();
#define max(x,y) (x)>(y)?(x):(y)
    w = max(1,(int)(w*scalePercent/100.0));
    h = max(1,(int)(h*scalePercent/100.0));
    /*
    if ( maxSide == 0)
    {
        w = p_OrigView->width();
        h = p_OrigView->height();
    }
    else
    {
        if (p_OrigView->width()>=p_OrigView->height())
        {
            w = maxSide;
            h = maxSide / ratio;
        }
        else
        {
            w = maxSide * ratio;
            h = maxSide;
        }
    }*/

    if(bWantWidgets)
    {
        // resize canvas to desired size
        QSize sizeBackup=this->size();
        if (100!=scalePercent)
            resize(w,h);

        // create a new "screenshot" HUD camera
        osg::ref_ptr< osg::CCaptureCamera > p_Camera = new osg::CCaptureCamera();
        osg::ref_ptr< osg::Node >   scene = m_view->getSceneData();

        // setup camera
        p_Camera->setProjectionMatrixAsOrtho2D(0,w,0,h);
        p_Camera->setViewMatrix(osg::Matrix::identity());
        p_Camera->setReferenceFrame( osg::Transform::ABSOLUTE_RF );
        p_Camera->setViewport( 0, 0, w, h );
        p_Camera->setRenderOrder(osg::Camera::POST_RENDER);
        p_Camera->setClearMask(GL_DEPTH_BUFFER_BIT);
        p_Camera->setAllowEventFocus(false);

        // setup post draw callback for screenshot capture
        osg::ref_ptr< osg::CScreenshotCapture > p_Capture = new osg::CScreenshotCapture();
        p_Capture->enable(true);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
        p_Camera->setFinalDrawCallback(p_Capture.get());
        p_Camera->setPostDrawCallback(new UnBindFboPostDrawCallback);
#else
        p_Camera->setPostDrawCallback(p_Capture.get());
#endif

        // set camera as scene child
        scene->asGroup()->addChild(p_Camera);
        p_Camera->setMode(osg::CCaptureCamera::MODE_SINGLE);

        // redraw the scene
        makeCurrent();
        m_view->frame();
        doneCurrent();

        // get screenshot
        p_Capture->getImage(img);

        // remove camera from the scene
        scene->asGroup()->removeChild(p_Camera);

        // resize the scene back to its original size
        if (100!=scalePercent)
            resize(sizeBackup);
        return;
    }

    // allocate image
    p_Image->allocateImage( w, h, 24, GL_RGB, GL_UNSIGNED_BYTE );
    p_Image->setPixelFormat( GL_RGB );

    // create a new camera and attach the image as color buffer
    osg::ref_ptr< osg::Camera > p_Cam = new osg::Camera();
    p_Cam->setProjectionMatrix( p_OrigCam->getProjectionMatrix() );
    p_Cam->setViewMatrix( p_OrigCam->getViewMatrix() );
    p_Cam->setReferenceFrame( osg::Transform::ABSOLUTE_RF );
    p_Cam->setViewport( 0, 0, w, h );
    p_Cam->setRenderOrder( osg::Camera::PRE_RENDER );
    p_Cam->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT );
    p_Cam->attach( osg::Camera::COLOR_BUFFER, p_Image.get() );

    // set new custom camera to view
    osg::ref_ptr< osg::Node >   scene = m_view->getSceneData();
    p_Cam->addChild( scene.get() );
    m_view->setSceneData( p_Cam.get() );

    // redraw scene
    makeCurrent();
    m_view->frame();
    doneCurrent();

    p_Cam->setViewport( p_OrigView.get() );

    // restore settings and redraw scene
    m_view->setSceneData( scene.get() );
    m_view->getCameraManipulator()->setByMatrix( m );
    makeCurrent();
    m_view->frame();
    doneCurrent();
}

////////////////////////////////////////////////////////////
// adopted from scene::CBDRulerWidget
void QGLOSGWidget::getDistances(float & dx, float & dy)
{
    osg::ref_ptr< osg::Camera > p_OrigCam = m_view->getCamera();
    osg::ref_ptr< osg::Viewport > viewport = p_OrigCam->getViewport();

    // Compute transformation matrix
    osg::Matrixd ipm, pm( m_view->getCamera()->getProjectionMatrix() );
    ipm.invert(pm);

    osg::Matrixd iwm, wm( viewport->computeWindowMatrix() );
    iwm.invert(wm);

    /*
        This is really ugly and dirty hack. Update traversal is called BEFORE
        the viewer manipulator updates the camera view matrix. So if we use
        current view matrix, we are using one step backward matrix in fact.
        This hack gets around this harsh reality.
    */
    osg::Matrixd ivm, vm( m_view->getCameraManipulator()->getMatrix() );
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

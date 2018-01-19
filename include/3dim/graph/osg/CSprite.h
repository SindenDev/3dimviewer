///////////////////////////////////////////////////////////////////////////////
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

#ifndef CSprite_H_INCLUDED
#define CSprite_H_INCLUDED

///////////////////////////////////////////////////////////////////////////////
// include files
#include <base/Macros.h>

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osg/Depth>
#include <osg/BlendFunc>
#include <osg/CForceCullCallback.h>
#include <osg/Transform>

namespace osg
{

//! Class for ensuring that sprite is always rendered
class CPreventCullCallback : public osg::CForceCullCallback
{
public:
    //! Constructor
    CPreventCullCallback()
        : CForceCullCallback(false)
    { }

    //! Culling callback
    virtual bool cull(osg::NodeVisitor *nv, osg::Drawable *drawable, osg::State *state) const
    {
        return CForceCullCallback::cull(nv, drawable, state);
    }
};

//! More advanced sprite node
class CAdvSprite : public Transform
{
protected:
    Vec3d                           _position;
    mutable Quat                    _rotation;
    mutable Vec3d                   _scale;
    mutable bool                    _firstTimeToInitEyePoint;
    mutable osg::Vec3               _previousEyePoint;
    mutable osg::Vec3               _previousLocalUp;
    mutable Viewport::value_type    _previousWidth;
    mutable Viewport::value_type    _previousHeight;
    mutable osg::Matrixd            _previousProjection;
    mutable osg::Vec3d              _previousPosition;
    mutable bool                    _matrixDirty;
    mutable osg::Matrixd            _cachedMatrix;
    mutable osg::Matrixd            _matrix;
    osg::Vec3                       _side;
    bool                            _absolutePosition;
    osg::Vec4                       _color;
    int                             _spriteWidth;
    int                             _spriteHeight;
    osg::Vec2                       _pivotPoint;
    osg::ref_ptr<osg::Geometry>     _quad;

public:
    CAdvSprite(osg::Texture2D *texture, osg::Vec2 pivotPoint = osg::Vec2(0.0f, 0.0f));
    virtual void accept(NodeVisitor &nv);
    void setPosition(const Vec3d &pos);
    const Vec3d &getPosition() const;
    void setColor(const Vec4 &color);
    Vec4 getColor() const;
    virtual bool computeLocalToWorldMatrix(Matrix &matrix, NodeVisitor *nv) const;
    virtual bool computeWorldToLocalMatrix(Matrix &matrix, NodeVisitor *nv) const;
    virtual BoundingSphere computeBound() const;

protected:
    virtual ~CAdvSprite();
    void computeMatrix() const;
    void setRotation(const Quat& quat);
    const Quat &getRotation() const;
    void setScale(double scale);
    void setScale(const Vec3d& scale);
    const Vec3d &getScale() const;
};

//! Class for simple rendering 2D textures on screen
class CSprite : public osg::Geometry
{
    // helper class computing bounding volume
    class CSpriteComputeBoundingBoxCallback : public osg::Drawable::ComputeBoundingBoxCallback
    {
    private:
        osg::BoundingBox m_boundingBox;

    public:
        CSpriteComputeBoundingBoxCallback(CSprite *sprite);
        ~CSpriteComputeBoundingBoxCallback();
        virtual BoundingBox computeBound(const osg::Drawable &drawable) const;
    };

private:
    bool m_absolutePosition;
    osg::ref_ptr<osg::Texture2D> m_texture;
    osg::Vec3 m_position;
    osg::Vec2 m_scale;
    osg::Vec4 m_color;

    osg::ref_ptr<osg::StateSet> m_stateSet;
    osg::ref_ptr<osg::Vec3Array> m_vertices;
    osg::ref_ptr<osg::Vec4Array> m_colors;
    osg::ref_ptr<osg::Vec2Array> m_texCoords;
    osg::ref_ptr<DrawElementsUInt> m_indices;
    osg::ref_ptr<osg::Depth> m_depth;
    osg::ref_ptr<osg::BlendFunc> m_blend;

public:
    //! Ctor - position is given by parents
    CSprite(osg::Texture2D *texture, osg::Vec4 color = osg::Vec4(1.0, 1.0, 1.0, 1.0), osg::Vec2 scale = osg::Vec2(1.0, 1.0), osg::BlendFunc *blendFunc = NULL, osg::Depth *depth = NULL);

    //! Ctor - position in window coordinates is given by user
    CSprite(osg::Texture2D *texture, osg::Vec3 position, bool absolutePosition, osg::Vec4 color = osg::Vec4(1.0, 1.0, 1.0, 1.0), osg::Vec2 scale = osg::Vec2(1.0, 1.0), osg::BlendFunc *blendFunc = NULL, osg::Depth *depth = NULL);

    //! Copy Ctor
    CSprite(const CSprite &sprite);

    //! Dtor
    ~CSprite();

public:
    //! Draw implamentation of sprite
    virtual void drawImplementation(osg::RenderInfo &renderInfo) const;

private:
    //! Helper method for creating geometry and state set
    void createGeometry();
};


// Class for placing sprites into scene graph
class CSpriteNode : public osg::Geode
{
private:
    osg::ref_ptr<osg::CSprite> m_sprite;

public:
    //! Ctor
    CSpriteNode(osg::CSprite *sprite);

    //! Copy Ctor
    CSpriteNode(const CSpriteNode &spriteNode);

    //! Dtor
    ~CSpriteNode();
};

} // namespace osg

#endif // CSprite_H_INCLUDED

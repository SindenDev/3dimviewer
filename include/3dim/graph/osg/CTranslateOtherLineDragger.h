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

#ifndef CTRANSLATEOTHERLINEDRAGGER_H
#define CTRANSLATEOTHERLINEDRAGGER_H

#include <osgManipulator/Translate1DDragger>

namespace osgManipulator
{

///////////////////////////////////////////////////////////////////////////////
//

class CTranslateOtherLineDragger : public Translate1DDragger
{
public:
    //! Constructor - simple
    CTranslateOtherLineDragger()
        : Translate1DDragger()
    {
		setColor(osg::Vec4(1,1,1,1));
    }

    //! Constructor - set translating line. Projection line is the same.
    CTranslateOtherLineDragger(const osg::Vec3 &s, const osg::Vec3 &e)
        : Translate1DDragger(s, e),
          m_movementStartPoint(s),
          m_movementEndPoint(e),
          m_mouseStartPoint(s),
          m_mouseEndPoint(e),
          m_tmpMouseStartPoint(s),
          m_tmpMouseEndPoint(e),
          m_scaleFactor(1.0)
    {		
        // compute zero rotation
       ptRotation.makeRotate(e, e);
       tpRotation.makeRotate(e, e);

	   setColor(osg::Vec4(1,1,1,1));
    }

    //! Constructor - set translation line, set projecting line
    CTranslateOtherLineDragger(const osg::Vec3 &ts, const osg::Vec3 &te, const osg::Vec3 &ps, const osg::Vec3 &pe)
        : Translate1DDragger(ps, pe),
        m_movementStartPoint(ts),
        m_movementEndPoint(te),
        m_mouseStartPoint(ps),
        m_mouseEndPoint(pe),
        m_tmpMouseStartPoint(ps),
        m_tmpMouseEndPoint(pe),
        m_scaleFactor(1.0)
    {
        ptRotation.makeRotate(pe - ps, te - ts);
        tpRotation.makeRotate(te - ts, pe - ps);

		setColor(osg::Vec4(1,1,1,1));
    }

    //! Handle pick events on dragger and generate TranslateInLine commands.
    virtual bool handle(const PointerInfo& pointer, const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

    //! Get projector
    LineProjector & getProjector() { return *_projector; }

    //! Set projector line
    void setProjectorLine(const osg::Vec3 &s, const osg::Vec3 &e)
    {
        m_mouseStartPoint = s;
        m_mouseEndPoint = e;
        m_tmpMouseStartPoint = s;
        m_tmpMouseEndPoint = e;

        _projector->setLine(s, e);
        ptRotation.makeRotate(e - s, m_movementEndPoint - m_movementStartPoint);
        tpRotation.makeRotate(m_movementEndPoint - m_movementStartPoint, e - s);
    }

    osg::Vec3d translation()
    {
        // Get current transformation
        osg::Quat rotation, so;
        osg::Vec3d translation, scale;
        getMatrix().decompose(translation, rotation, scale, so);

        return translation;
    }

    //! Set scale factor
    void setScaleFactor(float sf) {m_scaleFactor = sf;}

    //! Get current scale factor
    float getScaleFactor() const {return m_scaleFactor;}

protected:
    //! Starting point of movement line.
    osg::Vec3 m_movementStartPoint;

    //! End point of movement line
    osg::Vec3 m_movementEndPoint;

    //! Starting point of mouse projection line - initial value.
    osg::Vec3 m_mouseStartPoint;

    //! End point of mouse projection line - initial value.
    osg::Vec3 m_mouseEndPoint;

    //! Starting point of mouse projection line - temporary value.
    osg::Vec3 m_tmpMouseStartPoint;

    //! End point of mouse projection line - temporary value.
    osg::Vec3 m_tmpMouseEndPoint;

    //! Rotation from projection line to the translation line.
    osg::Quat ptRotation;

    //! Rotation from translation line to the projection line.
    osg::Quat tpRotation;

    //! Used scale factor
    float m_scaleFactor;
};


} // namespace osgManipulator

#endif // CTRANSLATEOTHERLINEDRAGGER_H

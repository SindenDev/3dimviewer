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

#ifndef CDRAGGERSCALE_H_included
#define CDRAGGERSCALE_H_included

#include <draggers/CDraggerBase.h>
#include <osgManipulator/Scale1DDragger>
#include <osg/CGeometryGenerator.h>
#include <osg/CModifiedScaleCommand.h>

namespace osg
{
    class CDraggerScaleGeometry : public osg::Group
    {
    public:
        CDraggerScaleGeometry();

        void setOffsets(const osg::Vec3& offsetHead, const osg::Vec3& offsetTail);

        void scale(double scaleFactor);

    protected:
        osg::ref_ptr<osg::MatrixTransform> m_matrixHead;
        osg::ref_ptr<osg::MatrixTransform> m_matrixTail;

        osg::Matrix m_defaultMatrixHead;
        osg::Matrix m_defaultMatrixTail;

        osg::Vec3 m_offsetHead;
        osg::Vec3 m_offsetTail;
    };
}


namespace osgManipulator
{

class CDraggerScale : public CDraggerBase < Scale1DDragger >
{
public:
    CDraggerScale(osg::Node* geometry = new osg::CDraggerScaleGeometry());
    CDraggerScale(CModifiedScaleCommand::EScalingMode mode, osg::Vec3 scaleVector, osg::Node* geometry = new osg::CDraggerScaleGeometry());

    // //////////////////////////////////////////////////////////////
    // Base dragger interface
    /** Handle pick events on dragger and generate TranslateInLine commands. */
    virtual bool handle(const PointerInfo& pointer, const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) override;

    osg::Node* getGeometry();

    //! \fn enableInverseScaling(bool enable)
    //!
    //! \brief          Enables/disables inverse scaling. 
    //! \param  enable  Enable/disable.      
    void enableInverseScaling(bool enable);

    //! \fn inverseScalingEnabled()
    //!
    //! \return     Is inverse scaling enabled?
    bool inverseScalingEnabled();

protected: 
    //! \fn computeScale(const osg::Vec3d & startProjectedPoint, const osg::Vec3d & projectedPoint, double scaleCenter)
    //!
    //! \brief                          Computes scale from start point and current projected point. 
    //!                                 The scale is computed as fraction: (distance from start to center) / (distance from projected point to center)
    //!
    //! \param  startProjectedPoint     Scale start point.
    //! \param  projectedPoint          Current projected point.
    //! \param  scaleCenter             Scale center, usually 0.
    double computeScale(const osg::Vec3d & startProjectedPoint, const osg::Vec3d & projectedPoint, double scaleCenter);

protected:

    //! Scaling vector
    osg::Vec3 m_scaleVector;

    //! \brief Indicateor wether inverse scaling is enabled or disabled. Enabled by default.
    bool m_inverseScalingEnabled;

    //! \brief Scale mode. See CModifiedScaleCommand::EScalingMode.
    CModifiedScaleCommand::EScalingMode m_mode;

    //! \brief Reference point, corresponding to start point multiplied by scale vector.
    double m_referencePoint;

    osg::ref_ptr<osg::Node> m_geometry;
};

} //namespace


#endif //CDRAGGERSCALE_H_included
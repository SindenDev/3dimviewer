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

#include <osg/CModifiedScaleCommand.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   Gets the motion matrix.
//!
//!\return  The motion matrix.
////////////////////////////////////////////////////////////////////////////////////////////////////
osg::Matrix osgManipulator::CModifiedScaleCommand::getMotionMatrix() const
{
    osg::Vec3 s(    fabs( m_smx ) > m_minScale[0] ? m_smx * m_scale : 1.0,
                    fabs( m_smy ) > m_minScale[1] ? m_smy * m_scale : 1.0,
                    fabs( m_smz ) > m_minScale[2] ? m_smz * m_scale : 1.0 );

    return (osg::Matrix::translate(-m_scaleCenter) 
        * osg::Matrix::scale(s)
        * osg::Matrix::translate(m_scaleCenter));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   Sets a scaling mode.
//!
//!\param   mode    The mode.
////////////////////////////////////////////////////////////////////////////////////////////////////
void osgManipulator::CModifiedScaleCommand::setScalingMode( EScalingMode mode )
{
    switch( mode )
    {
    case SCALE_X:
        m_smx = 1.0; m_smy = m_smz = 0.0;
        break;

    case SCALE_Y:
        m_smy = 1.0; m_smx = m_smz = 0.0;
        break;

    case SCALE_Z:
        m_smz = 1.0; m_smx = m_smy = 0.0;
        break;

    case SCALE_XY:
        m_smx = m_smy = 1.0; m_smz = 0.0;
        break;

    case SCALE_XZ:
        m_smx = m_smz = 1.0; m_smy = 0.0;
        break;

    case SCALE_YZ:
        m_smz = m_smy = 1.0; m_smx = 0.0;
        break;

    case SCALE_UNIFORM:
        m_smx = m_smy = m_smz = 1.0;
    }

    m_mode = mode;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   Creates the command inverse.
//!
//!\return  null if it fails, else.
////////////////////////////////////////////////////////////////////////////////////////////////////
osgManipulator::MotionCommand* osgManipulator::CModifiedScaleCommand::createCommandInverse()
{
    osg::ref_ptr<CModifiedScaleCommand> inverse = new CModifiedScaleCommand();
    *inverse = *this;
    if (m_scale) inverse->setScale(1.0/m_scale);

    return inverse.release();
}

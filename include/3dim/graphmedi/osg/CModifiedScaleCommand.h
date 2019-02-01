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

#pragma once
#ifndef CCModifiedScaleCommand_H_included
#define CCModifiedScaleCommand_H_included

#include <osgManipulator/Command>

namespace osgManipulator
{
    /**
    * Command for configurable uniform 3D scaling.
    */
    class CModifiedScaleCommand : public Scale1DCommand
    {
    public:
        //! Scaling mode
        enum EScalingMode
        {
            SCALE_X,
            SCALE_Y,
            SCALE_Z,
            SCALE_XY,
            SCALE_XZ,
            SCALE_YZ,
            SCALE_UNIFORM
        }; // enum EScalingMode

    public:
        //! Constructor
        CModifiedScaleCommand()
            : m_smx( 1.0 )
            , m_smy( 1.0 )
            , m_smz( 1.0 )
            , m_scale( 0 )
            , m_minScale( osg::Vec3d( 0.001, 0.001, 0.00 ) )
        { }

        //! Constructor with mode set
        CModifiedScaleCommand( EScalingMode mode )
        {
            setScalingMode(mode);
        }
        
        //! Get motion matrix
        virtual osg::Matrix getMotionMatrix() const;

        virtual MotionCommand* createCommandInverse();

        //! Scale amount access methods
        inline void setScale(double s) { m_scale = s; }
        inline double getScale() const { return m_scale; }

        //! Scale center access methods
        inline void setScaleCenter(const osg::Vec3d& center) { m_scaleCenter = center; }
        inline const osg::Vec3d& getScaleCenter() const { return m_scaleCenter; }

        //! Set scaling multipliers
        void setMultipliers( double mx, double my, double mz ) 
        {
            m_smx = mx; m_smy = my; m_smz = mz; 
        }

        //! Set scaling multipliers by mode
        void setScalingMode( EScalingMode mode );

        //! Returns scaling mode
        EScalingMode getScalingMode() const { return m_mode; }
    private:
        //! Scale amount
        double m_scale;

        //! Center of scale
        osg::Vec3d m_scaleCenter;

        //! Scaling multipliers
        double m_smx, m_smy, m_smz;

        //! 1D scaling parameters
        osg::Vec3d m_minScale;

        //! Scaling mode
        EScalingMode m_mode;

    };
} // namespace

#endif


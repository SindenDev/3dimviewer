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

#ifndef CLineOptimizer_H
#define CLineOptimizer_H

#include <osg/Array>

namespace draw
{

///////////////////////////////////////////////////////////////////////////////
//! CLASS CLineOptimizer - removes duplicities and not needed points

class CLineOptimizer
{
public:
    //! Optimize 2D line
    void Optimize( const osg::Vec2Array * input, osg::Vec2Array * output );

    //! Optimize 2D line
    void Optimize( const osg::Vec3Array * input, osg::Vec3Array * output );

protected:
    //! Remove duplicities in 2D array
    void RemoveDuplicities( const osg::Vec2Array * input, osg::Vec2Array * output );

    //! Remove collinear segments in 2D
    void RemoveCollinear( const osg::Vec2Array * input, osg::Vec2Array * output );

    //! Remove duplicities in 3D array
    void RemoveDuplicities( const osg::Vec3Array * input, osg::Vec3Array * output );

    //! Remove collinear segments in 3D
    void RemoveCollinear( const osg::Vec3Array * input, osg::Vec3Array * output );

}; // class CLineOptimizer


} // namespace draw

#endif // CLineOptimizer_H

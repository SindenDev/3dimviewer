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

#ifndef CColorGenerator_H_included
#define CColorGenerator_H_included

#include <osg/Array>

namespace osg
{

//! Color generator class. There are 25 predefined different colors, next colors are randomly generated.
class CColorGenerator
{
public:
	//! Constructor
	CColorGenerator()
	{
		m_colors = new osg::Vec3Array;
		fillColorsVector();
	}

	//! Destructor
	~CColorGenerator()
	{ }

	//! Returns color on given index.
	const osg::Vec3 getColor(int index)
	{
		return (index >= 0 && index < m_colors->size()) ? m_colors->at(index) : generateNewColor();
	}

	//! Returns color on given index with given alpha value.
	const osg::Vec4 getColor(int index, float alpha)
	{
		osg::Vec3 c = (index >= 0 && index < m_colors->size()) ? m_colors->at(index) : generateNewColor();
		return osg::Vec4(c[0], c[1], c[2], alpha);
	}

protected:

	//! Fills vector with predefined colors.
	void fillColorsVector()
	{
		m_colors->push_back(osg::Vec3(1.0, 1.0, 0.0));
		m_colors->push_back(osg::Vec3(1.0, 0.0, 1.0));
		m_colors->push_back(osg::Vec3(0.0, 1.0, 1.0));
		//m_colors->push_back(osg::Vec3(1.0, 0.5, 0.0));
		m_colors->push_back(osg::Vec3(0.3, 0.5, 0.2));
		m_colors->push_back(osg::Vec3(0.7, 0.4, 0.5));
		m_colors->push_back(osg::Vec3(0.3, 0.6, 0.8));
		m_colors->push_back(osg::Vec3(1.0, 0.9, 0.7));
		m_colors->push_back(osg::Vec3(0.5, 0.2, 0.2));
		m_colors->push_back(osg::Vec3(0.8, 0.7, 0.2));
		m_colors->push_back(osg::Vec3(0.6, 0.2, 0.9));		
		m_colors->push_back(osg::Vec3(0.7, 0.8, 0.7));
		m_colors->push_back(osg::Vec3(0.6, 0.5, 0.3));
		//m_colors->push_back(osg::Vec3(0.0, 0.8, 0.6));
		m_colors->push_back(osg::Vec3(1.0, 0.7, 0.9));		
		m_colors->push_back(osg::Vec3(0.3, 0.3, 0.3));
		m_colors->push_back(osg::Vec3(0.8, 0.1, 0.4));
		m_colors->push_back(osg::Vec3(0.8, 1.0, 0.0));
		m_colors->push_back(osg::Vec3(1.0, 0.5, 0.5));
		m_colors->push_back(osg::Vec3(1.0, 0.8, 0.0));
		m_colors->push_back(osg::Vec3(0.8, 0.6, 1.0));
		m_colors->push_back(osg::Vec3(0.5, 0.5, 0.6));
        m_colors->push_back(osg::Vec3(1.0, 0.0, 0.0));
        //m_colors->push_back(osg::Vec3(0.0, 1.0, 0.0));
        //m_colors->push_back(osg::Vec3(0.0, 0.0, 1.0));
	}

	//! Randomly generates new color.
	osg::Vec3 generateNewColor()
	{
		return osg::Vec3(0.1 + 0.9 * rand() / (double)RAND_MAX, 0.1 + 0.9 * rand() / (double)RAND_MAX, 0.1 + 0.9 * rand() / (double)RAND_MAX);
	}

	//! Vector of colors.
	osg::ref_ptr< osg::Vec3Array > m_colors;
};

}

// CColorGenerator_H_included
#endif
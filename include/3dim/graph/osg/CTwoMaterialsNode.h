///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// 3DimViewer
// Lightweight 3D DICOM viewer.
//
// Copyright 2008-2012 3Dim Laboratory s.r.o.
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

#ifndef CTWOMATERIALSNODE_H_INCLUDED
#define CTWOMATERIALSNODE_H_INCLUDED

#include <osg/Material>
#include <osg/StateSet>
#include <osg/ShadeModel>

namespace osg
{
	//! Material number
	enum EMaterialNumber
	{
		FIRST = 0,
		SECOND = 1
	};

	template < class tpNodeType >
	class CTwoMaterialsNode : public tpNodeType
	{
	public:
		//! Constructor - create and set defaults
		CTwoMaterialsNode();

		//! Virtual destructor
        virtual ~CTwoMaterialsNode() {}

        EMaterialNumber getCurrentMaterial() { return m_currentMaterial; }

		//! Get material reference
		osg::Material & getMaterial( EMaterialNumber m ) { return *m_material[m]; }

		//! Set current material
		void setMaterial( EMaterialNumber m );

		//! Set material colors
		void setColors( EMaterialNumber m, osg::Vec4 difuse, const osg::Vec4 & specular = osg::Vec4( 0.0, 0.0, 0.0, 1.0 ), const osg::Vec4 & ambient = osg::Vec4( 0.0, 0.0, 0.0, 1.0 ) );

		//! Get material colors
		void getColors( EMaterialNumber m, osg::Vec4 &difuse, osg::Vec4 & specular, osg::Vec4 & ambient );

        //! Get material difuse color
        osg::Vec4 getColor( EMaterialNumber m );

        //! Enable/disable lighting
        void enableLighting(bool bEnable);

	protected:
        EMaterialNumber m_currentMaterial;
        osg::ref_ptr< osg::Material > m_material[2];
        bool    m_bLighting;
	}; // template class CTwoMaterialsNode

	////////////////////////////////////////////////////////////////////////////////////////////////////
	//!\brief	Sets the colors. 
	//!
	//!\typeparam	tpNodeType	. 
	//!\param	m			The. 
	//!\param	difuse	The difuse. 
	//!\param	specular	The specular. 
	//!\param	ambient	The ambient. 
	////////////////////////////////////////////////////////////////////////////////////////////////////
	template < class tpNodeType >
	void osg::CTwoMaterialsNode<tpNodeType>::setColors( EMaterialNumber m, osg::Vec4 difuse, const osg::Vec4 & specular /*= osg::Vec4( 0.0, 0.0, 0.0, 1.0 )*/, const osg::Vec4 & ambient /*= osg::Vec4( 0.0, 0.0, 0.0, 1.0 ) */ )
	{
		m_material[ m ]->setDiffuse( osg::Material::FRONT_AND_BACK, difuse );
		m_material[ m ]->setSpecular( osg::Material::FRONT_AND_BACK, specular );
		m_material[ m ]->setAmbient( osg::Material::FRONT_AND_BACK, ambient );
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	//!\brief	Gets the color. 
	//!
	//!\typeparam	tpNodeType	. 
	//!\param	m			The. 
	//!\param	difuse	The difuse. 
	//!\param	specular	The specular. 
	//!\param	ambient	The ambient. 
	////////////////////////////////////////////////////////////////////////////////////////////////////
	template < class tpNodeType >
	void osg::CTwoMaterialsNode<tpNodeType>::getColors( EMaterialNumber m, osg::Vec4 & difuse, osg::Vec4 & specular, osg::Vec4 & ambient )
	{
		difuse = m_material[ m ]->getDiffuse( osg::Material::FRONT_AND_BACK );
		specular = m_material[ m ]->getSpecular( osg::Material::FRONT_AND_BACK );
		ambient = m_material[ m ]->getAmbient( osg::Material::FRONT_AND_BACK );
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	//!\brief	Gets the color. 
	//!
	//!\typeparam	tpNodeType	. 
	//!\param	m			The. 
	//!\param	difuse	The difuse. 
	////////////////////////////////////////////////////////////////////////////////////////////////////
	template < class tpNodeType >
	osg::Vec4 osg::CTwoMaterialsNode<tpNodeType>::getColor( EMaterialNumber m )
	{
		return m_material[ m ]->getDiffuse( osg::Material::FRONT_AND_BACK );
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	//!\brief	Sets a material. 
	//!
	//!\typeparam	tpNodeType	. 
	//!\param	m	The. 
	////////////////////////////////////////////////////////////////////////////////////////////////////
	template < class tpNodeType >
	void osg::CTwoMaterialsNode<tpNodeType>::setMaterial( EMaterialNumber m )
	{
        m_currentMaterial = m;

		osg::StateSet * ss( this->getOrCreateStateSet() );
		ss->setAttributeAndModes(m_material[m],osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);

		// Set shading model to flat
		osg::ref_ptr<osg::ShadeModel> shadeModel = new osg::ShadeModel(osg::ShadeModel::FLAT); 
		ss->setAttributeAndModes(shadeModel, osg::StateAttribute::ON );

		// Enable lighting
        if (m_bLighting)
            ss->setMode(GL_LIGHTING,osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);
        else
            ss->setMode(GL_LIGHTING,osg::StateAttribute::OVERRIDE|osg::StateAttribute::OFF);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	//!\brief	Enable/disable lighting
	////////////////////////////////////////////////////////////////////////////////////////////////////
	template < class tpNodeType >
	void osg::CTwoMaterialsNode<tpNodeType>::enableLighting(bool bEnable)
    {
        m_bLighting = bEnable;
        osg::StateSet * ss( this->getOrCreateStateSet() );
        if (m_bLighting)
            ss->setMode(GL_LIGHTING,osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);
        else
            ss->setMode(GL_LIGHTING,osg::StateAttribute::OVERRIDE|osg::StateAttribute::OFF);
    }

	////////////////////////////////////////////////////////////////////////////////////////////////////
	//!\brief	Two materials node. 
	//!
	//!\typeparam	tpNodeType	. 
	//!
	//!\return	. 
	////////////////////////////////////////////////////////////////////////////////////////////////////
	template < class tpNodeType >
	osg::CTwoMaterialsNode<tpNodeType>::CTwoMaterialsNode()
	{
        m_bLighting = true;

		for( int i = 0; i < 2; ++i )
		{
			m_material[ i ] = new osg::Material;
			m_material[ i ]->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4( 0.5, 0.5, 0.5, 1.0 ));
			m_material[ i ]->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4( 0.9, 0.9, 0.9, 1.0 ));
			m_material[ i ]->setAmbient( osg::Material::FRONT_AND_BACK, osg::Vec4( 0.1, 0.1, 0.1, 1.0 ));
		}

		setMaterial( FIRST );
	}

}

// CTWOMATERIALSNODE_H_INCLUDED
#endif


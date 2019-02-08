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

#ifndef CTWOMATERIALSNODE_H_INCLUDED
#define CTWOMATERIALSNODE_H_INCLUDED

#include <osg/CPseudoMaterial.h>
#include <osg/Node>
#include <osg/CPseudoMaterialCache.h>

#define USE_PSEUDOMATERIALS_CACHE

namespace osg
{
    enum MaterialNumber
    {
        FIRST = 0,
        SECOND = 1
    };

    template<class Node>
    class CTwoMaterialsNode : public Node
    {
    public:
        CTwoMaterialsNode(bool autoApply = true);

        virtual void applyMaterial(MaterialNumber m);

        void setMaterial(MaterialNumber m, osg::CPseudoMaterial* material);
        osg::CPseudoMaterial* getMaterial(MaterialNumber m);

        virtual void setDiffuse(MaterialNumber m, const osg::Vec3& diffuse);
        virtual void setEmission(MaterialNumber m, const osg::Vec3& emission);
        virtual void setSpecularity(MaterialNumber m, float specularity);
        virtual void setShininess(MaterialNumber m, float shininess);

        osg::Vec3 getDiffuse(MaterialNumber m);
        osg::Vec3 getEmission(MaterialNumber m);
        float getSpecularity(MaterialNumber m);
        float getShininess(MaterialNumber m);

    protected:
        MaterialNumber m_currentMaterial = FIRST;

        osg::ref_ptr<osg::CPseudoMaterial> m_material[2];
    };


    template<class Node>
    void osg::CTwoMaterialsNode<Node>::setDiffuse(MaterialNumber m, const osg::Vec3& diffuse)
    {
#ifdef USE_PSEUDOMATERIALS_CACHE
        const CPseudoMaterial* prevMaterial = m_material[m];
        m_material[m] = CPseudoMaterialCache::getMaterialCache()->getAdjustedMaterialDiffuse(m_material[m], diffuse);
        if (m_material[m] != prevMaterial && m == m_currentMaterial) // auto apply material when using a new object
            applyMaterial(m);
#else
        m_material[m]->uniform("Diffuse")->set(diffuse);
#endif
    }

    template<class Node>
    void osg::CTwoMaterialsNode<Node>::setEmission(MaterialNumber m, const osg::Vec3& emission)
    {
#ifdef USE_PSEUDOMATERIALS_CACHE
        const CPseudoMaterial* prevMaterial = m_material[m];
        m_material[m] = CPseudoMaterialCache::getMaterialCache()->getAdjustedMaterialEmission(m_material[m], emission);
        if (m_material[m] != prevMaterial && m == m_currentMaterial) // auto apply material when using a new object
            applyMaterial(m);
#else
        m_material[m]->uniform("Emission")->set(emission);
#endif
    }

    template<class Node>
    void osg::CTwoMaterialsNode<Node>::setSpecularity(MaterialNumber m, float specularity)
    {
#ifdef USE_PSEUDOMATERIALS_CACHE
        const CPseudoMaterial* prevMaterial = m_material[m];
        m_material[m] = CPseudoMaterialCache::getMaterialCache()->getAdjustedMaterialSpecularity(m_material[m], specularity);
        if (m_material[m] != prevMaterial && m == m_currentMaterial) // auto apply material when using a new object
            applyMaterial(m);
#else
        m_material[m]->uniform("Specularity")->set(specularity);
#endif
    }

    template<class Node>
    void osg::CTwoMaterialsNode<Node>::setShininess(MaterialNumber m, float shininess)
    {
#ifdef USE_PSEUDOMATERIALS_CACHE
        const CPseudoMaterial* prevMaterial = m_material[m];
        m_material[m] = CPseudoMaterialCache::getMaterialCache()->getAdjustedMaterialShininess(m_material[m], shininess);
        if (m_material[m] != prevMaterial && m == m_currentMaterial) // auto apply material when using a new object
            applyMaterial(m);
#else
        m_material[m]->uniform("Shininess")->set(shininess);
#endif
    }

    template<class Node>
    osg::Vec3 osg::CTwoMaterialsNode<Node>::getDiffuse(MaterialNumber m)
    {
        osg::Vec3 diffuse;
        m_material[m]->uniform("Diffuse")->get(diffuse);

        return diffuse;
    }

    template<class Node>
    osg::Vec3 osg::CTwoMaterialsNode<Node>::getEmission(MaterialNumber m)
    {
        osg::Vec3 emission;
        m_material[m]->uniform("Emission")->get(emission);

        return emission;
    }

    template<class Node>
    float osg::CTwoMaterialsNode<Node>::getSpecularity(MaterialNumber m)
    {
        float specularity;
        m_material[m]->uniform("Specularity")->get(specularity);

        return specularity;
    }

    template<class Node>
    float osg::CTwoMaterialsNode<Node>::getShininess(MaterialNumber m)
    {
        float shininess;
        m_material[m]->uniform("Shininess")->get(shininess);

        return shininess;
    }

    template<class Node>
    void osg::CTwoMaterialsNode<Node>::applyMaterial(MaterialNumber m)
    {
        m_currentMaterial = m;

        m_material[m]->apply(this, osg::StateAttribute::ON);
    }

    template<class Node>
    void osg::CTwoMaterialsNode<Node>::setMaterial(MaterialNumber m, osg::CPseudoMaterial* material)
    {
        m_material[m] = material;
    }

    template<class Node>
    osg::CPseudoMaterial* osg::CTwoMaterialsNode<Node>::getMaterial(MaterialNumber m)
    {
#ifdef USE_PSEUDOMATERIALS_CACHE
        if (CPseudoMaterialCache::getMaterialCache()->isInCache(m_material[m]))
        {
            // can't change material which is in cache - make an uncached copy of the material
            m_material[m] = CPseudoMaterialCache::getMaterialCache()->getMaterialCopy(m_material[m]);
        }
#endif
        return m_material[m];
    }    

    template<class Node>
    osg::CTwoMaterialsNode<Node>::CTwoMaterialsNode(bool autoApply/* = true*/)
    {
        osg::Node::setName("CTwoMaterialsNode");

        for (int i = 0; i < 2; ++i)
        {
#ifdef USE_PSEUDOMATERIALS_CACHE
            m_material[i] = CPseudoMaterialCache::getMaterialCache()->getCachedMaterial(osg::Vec3(1.0f, 1.0f, 1.0f), osg::Vec3(0.1f, 0.1f, 0.1f), 4.0f, 0.125f, true);
#else
            m_material[i] = new osg::CPseudoMaterial();

            m_material[i]->uniform("Diffuse")->set(osg::Vec3(1.0f, 1.0f, 1.0f));
            m_material[i]->uniform("Emission")->set(osg::Vec3(0.1f, 0.1f, 0.1f));
            m_material[i]->uniform("Shininess")->set(4.0f);
            m_material[i]->uniform("Specularity")->set(0.125f);
#endif
        }

        if (autoApply)
        {
            applyMaterial(osg::FIRST);
        }
    }
}

// CTWOMATERIALSNODE_H_INCLUDED
#endif


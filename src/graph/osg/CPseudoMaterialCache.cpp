///////////////////////////////////////////////////////////////////////////////
//
// 3DimViewer
// Lightweight 3D DICOM viewer.
//
// Copyright 2018 TESCAN 3DIM, s.r.o.
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

#include <osg/CPseudoMaterialCache.h>

namespace osg
{
    // init static member
    CPseudoMaterialCache* CPseudoMaterialCache::m_pMaterialCache = nullptr;

    // Pseudo material cache shared by all TwoMaterialsNode instances, for simple materials only
    CPseudoMaterialCache g_pseudoMaterialCache;

    CPseudoMaterialCache::CPseudoMaterialCache()
    {
        m_pMaterialCache = this;
    }

    CPseudoMaterialCache::~CPseudoMaterialCache()
    {
#ifdef _WIN32
        std::stringstream ss;
        ss << m_materials.size() << " pseudo-materials were cached" << std::endl;
        OutputDebugStringA(ss.str().c_str());
#endif
    }

    CPseudoMaterial* CPseudoMaterialCache::getCachedMaterial(const osg::Vec3 &diffuse, const osg::Vec3 &emission, float shininess, float specularity, bool bAddOnNoHit)
    {
        for (osg::ref_ptr<CPseudoMaterial>& i : m_materials)
        {
            if (isSame(i.get(), diffuse, emission, shininess, specularity))
                return i.get();
        }
        if (bAddOnNoHit)
        {
            CPseudoMaterial* pMaterial = new osg::CPseudoMaterial();
            pMaterial->uniform("Diffuse")->set(diffuse);
            pMaterial->uniform("Emission")->set(emission);
            pMaterial->uniform("Shininess")->set(shininess);
            pMaterial->uniform("Specularity")->set(specularity);
            m_materials.push_back(pMaterial);
            return pMaterial;
        }
        return nullptr;
    }

    CPseudoMaterial* CPseudoMaterialCache::getAdjustedMaterial(const osg::Vec3 &diffuse, const osg::Vec3 &emission, float shininess, float specularity, bool bAddToCacheOnNoHit)
    {
        CPseudoMaterial *pMaterial = getCachedMaterial(diffuse, emission, shininess, specularity, bAddToCacheOnNoHit);
        if (nullptr == pMaterial)
        {
            assert(!bAddToCacheOnNoHit); // wasn't in cache and could not be added
            pMaterial = new osg::CPseudoMaterial();
            pMaterial->uniform("Diffuse")->set(diffuse);
            pMaterial->uniform("Emission")->set(emission);
            pMaterial->uniform("Shininess")->set(shininess);
            pMaterial->uniform("Specularity")->set(specularity);
            if (bAddToCacheOnNoHit)
                m_materials.push_back(pMaterial);
        }
        return pMaterial;
    }

    CPseudoMaterial* CPseudoMaterialCache::getAdjustedMaterialDiffuse(CPseudoMaterial* pPseudoMaterial, const osg::Vec3 &diffuse)
    {
        if (!isInCache(pPseudoMaterial))
        {
            pPseudoMaterial->uniform("Diffuse")->set(diffuse);
            return pPseudoMaterial;
        }

        if (typeid(*pPseudoMaterial) != typeid(CPseudoMaterial)) // no support for subclassed materials
        {
            assert(false);
            return nullptr;
        }

        osg::Vec3 emission;
        float specularity = 0, shininess = 0;
        pPseudoMaterial->uniform("Emission")->get(emission);
        pPseudoMaterial->uniform("Specularity")->get(specularity);
        pPseudoMaterial->uniform("Shininess")->get(shininess);
        return getAdjustedMaterial(diffuse, emission, shininess, specularity, true);
    }

    CPseudoMaterial* CPseudoMaterialCache::getAdjustedMaterialEmission(CPseudoMaterial* pPseudoMaterial, const osg::Vec3 &emission)
    {
        if (!isInCache(pPseudoMaterial))
        {
            pPseudoMaterial->uniform("Emission")->set(emission);
            return pPseudoMaterial;
        }

        if (typeid(*pPseudoMaterial) != typeid(CPseudoMaterial)) // no support for subclassed materials
        {
            assert(false);
            return nullptr;
        }

        osg::Vec3 diffuse;
        float specularity = 0, shininess = 0;
        pPseudoMaterial->uniform("Diffuse")->get(diffuse);
        pPseudoMaterial->uniform("Specularity")->get(specularity);
        pPseudoMaterial->uniform("Shininess")->get(shininess);
        return getAdjustedMaterial(diffuse, emission, shininess, specularity, true);
    }

    CPseudoMaterial* CPseudoMaterialCache::getAdjustedMaterialShininess(CPseudoMaterial* pPseudoMaterial, float shininess)
    {
        if (!isInCache(pPseudoMaterial))
        {
            pPseudoMaterial->uniform("Shininess")->set(shininess);
            return pPseudoMaterial;
        }

        if (typeid(*pPseudoMaterial) != typeid(CPseudoMaterial)) // no support for subclassed materials
        {
            assert(false);
            return nullptr;
        }

        osg::Vec3 diffuse, emission;
        float specularity = 0;
        pPseudoMaterial->uniform("Diffuse")->get(diffuse);
        pPseudoMaterial->uniform("Emission")->get(emission);
        pPseudoMaterial->uniform("Specularity")->get(specularity);
        return getAdjustedMaterial(diffuse, emission, shininess, specularity, true);
    }

    CPseudoMaterial* CPseudoMaterialCache::getAdjustedMaterialSpecularity(CPseudoMaterial* pPseudoMaterial, float specularity)
    {
        if (!isInCache(pPseudoMaterial))
        {
            pPseudoMaterial->uniform("Specularity")->set(specularity);
            return pPseudoMaterial;
        }

        if (typeid(*pPseudoMaterial) != typeid(CPseudoMaterial)) // no support for subclassed materials
        {
            assert(false);
            return nullptr;
        }

        osg::Vec3 diffuse, emission;
        float shininess = 0;
        pPseudoMaterial->uniform("Diffuse")->get(diffuse);
        pPseudoMaterial->uniform("Emission")->get(emission);
        pPseudoMaterial->uniform("Shininess")->get(shininess);
        return getAdjustedMaterial(diffuse, emission, shininess, specularity, true);
    }

    CPseudoMaterial* CPseudoMaterialCache::getMaterialCopy(CPseudoMaterial* pPseudoMaterial)
    {
        if (typeid(*pPseudoMaterial) != typeid(CPseudoMaterial)) // no support for subclassed materials
        {
            assert(false);
            return nullptr;
        }

        osg::Vec3 diffuse, emission;
        float shininess = 0, specularity = 0;
        pPseudoMaterial->uniform("Diffuse")->get(diffuse);
        pPseudoMaterial->uniform("Emission")->get(emission);
        pPseudoMaterial->uniform("Shininess")->get(shininess);
        pPseudoMaterial->uniform("Specularity")->get(specularity);

        CPseudoMaterial* pMaterial = new osg::CPseudoMaterial();
        pMaterial->uniform("Diffuse")->set(diffuse);
        pMaterial->uniform("Emission")->set(emission);
        pMaterial->uniform("Shininess")->set(shininess);
        pMaterial->uniform("Specularity")->set(specularity);
        return pMaterial;
    }

    bool CPseudoMaterialCache::isSame(CPseudoMaterial* pPseudoMaterial, const osg::Vec3 &diffuse, const osg::Vec3 &emission, float shininess, float specularity) const
    {
        osg::Vec3 mat_diffuse, mat_emission;
        float mat_specularity, mat_shininess;
        pPseudoMaterial->uniform("Diffuse")->get(mat_diffuse);
        pPseudoMaterial->uniform("Emission")->get(mat_emission);
        pPseudoMaterial->uniform("Specularity")->get(mat_specularity);
        pPseudoMaterial->uniform("Shininess")->get(mat_shininess);
        return (specularity == mat_specularity && mat_shininess == shininess &&
                mat_diffuse == diffuse && mat_emission == emission);
    }

    bool CPseudoMaterialCache::isInCache(CPseudoMaterial* pPseudoMaterial)
    {
        for (osg::ref_ptr<CPseudoMaterial>& i : m_materials)
            if (pPseudoMaterial == i.get())
                return true;
        return false;
    }

    CPseudoMaterial* CPseudoMaterialCache::findInCache(const osg::Vec3 &diffuse, const osg::Vec3 &emission, float shininess, float specularity)
    {
        for (osg::ref_ptr<CPseudoMaterial>& i : m_materials)
        {
            if (isSame(i.get(),diffuse,emission,shininess,specularity))
                return i.get();
        }
        return nullptr;
    }

    bool CPseudoMaterialCache::addToCache(CPseudoMaterial* pPseudoMaterial)
    {
        if (typeid(*pPseudoMaterial) != typeid(CPseudoMaterial)) // no support for subclassed materials
        {
            assert(false);
            return false;
        }
        if (!isInCache(pPseudoMaterial))
            m_materials.push_back(pPseudoMaterial);
        return true;
    }
}
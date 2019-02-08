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

#ifndef CPSEUDOMATERIALCACHE_H_INCLUDED
#define CPSEUDOMATERIALCACHE_H_INCLUDED

#include <osg/CPseudoMaterial.h>
#include <osg/Node>
#include <osg/Vec3>
#include <list>

namespace osg
{
    //! Cache for pseudo materials used in draggers via CTwoMaterialsNode, they use only 4 basic settings, it won't work correctly for advanced materials
    //! All materials are cached till the end of object existence, so use it only for frequently used materials
    class CPseudoMaterialCache
    {
    protected:
        // use map for faster lookup? there are about 20 cached materials at the moment of implementation
        std::list<osg::ref_ptr<CPseudoMaterial> > m_materials;
        //! Cache singleton
        static CPseudoMaterialCache* m_pMaterialCache;
    public:
        //! Singleton access method
        static CPseudoMaterialCache* getMaterialCache() { return m_pMaterialCache; }

        //! Constructor
        CPseudoMaterialCache();

        //! Destructor
        ~CPseudoMaterialCache();

        //! Compare parameters of given material with provided values
        bool isSame(CPseudoMaterial* pPseudoMaterial, const osg::Vec3 &diffuse, const osg::Vec3 &emission, float shininess, float specularity) const;

        //! Get material from cache matching the parameters, create new on no hit and add it on cache when bAddOnNoHit is set
        CPseudoMaterial* getCachedMaterial(const osg::Vec3 &diffuse, const osg::Vec3 &emission, float shininess, float specularity, bool bAddOnNoHit);

        //! Get material from cache matching the parameters, create new on no hit and add it on cache when bAddOnNoHit is set
        CPseudoMaterial* getAdjustedMaterial(const osg::Vec3 &diffuse, const osg::Vec3 &emission, float shininess, float specularity, bool bAddOnNoHit);

        //! Get material from cache with adjusted diffuse, create new on no hit and add it on cache
        CPseudoMaterial* getAdjustedMaterialDiffuse(CPseudoMaterial* pPseudoMaterial, const osg::Vec3 &diffuse);

        //! Get material from cache with adjusted emission, create new on no hit and add it on cache
        CPseudoMaterial* getAdjustedMaterialEmission(CPseudoMaterial* pPseudoMaterial, const osg::Vec3 &emission);

        //! Get material from cache with adjusted shininess, create new on no hit and add it on cache
        CPseudoMaterial* getAdjustedMaterialShininess(CPseudoMaterial* pPseudoMaterial, float shininess);

        //! Get material from cache with adjusted specularity, create new on no hit and add it on cache
        CPseudoMaterial* getAdjustedMaterialSpecularity(CPseudoMaterial* pPseudoMaterial, float specularity);

        //! Get an uncached material copy
        CPseudoMaterial* getMaterialCopy(CPseudoMaterial* pPseudoMaterial);

        //! Check whether the material is in cache (this exact copy, do not check params)
        bool isInCache(CPseudoMaterial* pPseudoMaterial);

        //! Check whether a material with given params is in cache
        CPseudoMaterial* findInCache(const osg::Vec3 &diffuse, const osg::Vec3 &emission, float shininess, float specularity);

        //! Add material to cache
        bool addToCache(CPseudoMaterial* pPseudoMaterial);
    };
}

// CPSEUDOMATERIALCACHE_H_INCLUDED
#endif


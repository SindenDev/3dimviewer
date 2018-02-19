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

#ifndef CPseudoMaterial_H
#define CPseudoMaterial_H

#include <osg/StateSet>

namespace osg
{
    class CDirectionalLightSource
    {
    protected:
        osg::Vec3 m_color;
        osg::Vec3 m_direction;

    public:
        CDirectionalLightSource();
        CDirectionalLightSource(osg::Vec3 color, osg::Vec3 direction);
        CDirectionalLightSource(const CDirectionalLightSource &other);
        ~CDirectionalLightSource();
        CDirectionalLightSource &operator=(const CDirectionalLightSource &other);

    public:
        osg::Vec3 &color();
        osg::Vec3 &direction();
    };

    class CPointLightSource
    {
    protected:
        osg::Vec3 m_color;
        osg::Vec3 m_position;
        double m_attenuation;

    public:
        CPointLightSource();
        CPointLightSource(osg::Vec3 color, osg::Vec3 position, double attenuation);
        CPointLightSource(const CPointLightSource &other);
        ~CPointLightSource();
        CPointLightSource &operator=(const CPointLightSource &other);

    public:
        osg::Vec3 &color();
        osg::Vec3 &position();
        double &attenuation();
    };

    class CSpotLightSource
    {
    protected:
        osg::Vec3 m_color;
        osg::Vec3 m_position;
        osg::Vec3 m_direction;
        double m_attenuation;
        double m_focus;
        double m_angle;

    public:
        CSpotLightSource();
        CSpotLightSource(osg::Vec3 color, osg::Vec3 position, osg::Vec3 direction, double attenuation, double focus, double angle);
        CSpotLightSource(const CSpotLightSource &other);
        ~CSpotLightSource();
        CSpotLightSource &operator=(const CSpotLightSource &other);

    public:
        osg::Vec3 &color();
        osg::Vec3 &position();
        osg::Vec3 &direction();
        double &attenuation();
        double &focus();
        double &angle();
    };

    class CAttribute
    {
    public:
        enum EAttributeType
        {
            EAT_VEC4 = 0,
            EAT_VEC3,
            EAT_VEC2,
            EAT_FLOAT,
        };

    public:
        int location;
        std::string name;
        EAttributeType type;

    public:
        CAttribute(int location, std::string name, EAttributeType type);
        ~CAttribute();
        static std::string getTypename(EAttributeType type);
    };

    class CPseudoMaterial : public osg::StateSet::Callback
    {
    protected:
        std::set<osg::ref_ptr<osg::Object> > m_objects;

        std::vector<osg::ref_ptr<osg::Uniform> > m_uniforms;
        std::vector<osg::ref_ptr<osg::Uniform> > m_internalUniforms;
        std::vector<osg::CAttribute> m_attributes;

        std::vector<osg::CDirectionalLightSource> m_directionalLightSources;
        int m_directionalLightSourceCount;
        std::vector<osg::CPointLightSource> m_pointLightSources;
        int m_pointLightSourceCount;
        std::vector<osg::CSpotLightSource> m_spotLightSources;
        int m_spotLightSourceCount;

        osg::ref_ptr<osg::Uniform> m_uniDummy;

        osg::ref_ptr<osg::Uniform> m_uniDirectionalLightSourceColor;
        osg::ref_ptr<osg::Uniform> m_uniDirectionalLightSourceDirection;
        osg::ref_ptr<osg::Uniform> m_uniPointLightSourceColor;
        osg::ref_ptr<osg::Uniform> m_uniPointLightSourcePosition;
        osg::ref_ptr<osg::Uniform> m_uniPointLightSourceAttenuation;
        osg::ref_ptr<osg::Uniform> m_uniSpotLightSourceColor;
        osg::ref_ptr<osg::Uniform> m_uniSpotLightSourcePosition;
        osg::ref_ptr<osg::Uniform> m_uniSpotLightSourceDirection;
        osg::ref_ptr<osg::Uniform> m_uniSpotLightSourceAttenuation;
        osg::ref_ptr<osg::Uniform> m_uniSpotLightSourceFocus;
        osg::ref_ptr<osg::Uniform> m_uniSpotLightSourceAngle;

        osg::ref_ptr<osg::Uniform> m_uniAlpha;
        osg::ref_ptr<osg::Uniform> m_uniDiffuse;
        osg::ref_ptr<osg::Uniform> m_uniEmission;
        osg::ref_ptr<osg::Uniform> m_uniShininess;
        osg::ref_ptr<osg::Uniform> m_uniSpecularity;

    protected:
        osg::ref_ptr<osg::Shader> m_vertShader;
        osg::ref_ptr<osg::Shader> m_geomShader;
        osg::ref_ptr<osg::Shader> m_fragShader;
        osg::ref_ptr<osg::Program> m_program;

        bool m_twoSided;
        bool m_flatShading;
        bool m_useVertexColors;
        bool m_dirty;

    public:
        CPseudoMaterial(bool twoSided = false, bool flatShading = false, bool useVertexColors = true);
        ~CPseudoMaterial();

        osg::Uniform *uniform(std::string name);

        void setFlatShading(bool flatShading);
        void setUseVertexColors(bool useVertexColors);
        std::vector<osg::CDirectionalLightSource> &directionalLightSources();
        std::vector<osg::CPointLightSource> &pointLightSources();
        std::vector<osg::CSpotLightSource> &spotLightSources();
        void fourLightSetup();
        void singleLightSetup();
        void updateLightUniforms();
        void makeDirty();

        void copyInternals(CPseudoMaterial *other);

    protected:
        void removeUniform(osg::Uniform *uniform);

        virtual std::string programName() { return "// CPseudoMaterial"; }

        virtual std::string vertVersion();
        virtual std::string geomVersion();
        virtual std::string fragVersion();

        virtual std::string uniforms();
        virtual std::string attributes();

        virtual std::vector<std::pair<std::string, std::string> > shaderInOuts();
        virtual std::string vertOuts();
        virtual std::string geomIns();
        virtual std::string geomOuts();
        virtual std::string fragIns();

        virtual std::string fragStruct();
        virtual std::string surfStruct();
        virtual std::string insToStruct();

        virtual std::string vertShaderSrc();
        virtual std::string fragShaderSrc();
        virtual std::string geomShaderSrc();
        virtual std::string surfaceShaderSrc();
        virtual std::string directionalLightsSrc();
        virtual std::string pointLightsSrc();
        virtual std::string spotLightsSrc();
        virtual std::string lightingSrc();

    public:
        virtual void apply(osg::Object *object, StateAttribute::GLModeValue value = osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

        //! you really should know what you're doing if you try to call this
        void revert(osg::Object *object);

    public:
        virtual void operator()(StateSet *stateSet, NodeVisitor *nodeVisitor);
    };

    class CPseudoMaterial_Rim : public CPseudoMaterial
    {
    public:
        osg::ref_ptr<osg::Uniform> m_uniRimColor;
        osg::ref_ptr<osg::Uniform> m_uniRimIntensity;
        osg::ref_ptr<osg::Uniform> m_uniRimPower;

    public:
        CPseudoMaterial_Rim();
        ~CPseudoMaterial_Rim();

    protected:
        virtual std::string programName() { return "// CPseudoMaterial_Rim"; }
        virtual std::string surfaceShaderSrc();
    };

    class CPseudoMaterial_Skinned : public CPseudoMaterial
    {
    protected:
        osg::ref_ptr<osg::Uniform> m_uniBoneMatrices;

    public:
        CPseudoMaterial_Skinned(int maxBones, bool twoSided = false);
        ~CPseudoMaterial_Skinned();

    protected:
        virtual std::string programName() { return "// CPseudoMaterial_Skinned"; }
        virtual std::string vertShaderSrc();
    };

    class CPseudoMaterial_Skinned_Rim : public CPseudoMaterial_Skinned
    {
    public:
        osg::ref_ptr<osg::Uniform> m_uniRimColor;
        osg::ref_ptr<osg::Uniform> m_uniRimIntensity;
        osg::ref_ptr<osg::Uniform> m_uniRimPower;

    public:
        CPseudoMaterial_Skinned_Rim(int maxBones, bool twoSided = false);
        ~CPseudoMaterial_Skinned_Rim();

    protected:
        virtual std::string programName() { return "// CPseudoMaterial_Skinned_Rim"; }
        virtual std::string surfaceShaderSrc();
    };

} // namespace osg

#endif // CPseudoMaterial_H

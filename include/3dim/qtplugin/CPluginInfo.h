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

#ifndef CPluginInfo_H
#define CPluginInfo_H

#include <app/CProductInfo.h>
#include <QString>
#include <sstream>

namespace plug
{

///////////////////////////////////////////////////////////////////////////////
//! Class encapsulates plugin info such as the name and version.

class CPluginInfo : public app::CProductInfo
{
public:
    //! Default constructor.
    CPluginInfo() {}

    //! Constructor.
    CPluginInfo(const std::string& ssHost,
                const std::string& ssName,
                const std::string& ssActName,
                int MajorNum,
                int MinorNum = 0
                )
        : app::CProductInfo(ssName, MajorNum, MinorNum)
        , m_ssHost(ssHost)
        , m_ssActivationName(ssActName)
    {}

    //! Copy constructor.
    CPluginInfo(const CPluginInfo& p)
        : app::CProductInfo(p)
        , m_ssHost(p.m_ssHost)
        , m_ssActivationName(p.m_ssActivationName)
    {}

    //! Destructor.
    ~CPluginInfo() {}

    //! Returns name of the host application the plugin is compiled for.
    const std::string& getHost() const { return m_ssHost; }

    //! Returns activation name.
    const std::string& getActivationName() const { return m_ssActivationName; }

    //! Sets name of the host application the plugin is compiled for.
    CPluginInfo& setHost(const std::string& ssName)
    {
        m_ssHost = ssName;
        return *this;
    }

    //! Returns the plugin product identifier containing the word 'Plugin' at the end.
    virtual std::string getLongProductId() const
    {
        static const std::string NAME_SUFFIX(" Plugin ");
        std::stringstream Stream;
        Stream << m_ssActivationName << NAME_SUFFIX << m_Version.getMajorNum() << '.' << m_Version.getMinorNum();
        return Stream.rdbuf()->str();
    }

    //! Returns true if the plugin is compiled for a given product.
    bool isCompatibleWithHost(const app::CProductInfo& Id) const
    {
        return (m_ssHost == Id.getName() && m_Version.isCompatibleWith(Id.getVersion()));
    }

    //! Used by an inherited class
    CPluginInfo& getPluginInfo()
    {
        return *this;
    }

	//! Current host setter
	void	setCurrentHost(const QString& curHost) { m_wsCurrentHost = curHost; } 

	//! Current host getter
	const QString&	getCurrentHost() const { return m_wsCurrentHost; } 

protected:
    //! Product name of the host application the plugin is compiled for.
    std::string m_ssHost;

    //! Name used for product activation.
    std::string m_ssActivationName;
	
	//! current host application including version number
	QString	m_wsCurrentHost;
};


} // namespace plug

#endif // CPluginInfo_H

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

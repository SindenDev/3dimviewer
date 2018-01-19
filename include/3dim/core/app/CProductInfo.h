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

#ifndef CProductInfo_H
#define CProductInfo_H

#include "CVersionNum.h"

// STL
#include <string>
#include <sstream>


namespace app
{

////////////////////////////////////////////////////////////
// Forward declarations.

class CProductInfo;

////////////////////////////////////////////////////////////
// Global functions

//! Returns product info about the current application.
//! - The macro DECLARE_PRODUCT_INFO must be used somewhere
//!   in the application source file.
const CProductInfo& getProductInfo();


////////////////////////////////////////////////////////////
// Product notes

//! Empty (default) product note.
const std::string EMPTY_NOTE = std::string("");

//! Release candidate...
const std::string RC_NOTE = std::string("rc");

//! Beta version...
const std::string BETA_NOTE = std::string("beta");

//! Alpha version...
const std::string ALPHA_NOTE = std::string("alpha");

//! Development version...
const std::string DEV_NOTE = std::string("dev");

////////////////////////////////////////////////////////////
// Macro definitions

//! Macro declares product name and version.
#define DECLARE_PRODUCT_INFO(Name, MajorNum, MinorNum, BuildNum, Note) \
    namespace app { \
    const CProductInfo& getProductInfo() \
    { \
        static const CProductInfo Info(Name, MajorNum, MinorNum, BuildNum, Note); \
        return Info; \
    } }

//! Macro returns reference to the product info.
#define APP_PRODUCT_INFO (app::getProductInfo())


///////////////////////////////////////////////////////////////////////////////
//! Class encapsulates product info such as the name and version.

class CProductInfo
{
public:
    //! Default constructor.
    CProductInfo() : m_Version(1,0) {}

    //! Constructor.
    CProductInfo(const std::string& ssName,
                 int MajorNum,
                 int MinorNum = 0,
                 int BuildNum = 0,
                 const std::string& ssNote = EMPTY_NOTE
                 )
        : m_ssName(ssName)
        , m_Version(MajorNum, MinorNum, BuildNum)
        , m_ssNote(ssNote)
    {}

    //! Copy constructor.
    CProductInfo(const CProductInfo& p)
        : m_ssName(p.m_ssName)
        , m_Version(p.m_Version)
        , m_ssNote(p.m_ssNote)
    {}

    //! Destructor.
    virtual ~CProductInfo() {}

    //! Returns product name.
    const std::string& getName() const { return m_ssName; }

    //! Returns product version.
    const CVersionNum& getVersion() const { return m_Version; }

    //! Returns product note.
    const std::string& getNote() const { return m_ssNote; }

    //! Sets the product name.
    CProductInfo& setName(const std::string& ssName)
    {
        m_ssName = ssName;
        return *this;
    }

    //! Sets the product version.
    CProductInfo& setVersion(const CVersionNum& Version)
    {
        m_Version = Version;
        return *this;
    }

    //! Sets the product note.
    CProductInfo& setNote(const std::string& ssNote)
    {
        m_ssNote = ssNote;
        return *this;
    }


    //! Returns string product identifier containing both the name and the current version.
    virtual std::string getProductId() const;

    //! Returns string product identifier containing the name, the current version and the note.
    std::string getProductIdWithNote() const;

    //! Sets product identifier.
    //! - Returns false on failure (e.g. wrong Id string format).
    virtual bool setProductId(const std::string& Id);


    //! Returns true if two product identifiers are identical.
    bool isIdentical(const CProductInfo& Id) const
    {
        return (m_ssName == Id.m_ssName && m_Version.isIdentical(Id.m_Version));
    }

    //! Returns true if two product identifiers are compatible.
    bool isCompatibleWith(const CProductInfo& Id) const
    {
        return (m_ssName == Id.m_ssName && m_Version.isCompatibleWith(Id.m_Version));
    }


    //! Normalizes the product id so that it will contain 'x' character
    //! instead of the minor version number.
    static bool normalizeProductId(std::string& Id);

protected:
    //! Product name.
    std::string m_ssName;

    //! Version number.
    CVersionNum m_Version;

    //! Product note.
    std::string m_ssNote;
};


} // namespace app

#endif // CProductInfo_H

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


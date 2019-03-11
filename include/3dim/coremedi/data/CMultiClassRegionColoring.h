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

#ifndef CMultiClassRegionColoring_H
#define CMultiClassRegionColoring_H

///////////////////////////////////////////////////////////////////////////////
// include files

#include "data/CColoringFunc.h"
#include "data/CObjectHolder.h"
#include "data/CSnapshot.h"
#include <data/CStorageInterface.h>
#include <data/storage_ids_core.h>
#include <data/CMultiClassRegionData.h>
#include <geometry/base/types.h>
#include <app/Signals.h>

// STL
#include <vector>

namespace data
{

///////////////////////////////////////////////////////////////////////////////
//! Additional region info.

class CMultiClassRegionInfo
{
public:
    //! Default constructor.
	CMultiClassRegionInfo() : m_ssName("Unnamed"), m_bVisible(true), m_bSelected(false), m_bAuxiliary(false) {}

    //! Just another constructor.
	CMultiClassRegionInfo(const std::string& Name) : m_ssName(Name), m_bVisible(true), m_bSelected(false), m_bAuxiliary(false) {}

    //! Changes the region name.
    CMultiClassRegionInfo& setName(const std::string& ssName)
    {
        m_ssName = ssName;
        return *this;
    }

    //! Changes region visibility.
    CMultiClassRegionInfo& setVisibility(bool bVisible)
    {
        m_bVisible = bVisible;
        return *this;
    }

	//! Sets region selection flag.
	CMultiClassRegionInfo& setSelected(bool bSelected)
	{
		m_bSelected = bSelected;
		return *this;
	}

	//! Sets region auxiliary flag.
	CMultiClassRegionInfo& setAuxiliary(bool bAuxiliary)
	{
		m_bAuxiliary = bAuxiliary;
		return *this;
	}

    CMultiClassRegionInfo& setSeedPoints(std::vector<geometry::Vec3> points)
    {
        m_seedPoints = points;
        return *this;
    }

    //! Returns region name.
    const std::string& getName() const { return m_ssName; }

    //! Returns if the region is visible.
    bool isVisible() const { return m_bVisible; }

	//! Returns if the region is selected.
	bool isSelected() const { return m_bSelected; }

	//! Returns if the region is auxiliary.
	bool isAuxiliary() const { return m_bAuxiliary; }

    const std::vector<geometry::Vec3>& getSeedPoints()
    {
        return m_seedPoints;
    }

	//! Serialize
	template < class tpSerializer >
	void serialize(vpl::mod::CChannelSerializer<tpSerializer> & Writer)
	{
        Writer.write((vpl::sys::tUInt32) 1); // version
		Writer.write( m_ssName );
		Writer.write( (unsigned char)m_bVisible );

        // Write seeds vector size
        Writer.write((vpl::sys::tUInt32)m_seedPoints.size());

        // Serialize seeds
        for (int i = 0; i < m_seedPoints.size(); ++i)
        {
            CSerializableData< geometry::Vec3 >::serialize(&m_seedPoints[i], Writer);
        }
	}

	//! Deserialize
	template < class tpSerializer >
	void deserialize(vpl::mod::CChannelSerializer<tpSerializer> & Reader)
	{
        vpl::sys::tUInt32 ver = 0;
        Reader.read(ver); // version

		Reader.read( m_ssName );

        unsigned char b = 0;
		Reader.read( b );
        m_bVisible = b;

        if (ver > 0)
        {
            // Read and set seeds vector size
            vpl::sys::tUInt32 size = 0;
            Reader.read(size);
            m_seedPoints.resize(size);

            // Deserialize seeds
            for (int i = 0; i < m_seedPoints.size(); ++i)
            {
                CSerializableData< geometry::Vec3 >::deserialize(&m_seedPoints[i], Reader);
            }
        }
	}

protected:
    //! Region name.
    std::string m_ssName;

    //! Visibility flag.
    bool m_bVisible;

	//! Selection flag.
	bool m_bSelected;

	//! Auxiliary region flag (e.g. region created during supervoxels segmentation).
	bool m_bAuxiliary;

    std::vector<geometry::Vec3> m_seedPoints;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//\class CMultiClassRegionColoringSnapshot
//
//\brief Region coloring snapshot. 
////////////////////////////////////////////////////////////////////////////////////////////////////

class CMultiClassRegionColoringSnapshot : public CSnapshot
{
public:
   //! Constructor
   CMultiClassRegionColoringSnapshot( int type, CUndoProvider * provider = NULL ) 
      : CSnapshot( type, provider )
      , m_Colors(1) 
      {}

   //! Destructor
   ~CMultiClassRegionColoringSnapshot(){}

   //! Each snapshot object must return its data size in bytes
   virtual long getDataSize() { return sizeof(CColorVector4b) * m_Colors.getSize() + sizeof( CMultiClassRegionInfo ) * m_Regions.size(); }

protected:
   //! Vector of region info structures.
   typedef std::vector<CMultiClassRegionInfo> tRegions;

   //! Vector of assigned colors.
   CColorVector4b m_Colors;

   //! Regions
   tRegions m_Regions;

   // Friend class
   friend class CMultiClassRegionColoring;
};

///////////////////////////////////////////////////////////////////////////////
//! Functor for coloring of segmented density data.

class CMultiClassRegionColoring : public CColoringFunc4b, public CUndoProvider 
{
public:
    //! Smart pointer type.
    //! - Declares type tSmartPtr.
    VPL_SHAREDPTR(CMultiClassRegionColoring);

    //! Initial number of regions.
    enum { NUM_OF_REGIONS = 0 };

    //! Default color transparency. 
    enum { ALPHA = 128 };

public:
    //! Default constructor.
    CMultiClassRegionColoring();

    //! Destructor.
    virtual ~CMultiClassRegionColoring() {}

    //! Changes the number of regions.
    void resize(int Size, bool bRandomColor = true)
    {
        if (Size > data::CMultiClassRegionData::getMaxNumberOfRegions())
        {
            Size = data::CMultiClassRegionData::getMaxNumberOfRegions();
        }
        
        // Resize the vector of colors
        m_Colors.resize(Size, bRandomColor, m_alpha);

        int oldSize = m_Regions.size();

        // Region info
        m_Regions.resize(tRegions::size_type(Size));

        int newRegionsCnt = Size - oldSize;

        for (int i = 0; i < newRegionsCnt; ++i)
        {
            ++m_maxRegionIndex;
            std::stringstream ss;
            ss << "Region " << m_maxRegionIndex;
            m_Regions[i + oldSize].setName(ss.str());

            // try to find unused color from predefined colors
            for (int c = 0; c < m_colorsTable.getSize(); ++c)
            {
                if (m_Colors.findColor(m_colorsTable[c]) < 0)
                {
                    m_Colors[oldSize + i] = m_colorsTable[c];
                    break;
                }
            }
        }
    }

    void removeRegion(int regionIndex)
    {
        if (regionIndex < 0 || regionIndex >= m_Regions.size())
        {
            return;
        }

        m_Regions.erase(m_Regions.begin() + regionIndex);
        m_Colors.erase(regionIndex);

        setActiveRegion(0);
    }

    //! Returns the number of regions.
    int getNumOfRegions() const { return m_Colors.getSize(); }

    //! Returns label of the active region.
    int getActiveRegion() const { return m_Active; }

    //! Changes the active region.
    CMultiClassRegionColoring& setActiveRegion(int i)
    {
        m_Active = (i >= 0 && i < getNumOfRegions()) ? i : m_Active;
        return *this;
    }

    //! Returns true if a region is visible.
    bool isVisible(int i) const
    { 
//        return m_Regions[i].isVisible();
        return (i < 0 || i >= getNumOfRegions()) ? false : m_Regions[i].isVisible();
    }

    //! Changes the active region.
    CMultiClassRegionColoring& setVisibility(int i, bool bVisible)
    {
        if( i < 0 || i >= getNumOfRegions() )
        {
            return *this;
        }
        m_Regions[i].setVisibility(bVisible);
        return *this;
    }

    //! Changes a region color.
    CMultiClassRegionColoring& setColor(int i, const tColor& Color)
    {
//        m_Colors.setColor(i, Color);
        m_Colors.setColorSafe(i, Color);
        return *this;
    }

    //! Returns a region color.
    const tColor& getColor(int i) const
    {
//        return m_Colors.getColor(i);
        return m_Colors.getColorSafe(i);
    }

    void setAlpha(int alpha)
    {
        m_alpha = alpha;

        for (int c = 0; c < m_colorsTable.getSize(); ++c)
        {
            m_colorsTable[c].getA() = m_alpha;
        }
    }

    //! Sets region seeds.
    CMultiClassRegionColoring& setSeedPoints(int i, const std::vector<geometry::Vec3> seeds)
    {
        if (i < 0 || i >= getNumOfRegions())
        {
            return *this;
        }
        m_Regions[i].setSeedPoints(seeds);
        return *this;
    }

    //! Returns region seeds.
    const std::vector<geometry::Vec3> getSeedPoints(int i)
    {
        if (i < 0 || i >= getNumOfRegions())
        {
            return std::vector<geometry::Vec3>();
        }
        return m_Regions[i].getSeedPoints();
    }

    bool isActiveRegionBitSet(tPixel value)
    {
        unsigned int mask = 1LL << m_Active;
        unsigned int maskedValue = value & mask;
        return maskedValue >> m_Active;
    }

    tPixel getTopVisibleRegionBitIndex(const tPixel& Density)
    {
        tPixel value = Density;

        if (isActiveRegionBitSet(value))
        {
            return m_Active;
        }

        assert(value != 0); // handled separately

        unsigned int pos = 0;
        int bitIndex = -1;

        while (1)
        {
            if (value == 0)
            {
                break;
            }

            if (pos >= 0 && pos < m_Regions.size() && (value & 1) && m_Regions[pos].isVisible())
            {
                bitIndex = pos;
            }

            value >>= 1;
            ++pos;
        }

        return bitIndex;
    }

    //! Coloring function.
    virtual tColor makeColor(const tPixel& Density)
    {
        tPixel i = Density;
        if (i <= 0)
        {
            return m_DummyColor;
        }
        else
        {
            i = getTopVisibleRegionBitIndex(Density);
        }

        if( i < 0 || i >= getNumOfRegions())
        {
            return m_DummyColor;
        }

        return m_Colors.getColor(i);
    }

    //! Returns type of the coloring function.
    virtual int getType() const { return ColoringFunc::REGION_COLORING; }

    //! Does object contain relevant data?
    virtual bool hasData(){ return true; }

    //! Returns reference to the region info.
    CMultiClassRegionInfo& getRegionInfo(int i)
    {
//        return m_Regions[i];
        return (i < 0 || i >= getNumOfRegions()) ? m_DummyRegion : m_Regions[i];
    }

    //! Returns reference to the region info.
    const CMultiClassRegionInfo& getRegionInfo(int i) const
    {
//        return m_Regions[i];
        return (i < 0 || i >= getNumOfRegions()) ? m_DummyRegion : m_Regions[i];
    }

	void setRegionInfo(int i, const CMultiClassRegionInfo &info)
	{
		if (i >= 0 || i < getNumOfRegions())
		{
			m_Regions[i] = info;
		}
	}

    //! Regenerates the object state according to any changes in the data storage.
    void update(const CChangedEntries& VPL_UNUSED(Changes))
    {
        // Does nothing...
    }

	//! Initializes the object to its default state.
    void init()
    {
        m_alpha = ALPHA;

        resize(NUM_OF_REGIONS);

        // Initialize colors
        /*m_Colors[0] = tColor(255, 128, 128, m_alpha);
        m_Colors[1] = tColor(128, 255, 128, m_alpha);
        m_Colors[2] = tColor(128, 128, 255, m_alpha);
        m_Colors[3] = tColor(255, 255, 128, m_alpha);
        m_Colors[4] = tColor(128, 255, 255, m_alpha);
        m_Colors[5] = tColor(255, 128, 255, m_alpha);
        m_Colors[6] = tColor(255, 128, 64, m_alpha);
        m_Colors[7] = tColor(0, 128, 128, m_alpha);
        m_Colors[8] = tColor(128, 128, 64, m_alpha);

        // Initialize regions
		m_Regions[0].setName("Region 1").setVisibility(true).setSelected(false).setAuxiliary(false);
		m_Regions[1].setName("Region 2").setVisibility(true).setSelected(false).setAuxiliary(false);
		m_Regions[2].setName("Region 3").setVisibility(true).setSelected(false).setAuxiliary(false);
		m_Regions[3].setName("Region 4").setVisibility(true).setSelected(false).setAuxiliary(false);
		m_Regions[4].setName("Region 5").setVisibility(true).setSelected(false).setAuxiliary(false);
		m_Regions[5].setName("Region 6").setVisibility(true).setSelected(false).setAuxiliary(false);
		m_Regions[6].setName("Region 7").setVisibility(true).setSelected(false).setAuxiliary(false);
		m_Regions[7].setName("Region 8").setVisibility(true).setSelected(false).setAuxiliary(false);
		m_Regions[8].setName("Region 9").setVisibility(true).setSelected(false).setAuxiliary(false);*/

        m_Active = 0;
        m_maxRegionIndex = 0;
    }

    //! Returns true if changes of a given parent entry may affect this object.
    bool checkDependency(CStorageEntry * VPL_UNUSED(pParent)) { return true; }

    // Undo providing
    //! Create snapshot of the current state. 
    virtual CSnapshot * getSnapshot( CSnapshot * VPL_UNUSED(snapshot) )
    {
       CMultiClassRegionColoringSnapshot * s = new CMultiClassRegionColoringSnapshot( data::UNDO_ALL, this );

       // Store regions
       for( tRegions::iterator r = m_Regions.begin(); r != m_Regions.end(); ++r )
          s->m_Regions.push_back( *r );

       // Store colors
       s->m_Colors.resize( m_Colors.getSize() );
       for( int i = 0; i < m_Colors.getSize(); ++i )
          s->m_Colors.setColor( i, m_Colors.getColor( i ) );

       return s;
    }

    //! Restore state from the snapshot
    virtual void restore( CSnapshot * snapshot )
    {
       if( snapshot == 0 )
          return;

       CMultiClassRegionColoringSnapshot * s = dynamic_cast< CMultiClassRegionColoringSnapshot * >( snapshot );
       if( s == 0 )
          return;

       // Restore regions
       m_Regions.clear();
       for( tRegions::iterator r = s->m_Regions.begin(); r != s->m_Regions.end(); ++r )
          m_Regions.push_back( *r );

       // Restore colors
       m_Colors.resize( s->m_Colors.getSize() );
       for( int i = 0; i < s->m_Colors.getSize(); ++i )
          m_Colors.setColor( i, s->m_Colors.getColor( i ) );

       if (m_Active >= m_Regions.size())
       {
           m_Active = 0;
       }

       VPL_SIGNAL(SigUndoOnColoringPerformed).invoke();
    }

	//! Serialize
	template < class tpSerializer >
	void serialize(vpl::mod::CChannelSerializer<tpSerializer> & Writer)
	{
		Writer.beginWrite( *this );

		Writer.write( (vpl::sys::tUInt32) 2 ); // version

		m_Colors.serialize( Writer );
				
		// Write regions vector size
		Writer.write( (vpl::sys::tUInt32)m_Regions.size() );

		// Serialize regions
		tRegions::iterator it, itEnd( m_Regions.end() );
		for( it = m_Regions.begin(); it != itEnd; ++it )
			it->serialize( Writer );

		//m_DummyRegion.serialize( Writer );
		Writer.write( (vpl::sys::tInt32)m_Active );
		//m_DummyColor.serialize( Writer );

        Writer.write((vpl::sys::tInt32)m_maxRegionIndex);

		Writer.endWrite(*this);
	}

	//! Deserialize
	template < class tpSerializer >
	void deserialize(vpl::mod::CChannelSerializer<tpSerializer> & Reader)
	{
		Reader.beginRead(*this);

		vpl::sys::tUInt32 ver = 0;
		Reader.read( ver ); // version

		m_Colors.deserialize( Reader );

		// Read and set regions vector size
		vpl::sys::tUInt32 size = 0;
		Reader.read( size );
		m_Regions.resize( size );

		// Deserialize regions
		tRegions::iterator it, itEnd( m_Regions.end() );
		for( it = m_Regions.begin(); it != itEnd; ++it )
			it->deserialize( Reader );

		//m_DummyRegion.deserialize( Reader );
        vpl::sys::tInt32 active = 0;
		Reader.read( active );
        m_Active = active;
		//m_DummyColor.deserialize( Reader );

        if (ver > 1)
        {
            vpl::sys::tInt32 regionIndex = 0;
            Reader.read(regionIndex);
            m_maxRegionIndex = regionIndex;
        }

		Reader.endRead( *this );
	}

    //! Colorize whole slice image
    virtual void colorize(vpl::img::CRGBImage &rgbImage, const vpl::img::CDImage &densityImage, const data::CSlicePropertyContainer &properties)
    {
        // region coloring does nothing here
    }

    void colorize(vpl::img::CRGBImage &rgbImage, const vpl::img::CImage<data::tRegionVoxel> &regionImage);

protected:
    //! Vector of region info structures.
    typedef std::vector<CMultiClassRegionInfo> tRegions;

protected:

    //! Vector of predefined colors.
    CColorVector4b m_colorsTable;

    //! Vector of assigned colors.
    CColorVector4b m_Colors;

    //! Vector of region info structures.
    tRegions m_Regions;

    //! Dummy region.
    CMultiClassRegionInfo m_DummyRegion;

    //! Index of the active region.
    int m_Active;

    //! Dummy color.
    tColor m_DummyColor;

    int m_maxRegionIndex;

    int m_alpha;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Serialization wrapper. 
////////////////////////////////////////////////////////////////////////////////////////////////////
DECLARE_SERIALIZATION_WRAPPER( CMultiClassRegionColoring )

namespace Storage
{
	//! Region coloring.
	DECLARE_OBJECT(MultiClassRegionColoring, CMultiClassRegionColoring, CORE_STORAGE_MULTI_CLASS_REGION_COLORING_ID);
}

} // namespace data

#endif // CMultiClassRegionColoringFunc_H

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

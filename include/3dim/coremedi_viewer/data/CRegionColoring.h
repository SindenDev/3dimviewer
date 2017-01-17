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

#ifndef CRegionColoring_H
#define CRegionColoring_H

///////////////////////////////////////////////////////////////////////////////
// include files

#include "data/CColoringFunc.h"
#include "data/CObjectHolder.h"
#include "data/CSnapshot.h"
#include <data/CStorageInterface.h>
#include <data/storage_ids_core.h>

// STL
#include <vector>

namespace data
{

///////////////////////////////////////////////////////////////////////////////
//! Additional region info.

class CRegionInfo
{
public:
    //! Default constructor.
	CRegionInfo() : m_ssName("Unnamed"), m_bVisible(true), m_bSelected(false), m_bAuxiliary(false) {}

    //! Just another constructor.
	CRegionInfo(const std::string& Name) : m_ssName(Name), m_bVisible(true), m_bSelected(false), m_bAuxiliary(false) {}

    //! Changes the region name.
    CRegionInfo& setName(const std::string& ssName)
    {
        m_ssName = ssName;
        return *this;
    }

    //! Changes region visibility.
    CRegionInfo& setVisibility(bool bVisible)
    {
        m_bVisible = bVisible;
        return *this;
    }

	//! Sets region selection flag.
	CRegionInfo& setSelected(bool bSelected)
	{
		m_bSelected = bSelected;
		return *this;
	}

	//! Sets region auxiliary flag.
	CRegionInfo& setAuxiliary(bool bAuxiliary)
	{
		m_bAuxiliary = bAuxiliary;
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

	//! Serialize
	template < class tpSerializer >
	void serialize(vpl::mod::CChannelSerializer<tpSerializer> & Writer)
	{
		Writer.write( m_ssName );
		Writer.write( (unsigned char)m_bVisible );
		//Writer.write((unsigned char)m_bSelected);
		//Writer.write((unsigned char)m_bAuxiliary);
	}

	//! Deserialize
	template < class tpSerializer >
	void deserialize(vpl::mod::CChannelSerializer<tpSerializer> & Reader)
	{
		Reader.read( m_ssName );

        unsigned char b = 0;
		Reader.read( b );
        m_bVisible = b;

		/*b = 0;
		Reader.read(b);
		m_bSelected = b;

		b = 0;
		Reader.read(b);
		m_bAuxiliary = b;*/
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
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//\class CRegionColoringSnapshot
//
//\brief Region coloring snapshot. 
////////////////////////////////////////////////////////////////////////////////////////////////////

class CRegionColoringSnapshot : public CSnapshot
{
public:
   //! Constructor
   CRegionColoringSnapshot( int type, CUndoProvider * provider = NULL ) 
      : CSnapshot( type, provider )
      , m_Colors(1) 
      {}

   //! Destructor
   ~CRegionColoringSnapshot(){}

   //! Each snapshot object must return its data size in bytes
   virtual long getDataSize() { return sizeof(CColorVector4b) * m_Colors.getSize() + sizeof( CRegionInfo ) * m_Regions.size(); }

protected:
   //! Vector of region info structures.
   typedef std::vector<CRegionInfo> tRegions;

   //! Vector of assigned colors.
   CColorVector4b m_Colors;

   //! Regions
   tRegions m_Regions;

   // Friend class
   friend class CRegionColoring;
};

///////////////////////////////////////////////////////////////////////////////
//! Functor for coloring of segmented density data.

class CRegionColoring : public CColoringFunc4b, public CUndoProvider 
{
public:
    //! Smart pointer type.
    //! - Declares type tSmartPtr.
    VPL_SHAREDPTR(CRegionColoring);

    //! Initial number of regions.
    enum { NUM_OF_REGIONS = 10 };

    //! Default color transparency. 
    enum { ALPHA = 128 };

public:
    //! Default constructor.
    CRegionColoring();

    //! Destructor.
    virtual ~CRegionColoring() {}

    //! Changes the number of regions.
    void resize(int Size, bool bRandomColor = true)
    {
        // Resize the vector of colors
        m_Colors.resize(Size, bRandomColor, ALPHA);

        // Region info
        m_Regions.resize(tRegions::size_type(Size));
    }

    //! Returns the number of regions.
    int getNumOfRegions() const { return m_Colors.getSize(); }

    //! Returns label of the active region.
    int getActiveRegion() const { return m_Active; }

    //! Changes the active region.
    CRegionColoring& setActiveRegion(int i)
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
    CRegionColoring& setVisibility(int i, bool bVisible)
    {
        if( i < 0 || i >= getNumOfRegions() )
        {
            return *this;
        }
        m_Regions[i].setVisibility(bVisible);
        return *this;
    }

    //! Changes a region color.
    CRegionColoring& setColor(int i, const tColor& Color)
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

    //! Coloring function.
    virtual tColor makeColor(const tPixel& Density)
    {
        int i = int(Density);
        if( i < 0 || i >= getNumOfRegions() || !m_Regions[i].isVisible() )
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
    CRegionInfo& getRegionInfo(int i)
    {
//        return m_Regions[i];
        return (i < 0 || i >= getNumOfRegions()) ? m_DummyRegion : m_Regions[i];
    }

    //! Returns reference to the region info.
    const CRegionInfo& getRegionInfo(int i) const
    {
//        return m_Regions[i];
        return (i < 0 || i >= getNumOfRegions()) ? m_DummyRegion : m_Regions[i];
    }

	void setRegionInfo(int i, const CRegionInfo &info)
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
        resize(NUM_OF_REGIONS);

        // Initialize colors
        m_Colors[0] = tColor(0, 0, 0, 0);
        m_Colors[1] = tColor(255, 128, 128, ALPHA);
        m_Colors[2] = tColor(128, 255, 128, ALPHA);
        m_Colors[3] = tColor(128, 128, 255, ALPHA);
        m_Colors[4] = tColor(255, 255, 128, ALPHA);
        m_Colors[5] = tColor(128, 255, 255, ALPHA);
        m_Colors[6] = tColor(255, 128, 255, ALPHA);
        m_Colors[7] = tColor(255, 128, 64, ALPHA);
        m_Colors[8] = tColor(0, 128, 128, ALPHA);
        m_Colors[9] = tColor(128, 128, 64, ALPHA);

        // Initialize regions
        m_Regions[0].setName("Not classified").setVisibility(true).setSelected(false).setAuxiliary(false);
		m_Regions[1].setName("Region 1").setVisibility(true).setSelected(false).setAuxiliary(false);
		m_Regions[2].setName("Region 2").setVisibility(true).setSelected(false).setAuxiliary(false);
		m_Regions[3].setName("Region 3").setVisibility(true).setSelected(false).setAuxiliary(false);
		m_Regions[4].setName("Region 4").setVisibility(true).setSelected(false).setAuxiliary(false);
		m_Regions[5].setName("Region 5").setVisibility(true).setSelected(false).setAuxiliary(false);
		m_Regions[6].setName("Region 6").setVisibility(true).setSelected(false).setAuxiliary(false);
		m_Regions[7].setName("Region 7").setVisibility(true).setSelected(false).setAuxiliary(false);
		m_Regions[8].setName("Region 8").setVisibility(true).setSelected(false).setAuxiliary(false);
		m_Regions[9].setName("Region 9").setVisibility(true).setSelected(false).setAuxiliary(false);

        m_Active = 1;
    }

    //! Returns true if changes of a given parent entry may affect this object.
    bool checkDependency(CStorageEntry * VPL_UNUSED(pParent)) { return true; }

    // Undo providing
    //! Create snapshot of the current state. 
    virtual CSnapshot * getSnapshot( CSnapshot * VPL_UNUSED(snapshot) )
    {
       CRegionColoringSnapshot * s = new CRegionColoringSnapshot( data::UNDO_ALL, this );

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

       CRegionColoringSnapshot * s = dynamic_cast< CRegionColoringSnapshot * >( snapshot );
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
    }

	//! Serialize
	template < class tpSerializer >
	void serialize(vpl::mod::CChannelSerializer<tpSerializer> & Writer)
	{
		Writer.beginWrite( *this );

		Writer.write( (vpl::sys::tUInt32) 1 ); // version

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

		// Serialize regions
		tRegions::iterator it, itEnd( m_Regions.end() );
		for( it = m_Regions.begin(); it != itEnd; ++it )
			it->deserialize( Reader );

		//m_DummyRegion.deserialize( Reader );
        vpl::sys::tInt32 active = 0;
		Reader.read( active );
        m_Active = active;
		//m_DummyColor.deserialize( Reader );

		Reader.endRead( *this );
	}

    //! Colorize whole slice image
    virtual void colorize(vpl::img::CRGBImage &rgbImage, const vpl::img::CDImage &densityImage, const data::CSlicePropertyContainer &properties)
    {
        // region coloring does nothing here
    }

    void colorize(vpl::img::CRGBImage &rgbImage, const vpl::img::CImage16 &regionImage)
    {
		const int xSize = std::min(regionImage.getXSize(),rgbImage.getXSize());
		const int ySize = std::min(regionImage.getYSize(),rgbImage.getYSize());
#pragma omp parallel for
		for (int y = 0; y < ySize; ++y)
		{
			for (int x = 0; x < xSize; ++x)
            {
				if (getRegionInfo(regionImage(x, y)).isSelected())
				{
					tColor newColor(255, 255, 0, 255);
					rgbImage(x, y) = *(reinterpret_cast<vpl::img::tRGBPixel *>(&newColor));
				}
				else
				{
					vpl::img::CRGBPixel pixel = rgbImage(x, y);
					tColor prevColor = *(reinterpret_cast<tColor *>(&pixel));
					tColor currColor = makeColor(regionImage(x, y));
					tColor newColor = blendColors(currColor, prevColor);
					rgbImage(x, y) = *(reinterpret_cast<vpl::img::tRGBPixel *>(&newColor));
				}
            }
        }
    }

protected:
    //! Vector of region info structures.
    typedef std::vector<CRegionInfo> tRegions;

protected:
    //! Vector of assigned colors.
    CColorVector4b m_Colors;

    //! Vector of region info structures.
    tRegions m_Regions;

    //! Dummy region.
    CRegionInfo m_DummyRegion;

    //! Index of the active region.
    int m_Active;

    //! Dummy color.
    tColor m_DummyColor;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Serialization wrapper. 
////////////////////////////////////////////////////////////////////////////////////////////////////
DECLARE_SERIALIZATION_WRAPPER( CRegionColoring )

namespace Storage
{
	//! Region coloring.
	DECLARE_OBJECT(RegionColoring, CRegionColoring, CORE_STORAGE_REGION_COLORING_ID);
}

} // namespace data

#endif // CRegionColoringFunc_H

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

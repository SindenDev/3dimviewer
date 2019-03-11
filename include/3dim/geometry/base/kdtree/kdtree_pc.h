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

#ifndef kdtree_pc_H_included
#define kdtree_pc_H_included

#include <geometry/base/types.h>
#include <Eigen/StdVector>
#include <geometry/base/kdtree/CFlannTree.h>

namespace geometry
{
    /**
     * \class   CPointCloudAdaptor
     *
     * \brief   A point cloud adaptor - interface between flann and geometry library.
    **/
    template<typename tData>
    class CPointlCloudAdaptor : public vpl::base::CObject
    {
    protected:
        const static float FMAX;
        const static float FMIN;

    public:
        VPL_SHAREDPTR(CPointlCloudAdaptor);

        //! Data pointer type
        typedef std::vector<tData, Eigen::aligned_allocator<tData> > tDataVec;

    public:
        //! Simple constructor
        CPointlCloudAdaptor();

        //! Initializing constructor
        CPointlCloudAdaptor(const geometry::Vec3Array &points, const tDataVec &data);

        //! Add point (and data)
        void addPoint(const geometry::Vec3 &p, const tData &d);

        //! Get vertex coordinates in vpl format
        const geometry::Vec3 &getPoint(size_t id) const { return m_points[id]; }

        //! Get data item
        const tData &getData(size_t id) const { return m_data[id]; }

        //! Get data item - non const version
        tData &getData(size_t id) { return m_data[id]; }

        //! Get stored points
        const Vec3Array &getPoints() const { return m_points; }

        //! Does it have data?
        bool hasData() const { return m_points.size() > 0; }

        size_t size() const { return m_points.size(); }

        /**
         * Calculates the bounding box
         *
         * \param [in,out]  min The bounding box minimum.
         * \param [in,out]  max The bounding box maximum.
         *
         * \return  True if it succeeds, false if it fails.
         */
        bool calcBoundingBox(geometry::Vec3 &min, geometry::Vec3 &max) const;

        /**
         * Gets data vector access
         *
         * \return  The data vector.
         */

        const tDataVec &getDataVec() const 
        { 
            return m_data; 
        }
    protected:
        //! Stored points
        geometry::Vec3Array m_points;

        //! Stored data
        tDataVec m_data;
    }; // CPointCloudAdaptor

    /**
     * \class   CKDTreePointNormalCloud
     *
     * \brief   A KD tree for point+normal cloud.
    **/

    template<typename tData>
    class CKDTreePointCloud : public vpl::base::CObject
    {
    public:
        // Smart pointer definition
        VPL_SHAREDPTR(CKDTreePointCloud);

        //! Index vector type
        typedef std::vector<size_t> tIndexVec;

        //! Distance vector type
        typedef std::vector<double> tDistanceVec;

        //! Neighbor searches output type
        struct SIndexDistancePairs
        {
            // Constructors
            SIndexDistancePairs(){}
            SIndexDistancePairs(size_t allocated) : indexes(allocated), distances(allocated) {}

            // Reserve data
            void reserve(size_t size) { indexes.reserve(size); distances.reserve(size); }

            tIndexVec indexes;
            tDistanceVec distances;
        };

        bool calcBoundingBox(geometry::Vec3 &min, geometry::Vec3 &max) const;

    protected:
        //! Point cloud adaptor type
        typedef CPointlCloudAdaptor<tData> tPointCloudAdaptor;

        //! Point cloud adaptor pointer type
        typedef typename CPointlCloudAdaptor<tData>::tSmartPtr tPointCloudAdaptorPtr;

        //! Data vector type
        typedef typename CPointlCloudAdaptor<tData>::tDataVec tDataVec;


    public:
        //! Simple constructor
        CKDTreePointCloud();

        //! Destructor
        ~CKDTreePointCloud();

        //! Initialize empty structure
        void init() { clear(); m_pc = new tPointCloudAdaptor(); }

        //! Initializing constructor
        CKDTreePointCloud(const geometry::Vec3Array &points, const tDataVec &data);

        //! Initialize tree
        bool create(const geometry::Vec3Array &points, const tDataVec &data);

        //! Initialize tree from the currently stored points (use addPoint to add data)
        bool create();

        //! Add point
        void addPoint(const geometry::Vec3 &p, const tData &d);

        //! Has tree data?
        bool hasData() const { return m_tree.hasData() && m_pc != nullptr && m_pc->hasData(); }

        //! Get closest point index and distance
        bool getClosestPoints(const geometry::Vec3 &point, SIndexDistancePairs &result) const;

        //! Radius search
        int getPointsInsideRadius(const geometry::Vec3 &point, double radius, SIndexDistancePairs &result, size_t max_returned_points) const
        {
            if (!hasData())
                return 0;
            
            // Do search
            return(m_tree.radiusSearch(point, radius, result.distances, result.indexes, max_returned_points));
        }

        //! Get point coordinates in vpl format
        const geometry::Vec3 &getPoint(size_t idx) const { return m_pc->getPoint(idx); }

        //! Get data on given index
        const tData &getData(size_t idx) const { return m_pc->getData(idx); }

        //! Get data on given index - non const version
        tData &getData(size_t idx) { return m_pc->getData(idx); }

        //! Size of tree - number of stored points
        size_t size() const { return(m_pc != nullptr ? m_pc->size() : 0); }

        //! Get number of points stored in the cloud (points here can be before tree initialization).
        size_t getNumPointsInCloud() const { return m_pc != nullptr ? m_pc->size() : 0; }

        const Vec3Array &getPointsPositions() const { return m_pc->getPoints(); }

        //! Refresh tree after some points position changes.
        bool refreshTree()
        {
            // Create tree data structure
            m_tree.init(m_pc->getPoints());

            // Return true if tree is valid
            return m_tree.hasData();
        }

        /**
         * Gets data vector access
         *
         * \return  The data vector.
         */

        const tDataVec &getDataVec() const { return m_pc->getDataVec(); }

    protected:
        //! Delete all previous data
        void clear();

    protected:
        //! Mesh adaptor
        tPointCloudAdaptorPtr m_pc;

        //! KD tree
        CFlannTree m_tree;
    };

/************************************************************************/
/* IMPLEMENTATION                                                       */
/************************************************************************/


/**
* \fn  geometry::CPointNormalCloudAdaptor::CPointNormalCloudAdaptor()
*
* \brief   Default constructor.
**/
    template<typename tData>
    CPointlCloudAdaptor<tData>::CPointlCloudAdaptor()
    {

    }

    /**
    * \fn  geometry::CPointNormalCloudAdaptor::CPointNormalCloudAdaptor(const geometry::Vec3Array &points, const geometry::Vec3Array &normals)
    *
    * \brief   Constructor with initializing arrays.
    *
    * \param   points  The points.
    * \param   normals The normals.
    **/
    template<typename tData>
    CPointlCloudAdaptor<tData>::CPointlCloudAdaptor(const geometry::Vec3Array &points, const tDataVec &data)
    {
        if (points.size() == data.size())
        {
            // Copy data
            m_points = points;
            m_data = data;

        }
    }

    /**
    * \fn  void geometry::CPointNormalCloudAdaptor::addPoint(const geometry::Vec3 &p, const geometry::Vec3 &n)
    *
    * \brief   Adds a point to structures.
    *
    * \param   p   The input point.
    * \param   d   The data with point.
    **/
    template<typename tData>
    void CPointlCloudAdaptor<tData>::addPoint(const geometry::Vec3 &p, const tData &d)
    {
        // Add to the vectors
        m_points.push_back(p);
        m_data.push_back(d);
    }

    template <typename tData>

    /**
     * Calculates the bounding box of the point cloud. If cannot calculate it, inputs are not changed and false is returned.
     *
     * \tparam  tData   Type of the data.
     * \param [in,out]  min The minimum.
     * \param [in,out]  max The maximum.
     *
     * \return  True if it succeeds, false if it fails.
     */

    bool CPointlCloudAdaptor<tData>::calcBoundingBox(geometry::Vec3& min, geometry::Vec3& max) const
    {
        if (m_points.empty())
            return false;

        const geometry::Vec3::tElement infinity(std::numeric_limits<geometry::Vec3::tElement>::max());
        geometry::Vec3 _min(infinity, infinity, infinity);
        geometry::Vec3 _max(-_min);

        for(auto point : m_points)
        {
            for(int i = 0; i < 3; ++i)
            {
                if (point[i] > _max[i])
                    _max[i] = point[i];

                if (point[i] < _min[i])
                    _min[i] = point[i];
            }
        }

        if(_max[0] >= _min[0] && _max[1] >= _min[1] && _max[2] >= _min[2])
        {
            max = _max;
            min = _min;
            return true;
        }
        
        return false;
    }


    /************************************************************************/
    /* CLASS CKDTreePointNormalCloud                                        */
    /************************************************************************/

    template <typename tData>
    bool CKDTreePointCloud<tData>::calcBoundingBox(geometry::Vec3& min, geometry::Vec3& max) const
    {
        return m_pc->calcBoundingBox(min, max);
    }

    /**
    * \fn  geometry::CKDTreePointNormalCloud::CKDTreePointNormalCloud()
    *
    * \brief   Default constructor.
    **/
    template<typename tData>
    CKDTreePointCloud<tData>::CKDTreePointCloud()
    {
    }

    /**
    * \fn  geometry::CKDTreePointNormalCloud::~CKDTreePointNormalCloud()
    *
    * \brief   Destructor.
    **/
    template<typename tData>
    CKDTreePointCloud<tData>::~CKDTreePointCloud()
    {
        clear();
    }

    /**
    * \fn  geometry::CKDTreePointNormalCloud::CKDTreePointNormalCloud(const geometry::Vec3Array &points, const geometry::Vec3Array &normals)
    *
    * \brief   Constructor with initialization.
    *
    * \param   points  The points.
    * \param   data The data for each point.
    **/
    template<typename tData>
    CKDTreePointCloud<tData>::CKDTreePointCloud(const geometry::Vec3Array &points, const tDataVec &data)
    {
        create(points, data);
    }


    template<typename tData>

    /**
     * \fn  bool geometry::CKDTreePointCloud<tData>::create()
     *
     * \brief   Creates a new bool.
     *
     * \tparam  tData   Type of the data.
     *
     * \return  true if it succeeds, false if it fails.
    **/
    bool geometry::CKDTreePointCloud<tData>::create()
    {
        // Create tree data structure
        if(m_pc != nullptr)
            return m_tree.init(m_pc->getPoints());

        return false;
    }

    /**
     * \fn  template<typename tData> void geometry::CKDTreePointCloud<tData>::addPoint(const geometry::Vec3 &p, const tData &d)
     *
     * \brief   Adds a point and dat to the structure.
     *
     * \tparam  tData   Type of the data.
     * \param   p   The geometry point to add.
     * \param   d   The data tied with given point.
    **/
    template<typename tData>
    void geometry::CKDTreePointCloud<tData>::addPoint(const geometry::Vec3 &p, const tData &d)
    {
        if (m_pc == nullptr)
            m_pc = new tPointCloudAdaptor();

        m_pc->addPoint(p, d);
    }

    /**
    * \fn  bool geometry::CKDTreePointNormalCloud::getClosestPoints(const geometry::Vec3 &point, SIndexDistancePairs &result) const
    *
    * \brief   Gets closest points.
    *
    * \param   point           The point.
    * \param [in,out]  result  The result.
    *
    * \return  true if it succeeds, false if it fails.
    **/
    template<typename tData>
    bool CKDTreePointCloud<tData>::getClosestPoints(const geometry::Vec3 &point, SIndexDistancePairs &result) const
    {
        if (!hasData())
            return false;

        // Do search
        return(m_tree.nearestSearch(point, result.distances, result.indexes, std::max<size_t>(result.indexes.size(), 1)) > 0);
    }

    /**
    * \fn  void geometry::CKDTreePointNormalCloud::clear()
    *
    * \brief   Clears this object to its blank/initial state.
    **/
    template<typename tData>
    void CKDTreePointCloud<tData>::clear()
    {
        m_tree.clear();
        m_pc = nullptr;
    }

    /**
    * \fn  bool geometry::CKDTreePointNormalCloud::create(const geometry::Vec3Array &points, const geometry::Vec3Array &normals)
    *
    * \brief   Creates a new bool.
    *
    * \param   points  The points.
    * \param   normals The normals.
    *
    * \return  true if it succeeds, false if it fails.
    **/
    template<typename tData>
    bool CKDTreePointCloud<tData>::create(const geometry::Vec3Array &points, const tDataVec &data)
    {
        // Clear previous data
        clear();

        // Create new adaptor
        m_pc = new geometry::CPointlCloudAdaptor<tData>(points, data);


        return refreshTree();
    }
} // namespace geometry


// kdtree_pc_H_included
#endif

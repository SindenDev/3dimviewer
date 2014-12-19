///////////////////////////////////////////////////////////////////////////////
//
// 3DimViewer
// Lightweight 3D DICOM viewer.
//
// Copyright 2008-2014 3Dim Laboratory s.r.o.
// All rights reserved
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _LEVELSET3DCORE
#define _LEVELSET3DCORE

#include "GaussKernel.h"

#include <VPL/Base/Setup.h>
#include <VPL/Module/Module.h>
#include <VPL/Image/DensityVolume.h>

#include <list>
#include <functional>

#include <data/CRegionData.h>

#include <QMutex>

#ifdef __APPLE__
#define _NOSTDFUNCTION
#endif

//#define _NOSTDFUNCTION

typedef vpl::img::CCoordinates3<int>::tComponent CoordinateComponent;
typedef std::vector<vpl::img::CCoordinates3<int> > CoordinateList;

#define NEIGHBOR_MAX_3D 6

#define INNER_PHI -3
#define OUTTER_PHI 3
#define INNER_BOUNDARY_PHI -1
#define OUTTER_BOUNDARY_PHI 1

#define BACKGROUND_VF 100
#define FOREGROUND_VF 1
#define UNKNOWN_VF    0

/*
* Class providing the level-set segmentation of volumetric data
*/
class CLevelSet3DCore : public vpl::mod::CProgress
{
public:
	CLevelSet3DCore();	
	~CLevelSet3DCore();
	/*********************************************/
	/******		INITIALIZATION METHODS		******/
	/*********************************************/
		
	//initialization of phi
	void initializePhi3D(int value);
	
	//initial signed phi function as cube
	void initialCube(vpl::img::CCoordinates3<int> &leftup, int size, int height);
	
	//DOCASNE Initial sphere
	void sphere(int radius, vpl::img::CCoordinates3<int> center, int list);

	void fillTrigonometric();
	
	void InitialSphere(int radius, vpl::img::CCoordinates3<int> center);
	
	void initalizeFromRegionData(data::CRegionData* pData, int activeRegion);
	//--------------------------
	
	//checks if cube is in data and changes its values
	bool checkInitialCube(int leftUpX, int leftUpY, int leftUpZ, int *size, int *height);	
    
    //computes initial mean of foreground and background due to Chan-Vese
    void computeMean3D();
	
	//computes velocity field with respect to mean intensity in volume
	void computeVFMean3D(double foreground_weight, double background_weight);
	
	//computes velocity field by separating mean and variance of a volume
	void computeVFMeanVariance3D(double sigma_foreground, double sigma_background);

    int setVFFromRegionData(data::CRegionData* pData, bool bInverted);
	void setVFFromRegionData(data::CRegionData* pData, int activeRegion);

    // set velocity field through user provided function
    template <typename Fn> bool setVFCustom(Fn fn)
    {
        const vpl::tSize xSize = velocity_field_3D->getXSize();
        const vpl::tSize ySize = velocity_field_3D->getYSize();
        const vpl::tSize zSize = velocity_field_3D->getZSize();
        CProgress::tProgressInitializer StartProgress( *this );
        CProgress::setProgressMax( zSize );
        for( vpl::tSize k = 0; k < zSize; ++k )
        {
            if( !CProgress::progress() )
                return false;
#pragma omp parallel for
            for( vpl::tSize j = 0; j < ySize; ++j )
            {
                vpl::tSize idx = velocity_field_3D->getIdx(0, j, k);
                for( vpl::tSize i = 0; i < xSize; ++i, idx += velocity_field_3D->getXOffset())
                    velocity_field_3D->at(idx) = fn(i,j,k);
            }
        }
        return true;
    }	

    // holds custom function object for velocity field
#ifdef _NOSTDFUNCTION
    typedef int (*vfFN)(int,int,int,void*);
    vfFN m_pCustomFn;
	void* m_pCustomFnData;
#else
	std::function<int(int,int,int)> m_pCustomFn; // doesn't work on mac therefore we use a workaround
#endif
    
	// set custom function for velocity field (ie lambda function)
#ifdef _NOSTDFUNCTION
	bool setVFCustomFn(vfFN fn, void* pData)    
	{
		velocity_field_3D->fillEntire(UNKNOWN_VF);
        m_pCustomFn = fn;
		m_pCustomFnData = pData;
        return true;
	}
#else
    bool setVFCustomFn(std::function<int(int,int,int)> const& fn)
    {
        velocity_field_3D->fillEntire(UNKNOWN_VF);
        m_pCustomFn = fn;
        return true;
    }
#endif

    //! Apply VF modifier for given coordinates which are about to be accessed
    vpl::img::tPixel8 vfAt(vpl::tSize x, vpl::tSize y, vpl::tSize z);
    
	/********************************************/
	/******		EVOLUTION METHODS		*********/
	/********************************************/
		
	//add point to list of inner boundary lin
    inline void addToLin3D(const CoordinateComponent x, const CoordinateComponent y, const CoordinateComponent z);
	
	//add point to list of outter boundary lout
	inline void addToLout3D(const CoordinateComponent x, const CoordinateComponent y, const CoordinateComponent z);
    
    //updates phi function at specified point with given value
    inline int updatePhi3D(const CoordinateComponent x, const CoordinateComponent y, const CoordinateComponent z, const char val);
	
    //evolution of curve	
	void outwardEvolution3D();
	void inwardEvolution3D();
	
	//removing redundant points from boundary during the evolution
	void removeRedundantLin3D();
	void removeRedundantLout3D();
	
    //switch in
	CoordinateList::iterator switchIn3D(CoordinateList::iterator it);
    void switchIn3D(vpl::img::CCoordinates3<int>* coord, int index);
	
	//switch out
	CoordinateList::iterator switchOut3D(CoordinateList::iterator it);
    void switchOut3D(vpl::img::CCoordinates3<int>* coord, int index);
	
	//switches the neighborhood of point source from Lout to Lin
	void switchInNeighbors(vpl::img::CCoordinates3<int> *source);
	
	//switches the neighborhood of point source from Lin to Lout
	void switchOutNeighbors(vpl::img::CCoordinates3<int> *source);

    // checks sign
    bool neighbours3D(vpl::img::CCoordinates3<int> *source, vpl::img::CVolume<vpl::sys::tInt8>* volumePtr, int sign /*negative for inner*/);
	
	//Computes smoothed value of a given voxel by gauss kernel
	double returnSmoothPoint(const vpl::img::CCoordinates3<int>& point);
	
	//Smoths the interface of phi by a gauss kernel	
	void smoothStep3D();
	
	//prints the result phi
	void printPhi3D();
	
	//initial method for preparing and setting volumes that are needed
	bool prepareVolumes(vpl::img::CDensityVolumePtr, bool velocity, bool phi);
	
	//stopping critterion for automatic segmentation
	bool stoppingCriterion3D();
	
	//Manual evolution of model
	int manualEvolution(int iterations);
	
	//Automatic evolution of model
	int automaticEvolution(int maxIterations);

	//Automatic evolution of model
#ifdef _NOSTDFUNCTION
	template <class Fn> int automaticEvolution(int maxIterations, Fn fn)
#else
	int automaticEvolution(int maxIterations, const std::function <void (int)>& fn)
#endif
    {
        if (stop_iterations<=0) return 0;
            
        CProgress::tProgressInitializer StartProgress( *this );
        CProgress::setProgressMax( maxIterations );
            
        int iteration_count = 0;
        size_t outw = 0, inw = 0, rem = 0, smooth =0;
        int clkt = clock();
        int lastSmooth = 0;
        do
        {
            //after how many steps will be checked stoppingCriterion...after some iterations due to speed of overall computation
            for(int i=0;i < stop_iterations;i++)
            {
                if( !CProgress::progress() )
                {
                    return iteration_count;
                }
                /*===================================
                    **
                    **Step 3
                    **
                    **===================================
                    */
                int clk = clock();;
                if(outward)
                {
                    clk = clock();
                    outwardEvolution3D();
                    outw += clock()-clk;
                    clk = clock();
                    removeRedundantLin3D();
                    rem += clock()-clk;
                    clk = clock();
                }
                if(inward)
                {
                    clk = clock();
                    inwardEvolution3D();
                    inw += clock()-clk;
                    clk = clock();
                    removeRedundantLout3D();
                    rem += clock()-clk;
                    clk = clock();
                }
                //how many smoothing phases through evolution
                if(smoothing_enabled)
                {
                    clk = clock();
                    if(iteration_count % smoothing_steps == 0)
                    {
                        smoothStep3D();
                        lastSmooth = iteration_count;
                    }
                    smooth += clock()-clk;
                    clk = clock();
                }
                fn(iteration_count);
                ++iteration_count;
            }        			
        }while(iteration_count<maxIterations && !stoppingCriterion3D());
        // make one additional smoothing step
        if (smoothing_enabled && iteration_count-lastSmooth>smoothing_steps/2)
        {
            smoothStep3D();
        }
        //int total = clock()-clkt;
        //qDebug() << "In: " << inw << "Out: " << outw << "Rem: " << rem << "Smooth: " << smooth << "Total: " << total;
            
        return iteration_count;
    }
		
	//setting the smoothing
    void setSmoothing(bool value) { smoothing_enabled = value; }
	//is smoothing enabled
    bool isSmoothingEnabled(void) const { return smoothing_enabled; }
	//how many smoothing steps
    void setSmoothingSteps(int value) { smoothing_steps = value; }
	//sets iterations count
    void setIterations(int value) { iterations = value; }
	//clears the boundary-points
	void clearBoundary();
	
	//sets the directions of evolution
    void setInward(bool val) { inward = val; }
    void setOutward(bool val) { outward = val; } // outward goes first if both are set
	
	//sets how often will be the stopping criterion checked
    void setStoppingSteps(int count) { stop_iterations = count; }
	
	//returns number of l_out points
	int getBoundaryPointsCount() const;
	//return the pointer to the VF
    vpl::img::CVolume8Ptr getVelocityFieldPtr() const { return velocity_field_3D; }
	//return the pointer to the phi3D
    vpl::img::CVolume<vpl::sys::tInt8>::tSmartPtr getPhiFunctionPtr() const { return phi3D; }
	
	/********************************************/
	/******			LOCAL VARIABLES		*********/
	/********************************************/
public:
	//gauss kernel
	CGaussKernel kernel3D;
	
private:	
	//phi volume. Same size as volume
	vpl::img::CVolume<vpl::sys::tInt8>::tSmartPtr phi3D;
	//pointer to a processed volume
	vpl::img::CDensityVolumePtr ptrVolume;
	//velocity volume. Same size as volume
	vpl::img::CVolume8Ptr velocity_field_3D;		
		
	//list of inner boundary L_in representing LevelSet itself
	CoordinateList l_in3D;
	//list of outter boundary L_out representing LevelSet itself
	CoordinateList l_out3D;

    //synchronization for multithreaded list access
    QMutex  m_mutexIn3D,
            m_mutexOut3D;
	
	//mean intesities of foreground and background
	double foreground_mean, background_mean, foreground_count_old, background_count_old;
			
	bool smoothing_enabled;
	//automatic segmentation
	bool automatic;
	//manual segmntation
	bool manual;
	
	float u_sin_cos[2][790];
	float v_sin_cos[2][790];	    
	
private:	
	// outward evolution
	bool outward;
	// inward evolution
	bool inward;
	
	int iterations;
	int smoothing_steps;
	int stop_iterations;
};

#endif

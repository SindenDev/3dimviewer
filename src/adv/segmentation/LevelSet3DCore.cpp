///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// 3DimViewer
// Lightweight 3D DICOM viewer.
//
// Copyright 2008-2012 3Dim Laboratory s.r.o.
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

#include "math.h"
#define MAIN_STEP 0.008
#define MYIN 3
#define MYOUT 4
#include <segmentation/LevelSet3DCore.h>
#include <QDebug>


bool IsMarkedToDelete(const vpl::img::CCoordinates3<int> & o)
{
    return o.x()==-1 && o.y()==-1 && o.z()==-1;
}

CLevelSet3DCore::CLevelSet3DCore()
{
	outward = inward = true;	
	smoothing_enabled = false;	
    foreground_mean = background_mean = foreground_count_old = background_count_old = 0;
    stop_iterations = 1;
}

CLevelSet3DCore::~CLevelSet3DCore()
{
}

/*======================================================
** Initialize phi function to a given value
** 
**======================================================
*/
void CLevelSet3DCore::initializePhi3D(int value)
{
	phi3D->fillEntire((vpl::img::tDensityPixel)value);
}

/*======================================================
** Representation of the initial phi function as a Cube
** defined in Volume at specified coordinates
** For initialization cuold be used any shape.
**======================================================
*/
void CLevelSet3DCore::initialCube(vpl::img::CCoordinates3<int> &leftup, int size, int height)
{
	int leftup_x,leftup_y,leftup_z;
	int len = size;
	int h = height;
	
	leftup_x = leftup.getX();
	leftup_y = leftup.getY();
	leftup_z = leftup.getZ();
	
	//chcks cube if not crossing the volume to outside
	checkInitialCube(leftup_x, leftup_y , leftup_z, &len, &h);
		
	//=====================================================
	//boky
	
	for(int k=0;k<h;++k)
	{
		for(int i=0;i<len;++i)
		{
			addToLout3D(leftup_x+i, leftup_y,leftup_z + k);
			addToLout3D(leftup_x+i, leftup_y+size-1,leftup_z + k);
		}
		for(int i=0;i<len-2;++i)
		{
			addToLout3D(leftup_x, leftup_y+i+1,leftup_z + k);
			addToLout3D(leftup_x+len-1, leftup_y+i+1,leftup_z + k);
		}
	}
	//up and down podstava
	for(int k = 0;k <= h && h-1>0;k += (h-1))
	{
		for(int j=0; j<len-2;++j)
			for(int i=0; i<len-2;++i)
			{
				addToLout3D(leftup_x+i+1, leftup_y+1+j, leftup_z + k);
			}
	}
	//-----------------------------------
	//	LIN
	//-----------------------------------
	len -= 2;
	leftup_x +=1;
	leftup_y +=1;
	leftup_z +=1;
	h -=2;
	assert(len>0 && h>0);
	for(int k=0;k<h;++k)
	{
		
		for(int i=0;i<len;++i)
		{
			addToLin3D(leftup_x+i, leftup_y,leftup_z + k);
			addToLin3D(leftup_x+i, leftup_y+len-1,leftup_z + k);
		}
		for(int i=0;i<len-2;++i)
		{
			addToLin3D(leftup_x, leftup_y+i+1,leftup_z + k);
			addToLin3D(leftup_x+len-1, leftup_y+i+1,leftup_z + k);
		}
	}
	//up and down podstava
	for(int k = 0; k <=h && h-1>0; k+=(h-1))
	{
		for(int j=0;j<len-2;++j)
			for(int i=0;i<len-2;++i)
			{
				addToLin3D(leftup_x+i+1, leftup_y+1+j, leftup_z + k);
			}
	}
	
	//=====================================================

	//setting phi for interior points
	leftup_x += 1;
	leftup_y += 1;
	leftup_z += 1;
	len -=2;
	h-=2;
	
	for(int k=0;k<h;++k)
		for(int j=0;j<len;++j)
			for(int i=0;i<len;++i)
			{
				if(updatePhi3D(i+leftup_x,j+leftup_y,k+leftup_z,INNER_PHI) == -1)
					std::cerr << ("chyba");
			}

}

/*======================================================
** Checks if the initial cube is not crossing boundary
** of volumetric data. If so, makes it smaller
** Returns true if the size is OK.
**======================================================
*/
bool CLevelSet3DCore::checkInitialCube(int leftUpX, int leftUpY, int leftUpZ, int *size, int *height)
{
	bool ret = true;
	int diff;
		
	diff = (leftUpX + *size) - ptrVolume->getXSize();
	if(diff > 0)
	{
		*size -= diff;
		ret = false;
	}
	diff = (leftUpY + *size) - ptrVolume->getYSize();
	if(diff > 0)
	{
		*size -= diff;
		ret = false;
	}
	diff = (leftUpZ + *height) - ptrVolume->getZSize();
	if(diff > 0)
	{
		*height -= diff;
		ret = false;
	}	

	return ret;
}


/*======================================================
** Adds specified point to L_in3D list and updates its
** phi function
**
**======================================================
*/
inline void CLevelSet3DCore::addToLin3D(const CoordinateComponent x, const CoordinateComponent y, const CoordinateComponent z)
{
	if (INNER_BOUNDARY_PHI!=updatePhi3D(x,y,z, INNER_BOUNDARY_PHI))
    {
        m_mutexIn3D.lock();
        l_in3D.push_back(vpl::img::CCoordinates3<int>(x,y,z));
        m_mutexIn3D.unlock();
    }
}

/*======================================================
** Adds specified point to L_out3D list and updates its 
** phi function
**
**======================================================
*/
inline void CLevelSet3DCore::addToLout3D(const CoordinateComponent x, const CoordinateComponent y, const CoordinateComponent z)
{	
	if (OUTTER_BOUNDARY_PHI!=updatePhi3D(x,y,z, OUTTER_BOUNDARY_PHI))
    {
        m_mutexOut3D.lock();
        l_out3D.push_back(vpl::img::CCoordinates3<int>(x,y,z));
        m_mutexOut3D.unlock();
    }
}


/*======================================================
** Updates phi function on proper location with given value
**
**======================================================
*/
inline int CLevelSet3DCore::updatePhi3D(const CoordinateComponent x, const CoordinateComponent y, const CoordinateComponent z, const char val)
{
	//check boundaries    
	if(x >= phi3D->getXSize() ||
	   y >= phi3D->getYSize() ||
	   z >= phi3D->getZSize() ||
       x < 0 || y < 0 || z < 0)
			return -1;

    char res = phi3D->at(x,y,z);
    if (res!=val)
	    phi3D->set(x,y,z,val);
	return res;
}

/*======================================================
** Computes mean intensity of foreground and background
** Used to compute Velocity Field
**======================================================
*/
void CLevelSet3DCore::computeMean3D()
{
	
	double sum_foreground, sum_background;
	
	foreground_count_old = 0;
	background_count_old = 0;
	
	sum_foreground=sum_background=0;

    const vpl::tSize xSize = phi3D->getXSize();
    const vpl::tSize ySize = phi3D->getYSize();
    const vpl::tSize zSize = phi3D->getZSize();
    for( vpl::tSize k = 0; k < zSize; ++k )
    {
        for( vpl::tSize j = 0; j < ySize; ++j )
        {
            vpl::tSize idx = phi3D->getIdx(0, j, k);
            vpl::tSize idxVolume = ptrVolume->getIdx(0, j, k);
            for( vpl::tSize i = 0; i < xSize; ++i, idx += phi3D->getXOffset(), idxVolume += ptrVolume->getXOffset())
            {
                vpl::img::tDensityPixel val = phi3D->at(idx);
		        if(val == INNER_BOUNDARY_PHI || val == INNER_PHI)
		        {
			        ++foreground_count_old;
			        sum_foreground += ptrVolume->at(idxVolume);
		        }
		        else
		        {
			        ++background_count_old;
			        sum_background += ptrVolume->at(idxVolume);
		        }                
            }
        }
    }
	
	//compute mean
	foreground_mean = sum_foreground/foreground_count_old;
	background_mean = sum_background/background_count_old;
		
}


/*======================================================
** Computes Velocity Field by separating mean intensity 
** of foreground and backgound of the Volume
**
** Output is Density Volume containf Velocity binary field
**======================================================
*/
void CLevelSet3DCore::computeVFMean3D(double foreground_weight, double background_weight)
{
    const vpl::tSize xSize = velocity_field_3D->getXSize();
    const vpl::tSize ySize = velocity_field_3D->getYSize();
    const vpl::tSize zSize = velocity_field_3D->getZSize();
    for( vpl::tSize k = 0; k < zSize; ++k )
    {
        for( vpl::tSize j = 0; j < ySize; ++j )
        {
            vpl::tSize idx = velocity_field_3D->getIdx(0, j, k);
            vpl::tSize idxVolume = ptrVolume->getIdx(0, j, k);
            for( vpl::tSize i = 0; i < xSize; ++i, idx += phi3D->getXOffset(), idxVolume += ptrVolume->getXOffset())
            {
                vpl::img::CDensityVolume::tVoxel voxel = ptrVolume->at(idxVolume);
		        double a = voxel*voxel;
		        double f = voxel + voxel;
			
		        if((a- background_mean*(f - background_mean))*background_weight - 
			        (a- foreground_mean*(f - foreground_mean))*foreground_weight >= 0)
			        velocity_field_3D->at(idx) = FOREGROUND_VF;
		        else
			        velocity_field_3D->at(idx) = BACKGROUND_VF;
            }
        }
    }
}


/*======================================================
** Computes Velocity Field by separating mean and 
** variance intensity of foreground and backgound of the Volume
**
** Output is Density Volume contains Velocity binary field
**======================================================
*/
void CLevelSet3DCore::computeVFMeanVariance3D(double sigma_foreground, double sigma_background)
{
	double b = foreground_mean*foreground_mean;
	double c = background_mean*background_mean;
		
	double d = 1/(sigma_foreground*sigma_foreground);
	double e = 1/(sigma_background*sigma_background);
	
	double f = b*d;
	double g = c*e;
	
	double h = log(sigma_background*sigma_background/sigma_foreground*sigma_foreground);

    const vpl::tSize xSize = velocity_field_3D->getXSize();
    const vpl::tSize ySize = velocity_field_3D->getYSize();
    const vpl::tSize zSize = velocity_field_3D->getZSize();
    for( vpl::tSize k = 0; k < zSize; ++k )
    {
        for( vpl::tSize j = 0; j < ySize; ++j )
        {
            vpl::tSize idx = velocity_field_3D->getIdx(0, j, k);
            vpl::tSize idxVolume = ptrVolume->getIdx(0, j, k);
            for( vpl::tSize i = 0; i < xSize; ++i, idx += phi3D->getXOffset(), idxVolume += ptrVolume->getXOffset())
            {
                vpl::img::CDensityVolume::tVoxel voxel = ptrVolume->at(idxVolume);
		        double a = voxel*voxel;
			
		        if((e*(a- (voxel<<2)*background_mean) + g - (d*(a- (voxel<<2)*foreground_mean) + f) + h) >= 0)
			        velocity_field_3D->at(idx) = FOREGROUND_VF;
		        else
			        velocity_field_3D->at(idx) = BACKGROUND_VF;
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////

void CLevelSet3DCore::initalizeFromRegionData(data::CRegionData* pData, int activeRegion)
{
    if (NULL==pData) return;
    const vpl::tSize xSize = pData->getXSize();
    const vpl::tSize ySize = pData->getYSize();
    const vpl::tSize zSize = pData->getZSize();
#pragma omp parallel for
    for( vpl::tSize k = 0; k < zSize; ++k )
    {
        for( vpl::tSize j = 0; j < ySize; ++j )
        {
            vpl::tSize idxReg = pData->getIdx(0,j,k);
            for( vpl::tSize i = 0; i < xSize; ++i, idxReg += pData->getXOffset())
            {
                data::CRegionData::tVoxel voxel = pData->at(idxReg);
                // get count of active region points around
                int nActive = 0;
                for(int dz = -1; dz<=1; dz++)
                {
                    if (k+dz<0) continue;
                    if (k+dz>=zSize) break;
                    for(int dy = -1; dy<=1; dy++)
                    {
                        if (j+dy<0) continue;
                        if (j+dy>=ySize) break;
                        for(int dx = -1; dx<=1; dx++)
                        {
                            if (i+dx<0) continue;
                            if (i+dx>=xSize) break;
                            data::CRegionData::tVoxel voxel = pData->at(i+dx,j+dy,k+dz);
                            if (voxel==activeRegion)
                                nActive++;
                        }
                    }
                }
                // is active region?
                if (voxel == activeRegion)
                {
                    // boundary or inner
                    if (nActive<8) 
                        addToLin3D(i, j, k);
                    else
                        updatePhi3D(i,j,k,INNER_PHI);
                }
                else
                {
                    // outer boundary if has one near
                    if (nActive>0)
                        addToLout3D(i, j, k);
                }
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////

int CLevelSet3DCore::setVFFromRegionData(data::CRegionData* pData, bool bWantDataWithNoRegion)
{
    const vpl::tSize xSize = velocity_field_3D->getXSize();
    const vpl::tSize ySize = velocity_field_3D->getYSize();
    const vpl::tSize zSize = velocity_field_3D->getZSize();
    int gotSome = 0;
#pragma omp parallel for
    for( vpl::tSize k = 0; k < zSize; ++k )
    {
        for( vpl::tSize j = 0; j < ySize; ++j )
        {
			vpl::tSize idxReg = pData->getIdx(0,j,k);
            vpl::tSize idx = velocity_field_3D->getIdx(0, j, k);
            for( vpl::tSize i = 0; i < xSize; ++i, idx += phi3D->getXOffset(), idxReg += pData->getXOffset())
            {
				data::CRegionData::tVoxel voxel = pData->at(idxReg);
                if (voxel>0)
                {
                    if (!gotSome)
                    {
#pragma omp critical
                        gotSome=1;
                    }
					velocity_field_3D->at(idx) = bWantDataWithNoRegion? BACKGROUND_VF:FOREGROUND_VF;
                }
				else
					velocity_field_3D->at(idx) = bWantDataWithNoRegion? FOREGROUND_VF:BACKGROUND_VF;
            }
        }
    }
    return gotSome;
}

void CLevelSet3DCore::setVFFromRegionData(data::CRegionData* pData, int activeRegion)
{
    const vpl::tSize xSize = velocity_field_3D->getXSize();
    const vpl::tSize ySize = velocity_field_3D->getYSize();
    const vpl::tSize zSize = velocity_field_3D->getZSize();
#pragma omp parallel for
    for( vpl::tSize k = 0; k < zSize; ++k )
    {
        for( vpl::tSize j = 0; j < ySize; ++j )
        {
			vpl::tSize idxReg = pData->getIdx(0,j,k);
            vpl::tSize idx = velocity_field_3D->getIdx(0, j, k);
            for( vpl::tSize i = 0; i < xSize; ++i, idx += phi3D->getXOffset(), idxReg += pData->getXOffset())
            {
				data::CRegionData::tVoxel voxel = pData->at(idxReg);
				if (voxel==activeRegion)
					velocity_field_3D->at(idx) = FOREGROUND_VF;
				else
					velocity_field_3D->at(idx) = BACKGROUND_VF;
            }
        }
    }
}

//! Apply VF modifier for given coordinates which are about to be accessed
vpl::img::tPixel8 CLevelSet3DCore::vfAt(vpl::tSize x, vpl::tSize y, vpl::tSize z)
{
    if (!m_pCustomFn)
        return velocity_field_3D->at(x,y,z);
    vpl::img::tPixel8 val = velocity_field_3D->at(x,y,z);
    if (UNKNOWN_VF==val)
    {
#ifdef _NOSTDFUNCTION
		velocity_field_3D->at(x,y,z) = val = m_pCustomFn(x,y,z,m_pCustomFnData);
#else
        velocity_field_3D->at(x,y,z) = val = m_pCustomFn(x,y,z);
#endif
        if (UNKNOWN_VF==val) // couldn't determine, check neighbourhood
        {
#define NEIGHB  1
            int nForeground = 0;
            int nBackground = 0;
            const vpl::tSize xSize = velocity_field_3D->getXSize();
            const vpl::tSize ySize = velocity_field_3D->getYSize();
            const vpl::tSize zSize = velocity_field_3D->getZSize();
            for(int dz = -NEIGHB; dz<=NEIGHB; dz++)
            {
                if (z+dz<0) continue;
                if (z+dz>=zSize) break;
                for(int dy = -NEIGHB; dy<=NEIGHB; dy++)
                {
                    if (y+dy<0) continue;
                    if (y+dy>=ySize) break;
                    for(int dx = -NEIGHB; dx<=NEIGHB; dx++)
                    {
                        if (x+dx<0) continue;
                        if (x+dx>=xSize) break;
                        vpl::img::tPixel8 val = velocity_field_3D->at(x+dx,y+dy,z+dz);
                        if (val==BACKGROUND_VF)
                            nBackground++;
                        else
                            if (val==FOREGROUND_VF)
                                nForeground++;
                    }
                }
            }
            if (nForeground>nBackground)
                velocity_field_3D->at(x,y,z) = val = FOREGROUND_VF;
            else
                velocity_field_3D->at(x,y,z) = val = BACKGROUND_VF;
#undef NEIGHB
        }
    }
    return val;
}

/*======================================================
** Executes outward evolution of model-expanding of the surface
** background of the Volume
**
**====================================================== 
*/
void CLevelSet3DCore::outwardEvolution3D()
{
    const int size = l_out3D.size();
#pragma omp parallel for
    for(int i = 0 ; i < size; i++)
    {
        vpl::img::CCoordinates3<int> coord = l_out3D[i];
        if(vfAt(coord.getX(),coord.getY() ,coord.getZ()) == FOREGROUND_VF)
        {
            switchIn3D(&coord,i);
        }
    }
    l_out3D.erase( std::remove_if(l_out3D.begin(), l_out3D.end(), IsMarkedToDelete), l_out3D.end());
}

/*======================================================
** Switches point from Lout interface to Lin
** 
**======================================================
*/
CoordinateList::iterator CLevelSet3DCore::switchIn3D(CoordinateList::iterator it)
{
	
	//===========step 1.=============
	addToLin3D(it->getX(), it->getY(), it->getZ());
	//===========step 2.=============
	switchInNeighbors(&(*it));
	
	return l_out3D.erase(it);
}

void CLevelSet3DCore::switchIn3D(vpl::img::CCoordinates3<int>* coord, int index)
{
    addToLin3D(coord->getX(), coord->getY(), coord->getZ());	
	switchInNeighbors(coord);
    l_out3D[index].setXYZ(-1,-1,-1);
}

/*======================================================
** Method for switching neighbors of given point
**
**====================================================== 
*/
void CLevelSet3DCore::switchInNeighbors(vpl::img::CCoordinates3<int> *source)
{
	//img coordinate are the same as phi coordinates
	//up
	 if((source->getY()-1) >= 0)
	 {
		if(phi3D->at(source->getX(),source->getY()-1,source->getZ()) == OUTTER_PHI)
		{
			addToLout3D(source->getX(),source->getY()-1,source->getZ());
		}
	 }
	 //down
	 if((source->getY() + 1) < phi3D->getYSize())
	 {
		if(phi3D->at(source->getX(),source->getY()+1,source->getZ()) == OUTTER_PHI)
		{
				addToLout3D(source->getX(),source->getY()+1,source->getZ());
		}
		
	 }
	 //left
	 if((source->getX()-1) >= 0)
	 {
		if(phi3D->at(source->getX()-1,source->getY(),source->getZ()) == OUTTER_PHI)
		{
				addToLout3D(source->getX()-1,source->getY(),source->getZ());
		}
	 }
	 //right
	 if((source->getX()+1) < phi3D->getXSize())
	 {
 		if(phi3D->at(source->getX()+1,source->getY(),source->getZ()) == OUTTER_PHI)
 		{
 				addToLout3D(source->getX()+1,source->getY(),source->getZ());
		}
	 }
	 //z
	 //up
	 if((source->getZ()+1) < phi3D->getZSize())
	 {
		if(phi3D->at(source->getX(),source->getY(),source->getZ()+1) == OUTTER_PHI)
 		{
				addToLout3D(source->getX(),source->getY(),source->getZ()+1);
		}
	 } 
	 //down
	 if((source->getZ()-1) >= 0)
	 {
		if(phi3D->at(source->getX(),source->getY(),source->getZ()-1) == OUTTER_PHI)
		{
				addToLout3D(source->getX(),source->getY(),source->getZ()-1);
		}
	 }

}
/*======================================================
** Does outward evolution of model-expanding of the surface
** backgound of the Volume
**
**======================================================
*/
void CLevelSet3DCore::inwardEvolution3D()
{
    const int size = l_in3D.size();
#pragma omp parallel for
    for(int i = 0 ; i < size; i++)
    {
        vpl::img::CCoordinates3<int> coord = l_in3D[i];
        if(vfAt(coord.getX(),coord.getY() ,coord.getZ()) == BACKGROUND_VF)
        {
            switchOut3D(&coord,i);
        }
    }
    l_in3D.erase( std::remove_if(l_in3D.begin(), l_in3D.end(), IsMarkedToDelete), l_in3D.end());
}
/*======================================================
** Switches point from Lin interface to Lout
** 
**======================================================
*/
CoordinateList::iterator CLevelSet3DCore::switchOut3D(CoordinateList::iterator it)
{
	//===========step 1.=============
	addToLout3D(it->getX(), it->getY(), it->getZ());
	//===========step 2.=============
	//switch neighbors from outside of phi to lout interface
	switchOutNeighbors(&(*it));
	
	return l_in3D.erase(it);	
}

void CLevelSet3DCore::switchOut3D(vpl::img::CCoordinates3<int>* coord, int index)
{
    addToLout3D(coord->getX(), coord->getY(), coord->getZ());	
	switchOutNeighbors(coord);
    l_in3D[index].setXYZ(-1,-1,-1);
}

/*======================================================
** Method for switching neighbors of given point
** 
**======================================================
*/
void CLevelSet3DCore::switchOutNeighbors(vpl::img::CCoordinates3<int> *source)
{
	//up
	 if((source->getY()-1) >= 0)
	 {
		if(phi3D->at(source->getX(),source->getY()-1,source->getZ()) == INNER_PHI)
		{
				addToLin3D(source->getX(),source->getY()-1,source->getZ());
		}
	 }
	 //down
	 if((source->getY() + 1) < phi3D->getYSize())
	 {
		if(phi3D->at(source->getX(),source->getY()+1,source->getZ()) == INNER_PHI)
		{
				addToLin3D(source->getX(),source->getY()+1,source->getZ());
		}
		
	 }
	 //left
	 if((source->getX()-1) >= 0)
	 {
		if(phi3D->at(source->getX()-1,source->getY(),source->getZ()) == INNER_PHI)
		{
				addToLin3D(source->getX()-1,source->getY(),source->getZ());
		}
	 }
	 //right
	 if((source->getX()+1) < phi3D->getXSize())
	 {
 		if(phi3D->at(source->getX()+1,source->getY(),source->getZ()) == INNER_PHI)
 		{
				addToLin3D(source->getX()+1,source->getY(),source->getZ());
		}
	 }
	 //z
	 //up
	 if((source->getZ()+1) < phi3D->getZSize())
	 {
		if(phi3D->at(source->getX(),source->getY(),source->getZ()+1) == INNER_PHI)
 		{
				addToLin3D(source->getX(),source->getY(),source->getZ()+1);
		}
	 } 
	 //down
	 if((source->getZ()-1) >= 0)
	 {
		if(phi3D->at(source->getX(),source->getY(),source->getZ()-1) == INNER_PHI)
		{
				addToLin3D(source->getX(),source->getY(),source->getZ()-1);
		}
	 }

}
/*======================================================
** Method for removing standalone points which represented the bounddary in previous step
** 
**======================================================
*/
void CLevelSet3DCore::removeRedundantLin3D()
{
    const int size = l_in3D.size();
#pragma omp parallel for
    for(int i = 0 ; i < size; i++)
    {
        vpl::img::CCoordinates3<int> coord = l_in3D[i];                
        //condition of removing
        if(neighbours3D(&coord,phi3D,-1))
		{
			updatePhi3D(coord.getX(), coord.getY(),coord.getZ(), INNER_PHI);
            l_in3D[i].setXYZ(-1,-1,-1);
        }
    }
    l_in3D.erase( std::remove_if(l_in3D.begin(), l_in3D.end(), IsMarkedToDelete), l_in3D.end());
}


template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

bool CLevelSet3DCore::neighbours3D(vpl::img::CCoordinates3<int> *source, vpl::img::CVolume<vpl::sys::tInt8>* volumePtr, int sign /*negative*/)
{
    const int x = source->getX();
    const int y = source->getY();
    const int z = source->getZ();
	
	if((y-1) >= 0)
	{	
		if (sgn(volumePtr->at(x,y-1,z))!=sign)
            return false;
	}
	//down
	if((y + 1) < volumePtr->getYSize())
	{	
        if (sgn(volumePtr->at(x,y+1,z))!=sign)
            return false;
	}
	//left
	if((x-1) >= 0)
	{	
        if (sgn(volumePtr->at(x-1,y,z))!=sign)
            return false;
	}
	//right
	if((source->getX()+1) < volumePtr->getXSize())
	{	
        if (sgn(volumePtr->at(x+1,y,z))!=sign)
            return false;
	 }
	 //z
	 //up
	 if((z+1) < volumePtr->getZSize())
	 {	
        if (sgn(volumePtr->at(x,y,z+1))!=sign)
            return false;
	 }
	 //down
	 if((z-1) >= 0)
	 {	
		if (sgn(volumePtr->at(x,y,z-1))!=sign)
            return false;
	 }
	return true;
}

/*======================================================
** Method for removing standalone points which represented the bounddary in previous step
** 
**======================================================
*/
void CLevelSet3DCore::removeRedundantLout3D()
{
    const int size = l_out3D.size();
#pragma omp parallel for
    for(int i = 0 ; i < size; i++)
    {
        vpl::img::CCoordinates3<int> coord = l_out3D[i];   
        if(neighbours3D(&coord,phi3D,1))
		{
			updatePhi3D(coord.getX(), coord.getY(),coord.getZ(), OUTTER_PHI);
            l_out3D[i].setXYZ(-1,-1,-1); // tag as deleted
        }
    }
    // erase all tagged elements from the list
    l_out3D.erase( std::remove_if(l_out3D.begin(), l_out3D.end(), IsMarkedToDelete), l_out3D.end());
}

/*======================================================
** Computes smoothed value of a given voxel by gauss kernel
** 
**======================================================
*/
double CLevelSet3DCore::returnSmoothPoint(const vpl::img::CCoordinates3<int> &point)
{
	double smoothed_point = 0;
	
	const int dX = point.getX() - kernel3D.size_half;
	const int dY = point.getY() - kernel3D.size_half;
	const int dZ = point.getZ() - kernel3D.size_half;
	
		//cycle for all points which kernel hits within the image..except those whose kernel is out of volume boundaries
	for(int x = 0; x < kernel3D.size; ++x)			
    {
        if(dX + x < 0 || dX + x >= ptrVolume->getXSize())
            continue;
		for(int y = 0;y < kernel3D.size; y++)		
        {
            if(dY + y < 0 || dY + y >= ptrVolume->getYSize())
                continue;
			for(int z = 0; z < kernel3D.size; ++z)
			{
				if(dZ + z < 0 || dZ + z >= ptrVolume->getZSize())
                    continue;
                smoothed_point += kernel3D.kernel_3D[x][y][z] * phi3D->at(dX + x, dY + y, dZ + z);
			}
        }
    }
			
	return smoothed_point;

}
/*======================================================
** Smoths the interface of phi by a gauss kernel
** 
**======================================================
*/
void CLevelSet3DCore::smoothStep3D()
{
	//smoothing
    
    int size = l_in3D.size();
#pragma omp parallel for
    for(int i = 0 ; i < size; i++)
    {
        m_mutexIn3D.lock();
        vpl::img::CCoordinates3<int> coord = l_in3D[i];
        m_mutexIn3D.unlock();
        double result = returnSmoothPoint(coord);
        int phi_value = phi3D->at(coord.getX(),coord.getY(),coord.getZ());
        if((result < 0 && (phi_value == OUTTER_BOUNDARY_PHI || phi_value == OUTTER_PHI))
			|| (result > 0 && (phi_value == INNER_BOUNDARY_PHI || phi_value == INNER_PHI)))
		{
			switchOut3D(&coord,i);
		}

    }
    l_in3D.erase( std::remove_if(l_in3D.begin(), l_in3D.end(), IsMarkedToDelete), l_in3D.end());
	
	removeRedundantLout3D();
		
    size = l_out3D.size();
#pragma omp parallel for
    for(int i = 0 ; i < size; i++)
    {
        m_mutexOut3D.lock();
        vpl::img::CCoordinates3<int> coord = l_out3D[i];
        m_mutexOut3D.unlock();
        double result = returnSmoothPoint(coord);
        int phi_value = phi3D->at(coord.getX(),coord.getY(),coord.getZ());
        if((result < 0 && (phi_value == OUTTER_BOUNDARY_PHI || phi_value == OUTTER_PHI))
			|| (result > 0 && (phi_value == INNER_BOUNDARY_PHI || phi_value == INNER_PHI)))
		{
			switchIn3D(&coord,i);
		}

    }
    l_out3D.erase( std::remove_if(l_out3D.begin(), l_out3D.end(), IsMarkedToDelete), l_out3D.end());
	
	removeRedundantLin3D();	
}
/*======================================================
** Prints the phi interface to the volume
** 
**======================================================
*/
void CLevelSet3DCore::printPhi3D()
{
	CoordinateList::iterator it;
	
	for(it=l_out3D.begin();it != l_out3D.end();++it)
		ptrVolume->set(it->getX(), it->getY(),it->getZ(),1000);
							
	for(it=l_in3D.begin();it != l_in3D.end();++it)
		ptrVolume->set(it->getX(), it->getY(),it->getZ(),300);
		
		
		
}
/*======================================================
** prepares volumes like velocity field, phi for evolution
** 
**======================================================
*/
bool CLevelSet3DCore::prepareVolumes(vpl::img::CDensityVolumePtr spVolume, bool velocity, bool phi)
{
	if(velocity)
	{
		vpl::img::CVolume8Ptr vf(new vpl::img::CVolume8(spVolume->getXSize(),
														spVolume->getYSize(),
														spVolume->getZSize()));
		velocity_field_3D = vf;
	}
	
	if(phi)
	{
		//create phi signed function
		vpl::img::CVolume<vpl::sys::tInt8>::tSmartPtr ph(new vpl::img::CVolume<vpl::sys::tInt8>(spVolume->getXSize(),
																	spVolume->getYSize(),
																	spVolume->getZSize()));	
		phi3D = ph;
	}																
	
	ptrVolume = spVolume;

	return true;
}
/*======================================================
** Criterion which decides if the evolution is done or not
** 
**======================================================
*/
bool CLevelSet3DCore::stoppingCriterion3D()
{
	//smoothing constant stopping criterion
	double constant = 0.001;
	
	CoordinateList::iterator it;
	CoordinateList::iterator itEnd;
	
	itEnd = l_out3D.end();
	it = l_out3D.begin();
	
	int count = 0;
	
	//-------------------------------
	for(; it != itEnd; ++it)
	{
        if(vfAt(it->getX(),it->getY(),it->getZ()) == BACKGROUND_VF)
			++count;
	}
	
	if((l_out3D.size() - count) < l_out3D.size()*constant)
	{	
		count = 0;
		
		it = l_in3D.begin();
		itEnd = l_in3D.end();
		
		for(; it != itEnd; ++it)
		{
			if(vfAt(it->getX(),it->getY(),it->getZ()) == FOREGROUND_VF)
				++count;
		}
		
		if((l_in3D.size() - count) < l_in3D.size()*constant)
			return true;
		else
			return false;	
	}
	else
		return false;
	
}
/*======================================================
** Loop of manual evolution with given number of iteration and count of smoothing steps
** 
*======================================================
*/
int CLevelSet3DCore::manualEvolution(int iter)
{
	int count = 0;
	for(int i=0;i<iter;++i)
	{			
			/*===================================
			**Step 3
			**===================================
			*/
			if(outward)
			{
				outwardEvolution3D();
				removeRedundantLin3D();
			}
			if(inward)		
			{
				inwardEvolution3D();
				removeRedundantLout3D();
			}
			
			//makes screnshots of model	
			/*if(count % 20 == 0)
				printPhi3D();*/
			//smoothing
			if(smoothing_enabled)
			{
				if(count % smoothing_steps == 0)
					smoothStep3D();
			}
			++count;
			
			
	}
	return count;				
}
/*======================================================
** Loop of automatic evolution. Until stopping criterion is satisfied
** 
**======================================================
*/
int CLevelSet3DCore::automaticEvolution(int maxIterations)
{
	return automaticEvolution(maxIterations,[](int v){});
}

/*========================================================================
*Clears the boundary of interface
*========================================================================
*/
void CLevelSet3DCore::clearBoundary()
{
	l_in3D.clear();
	l_out3D.clear();
}

/*========================================================================
*Returns number of boudary points form lout
*========================================================================
*/

int CLevelSet3DCore::getBoundaryPointsCount() const
{
	return l_out3D.size();
}


void CLevelSet3DCore::sphere(int radius, vpl::img::CCoordinates3<int> center,int list)
{
	int xDim,yDim,zDim;	
	xDim = ptrVolume->getXSize();
	yDim = ptrVolume->getYSize();
	zDim = ptrVolume->getZSize();	
	
	int offset=1; // radius > 40
	if(radius <= 10)
	{	
		offset = 6;
	}
	else if(radius <= 40)
	{
		offset = 3;
	}
    double step = offset*MAIN_STEP;
	//---------------------------------------
	
	
	int centerx, centery, centerz;	
	centerx = center.getX();
	centery = center.getY();
	centerz = center.getZ();
	
	double halfPi = vpl::math::PI/2;
	double v;
	
	int x,y,z;
	double radcos;
	
	//naplnenie pola-----------
	
	int i,j;
	i=j=0;
	//-------------kruznica----------------------------------	
	for(double u = 0; u < vpl::math::TWO_PI; u += step, i+=offset)
	{
		j =0;
		for(v = -1*halfPi; v <= halfPi;v += step,j+=offset)
		{
			radcos = radius*(v_sin_cos[1][j]);
			
			x = int(radcos*(u_sin_cos[1][i])) + centerx;
			y = int(radcos*(u_sin_cos[0][i])) + centery;
			z = int(radius*(v_sin_cos[0][j])) + centerz;
		
			if(list == MYIN)
			{	
				if(x >= xDim-1)
					x = xDim-2;
				if(y >= yDim-1)
					y = yDim-2;
				if(z >= zDim-1)
					z = zDim-2;
					
				if(x < 1)
					x = 1;
				if(y < 1)
					y = 1;
				if(z < 1)
					z = 1;			
				
				addToLin3D(x,y,z);
				
			}
			else if(list == MYOUT)
			{
				if(x >= xDim)
					x = xDim-1;
				if(y >= yDim)
					y = yDim-1;
				if(z >= zDim)
					z = zDim-1;
					
				if(x < 0)
					x = 0;
				if(y < 0)
					y = 0;
				if(z < 0)
					z = 0;			
				
				addToLout3D(x,y,z);
			}
		}
	}
	//------------vnutro---------------------
	if(list == MYIN)
	{
		int rad2,k2,j2,i2;
		rad2 = (radius-1)*(radius-1);
		
		for(int k=-radius;k<radius;++k)
		{
			k2 = k*k;
			for(int j=-radius;j<radius;++j)
			{
				j2 = j*j;
				for(int i=-radius;i<radius;++i)
				{
					i2=i*i;
					if(i2+j2+k2 < rad2)
					{
						x = centerx+i;
						y = centery+j;
						z = centerz+k;	
						
							if(x >= xDim-2)
								x = xDim-3;
							if(y >= yDim-2)
								y = yDim-3;
							if(z >= zDim-2)
								z = zDim-3;
								
							if(x < 2)
								x = 2;
							if(y < 2)
								y = 2;
							if(z < 2)
								z = 2;
						
						updatePhi3D(x,y,z,INNER_PHI);
						
					}	
				}
			}
		}
	}
}

void CLevelSet3DCore::fillTrigonometric()
{
	int i=0;
	double halfPi = vpl::math::PI/2;
	//------fill----------------------------
	for(double u = 0; u < vpl::math::TWO_PI; u += MAIN_STEP, ++i)
	{
		u_sin_cos[0][i] = sin(u);
		u_sin_cos[1][i] = cos(u);
	}
	i =0;
	for(double v = -1*halfPi; v <= halfPi;v += MAIN_STEP,++i)
	{
		v_sin_cos[0][i] = sin(v);
		v_sin_cos[1][i] = cos(v);
	}
}

void CLevelSet3DCore::InitialSphere(int radius, vpl::img::CCoordinates3<int> center)
{
	fillTrigonometric();
	//outter boundary
	sphere(radius, center, MYOUT);
	//inner boundary
	sphere(radius-1, center, MYIN);
}

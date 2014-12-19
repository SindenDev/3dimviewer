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


#include <segmentation/GaussKernel.h>
#include <math.h>
#include <VPL/Module/Module.h>

CGaussKernel::CGaussKernel()
{
	kernel_3D = NULL;
}

CGaussKernel::~CGaussKernel()
{
}

/*
* - Gaussian filter kernel is defined by the following function: \n
*                         1                         (x^2 + y^2 + z^2)  \n
*   G_sigma(x,y,z) = ---------------------- * exp(- -----------------) \n
*                    (2 * pi)^3/2 * sigma^3            2 * sigma^2     \n
*/
void CGaussKernel::computeKernel(int kernel_size, double sigma)
{
	size = kernel_size;
	size_half = size /2;
	
	//3 dimensional kernel
	
	static const double dConst = 1.0 / pow(vpl::math::TWO_PI, 1.5);

    // Helper values
    double dS3 = 1.0 / (sigma * sigma * sigma);
    double dS2 = dS3 * sigma;
    double dA = dConst * dS3;
    double dB = -0.5 * dS2;
       
	kernel_3D = new double **[size];
	for(int x = 0; x < size; ++x){
		kernel_3D[x] =  new double *[size];
		for(int y = 0;y < size; y++){
			kernel_3D[x][y] = new double[size];
			for(int z = 0; z < size; ++z){
					
				;
			}
		}
	}
	
    //---------------------------------------------------------
	for( int k = -size_half; k <= size_half; ++k )
	{
		for( int j = -size_half; j <= size_half; ++j )
		{
			for( int i = -size_half; i <= size_half; ++i )
			{
				double dValue = dA * exp(dB * (i * i + j * j + k * k));
				kernel_3D[i + size_half][ j + size_half][ k + size_half] = dValue;
			}
		}
    }
}

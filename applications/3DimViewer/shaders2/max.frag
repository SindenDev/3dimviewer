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

#ifndef __GLSL_CG_DATA_TYPES
#define saturate(type) clamp(type,0.0,1.0)
#endif

uniform sampler3D t3D;      // 3D texture containing data 
uniform sampler1D LookUp;   // color and transparency look-up texture 
uniform sampler2D Osr;      // Off-Screen render texture with back-facing polygon data 
uniform sampler2D OsrWC;    // back-facing world coords 
uniform sampler2D Noise;    // Noise offset used to remove aliasing [replacable by internal GLSL noise generator]
uniform sampler2D Depth;
 
varying vec3 worldCoords; 

uniform vec2 inputAdjustment;  
uniform vec2 imageAdjustment; 
uniform vec3 tResolution; 
uniform vec2 wSize;             // Viewport size 1/X and 1/Y 
uniform vec4 pl;                // plane that cuts the volume 
uniform float textureSampling;  // 3D texture sampling rate

uniform sampler3D tSkip3D;      // skipping 3D texture 
uniform vec3 skipTexSize; 
uniform vec3 tSkipResolution; 

 
// Compute position against the cutting plane 
float cuttingPlane(vec3 pos) 
{ 
    return (pl.x * pos.x + pl.y * pos.y + pl.z * pos.z + pl.w);  
} 
 
// Compute plane intersection 
float planeCut(vec3 pos, float value, float defvalue) 
{  
    return (cuttingPlane(pos) > 0 ? value : defvalue); 
} 
 
// Use the skipping volume to evaluate the given position 
int isMeaningful(vec3 pos) 
{
    // is the voxel otside of the cutting plane
    if( cuttingPlane(pos) < 0 ) 
    { 
        return 0; 
    } 
    
    // sample a voxel at initial position
    vec4 voxel = texture3D(tSkip3D, pos * skipTexSize); 
    
    // minimum and maximum voxel value  
    float vmax = voxel.g;
    
    // is the position empty (Hu < -200)?
    return (vmax < 0.15) ? 0 : 1; 
} 
 
// Compute length of the step in the 3D texture 
float stepLength(vec3 path) 
{ 
    // All converted to the first quadrant 
    vec3 aPath = abs(path); 

    // to be explained later 
    // basically computes line/plane intersection 
    vec3 t = tResolution.xyz / (aPath+1e-20); 
    float d2x = length(aPath*t.x); // intersection with a plane perpendicular to the X axis 
    float d2y = length(aPath*t.y); // intersection with a plane perpendicular to the Y axis 
    float d2z = length(aPath*t.z); // intersection with a plane perpendicular to the Z axis 

    // return the closest intersection - that is, the correct one 
    return min(d2x,min(d2y,d2z)); 
} 

// Compute ray depth in depth buffer 
float computeDepth(vec3 backWC, vec3 frontWC, float rayTermination) 
{   
    vec3 terminationWC = frontWC + ((backWC - frontWC) * vec3(rayTermination)); 
    vec4 iproj = gl_ModelViewProjectionMatrix * vec4(terminationWC, 1); 
    iproj.z /= iproj.w; 
    return (iproj.z + 1.0) * 0.5; 
} 
 

void main()
{
    // sample FirstPass texture with back-facing tex. coords (RGB) and back-facing Z-Buffer value (A) 
    vec4 OSR = texture2D(Osr,(gl_FragCoord.xy)*wSize); 
    
    // back and front coordinates 
    vec3 backWC = texture2D(OsrWC, (gl_FragCoord.xy) * wSize).xyz; 
    vec3 frontWC = worldCoords; 

    // set initial fragment color/value
    vec3 texel = vec3(0.0);

    // set intilial monocromatic color
    float oldMonochromatic = 0.0;

    // set ray direction 
    vec3 RayPath = OSR.rgb - gl_TexCoord[0].stp; 
    
    // normalized direction 
    vec3 nPath = normalize(RayPath);
       
    // adjust length of the step (vector)   
    vec3 CoarsePath = nPath * tSkipResolution.x; 
    
    // adjust length of the step (vector)   
//    vec3 FinePath = nPath * stepLength(RayPath); 
    vec3 FinePath = nPath * tResolution.x; 
    
    // distance to travel by the ray 
    float tDistance = length(RayPath); 
    
    // length of a coarse step (scalar) 
    float tCoarseStep = length(CoarsePath); 
    
    // length of a fine step (scalar) 
    float tFineStep = length(FinePath); 

	// calculate the number of fine steps in a single coarse step
	int NumOfFineSteps = int(tCoarseStep / tFineStep);
    
    // where to start in the texture (scalar) 
    float tPositionStart = texture2D(Noise, (gl_FragCoord.xy - vec2(0.5)) * wSize).r * tFineStep; 
    
    // where to start in the texture (vector) 
    vec3 vtPositionStart = gl_TexCoord[0].stp + nPath * tPositionStart; 
      
    // is the initial voxel meaningful? 
    int voxelFlagStart = isMeaningful(vtPositionStart); 

    // outer loop 
    for( int i = 0; i < 256; i++ ) 
    {
        // check if the ray is still vithin the textured volume 
        if( tPositionStart > tDistance )
            break;
        
        // next position 
        vec3 vtPositionEnd = vtPositionStart + CoarsePath; 
        float tPositionEnd = tPositionStart + tCoarseStep; 
        
        // is the end position meaningful? 
        int voxelFlagEnd = isMeaningful(vtPositionEnd); 

        // should this part of the volume be passed
        if( (voxelFlagStart + voxelFlagEnd) != 0 )
		{
            // inner loop
            vec3 vtPosition = vtPositionStart; 
            float tPosition = tPositionStart; 
            for( int j = 0; j < NumOfFineSteps; j++ ) 
            {
                // z-buffer check
                if( texture2D(Depth, (gl_FragCoord.xy) * wSize).r < computeDepth(backWC, frontWC, tPosition / tDistance) )
                    break;
            
                // sample voxel at the current position 
                float voxel = planeCut(vtPosition, saturate((texture3D(t3D, vtPosition).r * inputAdjustment.x + 0.001) + inputAdjustment.y), 0.0);

                // get color value from look-up table 
                vec4 LUT = texture1D(LookUp, voxel); 
        
                // estimate monochromatic intensity and compare maximum
                float newMonochromatic = 0.2989 * LUT.r + 0.5870 * LUT.g + 0.1140 * LUT.b;
                
                // checks if the current texel is a new maximum
                if( newMonochromatic > oldMonochromatic )
                {
                    texel = LUT.rgb;
                    oldMonochromatic = newMonochromatic;
                }
                
                // move ray to the next position - both the scalar and the vector 
                vtPosition += FinePath;
                tPosition += tFineStep;
            } 
        }        
        
        // move to the next position 
        vtPositionStart = vtPositionEnd;
        tPositionStart = tPositionEnd;
        voxelFlagStart = voxelFlagEnd;        
    } 

    vec4 outputColor = vec4(saturate((texel * imageAdjustment.y) + imageAdjustment.x), 1.0f);
	float alpha = outputColor.r * 3;
	gl_FragData[0] = outputColor;
    gl_FragData[1] = vec4(0, alpha, 0, 0);
}


/*
 *    This file is part of CLraytracer.
 *    CLraytracer is a raytracing based renderer that take advantage of GPU 
 *    parallelism through OpenCL
 *    Copyright (C) 2012  Yuri Iozzelli <y.iozzelli@gmail.com>
 * 
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 * 
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 * 
 *    You should have received a copy of the GNU General Public License along
 *    with this program; if not, write to the Free Software Foundation, Inc.,
 *    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
 
typedef float cl_float;
typedef float3 cl_float3;
typedef float16 cl_float16;
typedef uint cl_uint;

#include "objectsCL.h"



float3 mult(const float16 matrix, const float3 vect)
{
	float4 vec = (float4)(vect,1);
	float3 ris;
	ris.x = matrix.s0*vec.s0 + matrix.s1*vec.s1 + matrix.s2*vec.s2 + matrix.s3*vec.s3;
	ris.y = matrix.s4*vec.s0 + matrix.s5*vec.s1 + matrix.s6*vec.s2 + matrix.s7*vec.s3;
	ris.z = matrix.s8*vec.s0 + matrix.s9*vec.s1 + matrix.sa*vec.s2 + matrix.sb*vec.s3;
	
	return ris;
}


 bool hitSphere(const Ray r, const Sphere s, float *t) 
 {  
	float3 dist = s.pos-r.start;
	float v = dot(r.dir,dist);
	float disc = s.radius*s.radius - (dot(dist,dist)-v*v);
	if(disc<0.0f)
		return false;
		
	float d = sqrt(disc);
	*t = v-d;
	if( *t < 0 ) {
		return false;
	}
	return true;
	
 }
 
 int checkNearestSphereHit(const Ray r, const __global Sphere* vs,const int nSph, float* t)
 {
	*t = -1;
	float t_min;
	int i ;
	int index = -1;
	for( i = 0; i<nSph; ++i )
	{
		if(hitSphere(r,vs[i],t))
		{
			if ( *t<t_min || index == -1 || *t<t_min )
			{
				t_min = *t;
				index = i;
			}
		}
		
	}
	*t = t_min;
	return index;
 }
 
 bool checkIfOtherSphereHit(const Ray r, const __global Sphere* vs,const int nSph,int current)
 {
	float t;
	int i ;
	for(i = 0;i<nSph;i=i+1)
	{
		if( i!=current && hitSphere(r,vs[i],&t) )
		{
			return true;
		}
		
	}
	return false;
}

#define REFLECT(i,n) ( (i) - 2*dot((n),(i)) * (n) )


__kernel void rayTracer (__write_only image2d_t img,const int imgW,const int imgH,
			const Camera cam,			
			const __global Sphere* spheres,const int nSph,
			const __global Light* lights,const int nLgh,
			const __global Material* materials,const int nMtr)
{
	if(get_global_id(0)>=imgW || get_global_id(1)>=imgH)
		return;
		
	float ambient_coef = 0.2f;
	float diffuse_coef = 1 - ambient_coef;
	
	Ray r;
	r.dir.x = (((float)get_global_id(0))/imgW) - 0.5f;
	r.dir.y = (((float)get_global_id(1))/imgH) - 0.5f;
	r.dir.z = 1;
	r.start = (float3)(0,0,0);
	
	r.start = mult(cam,r.start);
	r.dir = mult(cam,r.dir);
	r.dir = normalize(r.dir - r.start);
	
	float3 color = (float3)(0,0,0);
	float iterCoef = 1;
	int missingIters = 5;
	
	while( iterCoef>0 && missingIters > 0 ) {
		// finding the first surface colliding with the ray r
		// its distance from the ray emitter will be t the
		// hitting surface's index will be index
		float t = 0;
		int index = checkNearestSphereHit(r,spheres,nSph,&t);
		if( index == -1 ) {

			int2 imgCoords = (int2)(get_global_id(0), get_global_id(1));
			write_imagef(img, imgCoords, (float4)(0.0f,0.0f,0.0f,1.0f));
			break;
		}
		float3 hitPoint = r.start + t*r.dir;
		Material mat = materials[spheres[index].materialID];
		color += mat.color * ambient_coef * iterCoef;
		float3 norm;
		
		// calculating the intensity of the light according
		// to the lambert model
		float shade;
		int j;
		for(j = 0;j<nLgh;j=j+1) {
			Ray rToLight = {hitPoint,normalize(lights[j].pos-hitPoint)};
			norm = normalize(hitPoint-spheres[index].pos);
			shade = dot(norm,rToLight.dir);
			if( shade < 0 ) {
				// in shadow of this same sphere
				continue;
			}
			if( checkIfOtherSphereHit(rToLight,spheres,nSph,index) ) {
				// in shadow of another sphere
				continue;
			}
		
			color += mat.color * diffuse_coef * shade * lights[j].color * iterCoef;
			
			
		float fViewProjection = dot( r.dir, norm );
			float3 blinnDir = rToLight.dir - r.dir;
			float temp = dot(blinnDir, blinnDir);
			if( temp != 0.0f ) {
				float blinn = 1/sqrt(temp) * max(shade - fViewProjection , 0.0f);
				blinn = iterCoef * pow(blinn, mat.power);
				color += blinn * ( mat.reflection * lights[j].color );
			}
			
		}
		
		// updating the ray
		r.start = hitPoint;
		r.dir = REFLECT( r.dir, norm );
		iterCoef *= sqrt(dot(mat.reflection,mat.reflection));
		-- missingIters;
	}
	
	
		
	float exposure = -1.00f;
	color.x = 1.0f - exp(color.x * exposure);
	color.y = 1.0f - exp(color.y * exposure);
	color.z = 1.0f - exp(color.z * exposure);
	

	int2 imgCoords = (int2)(get_global_id(0), get_global_id(1));
	write_imagef(img, imgCoords, (float4)(color,1.0f));
	
}

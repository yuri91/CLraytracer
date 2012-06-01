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
typedef CLScene Scene;



float3 mult(const float16 matrix, const float3 vect)
{
	float4 vec = (float4)(vect,1);
	float3 ris;
	ris.x = matrix.s0*vec.s0 + matrix.s1*vec.s1 + matrix.s2*vec.s2 + matrix.s3*vec.s3;
	ris.y = matrix.s4*vec.s0 + matrix.s5*vec.s1 + matrix.s6*vec.s2 + matrix.s7*vec.s3;
	ris.z = matrix.s8*vec.s0 + matrix.s9*vec.s1 + matrix.sa*vec.s2 + matrix.sb*vec.s3;
	
	return ris;
}
 
int checkNearestHit( const Ray *r, const Scene* s, float* t )
{
	*t = -1;
	float t_min;
	int i;
	int offset = -1;
	const __global char *currentOffset = s->objects;
	for( i = 0; i < s->nObjects; ++i ) {
		const __global CommonObject *currentObj = (const __global CommonObject*) currentOffset;
		if( currentObj->type == SPHERE_TYPE ) {
			const __global Sphere *sphere = (const __global Sphere*) currentObj;
			if( hitSphere(r, sphere, t) ) {
				if ( *t<t_min || offset == -1 || *t<t_min ) {
					t_min = *t;
					offset = currentOffset - s->objects;
				}
			}
			currentOffset += sizeof(Sphere);
		}
		else {
			// TODO: ERROR
		}
	}
	*t = t_min;
	return offset;
}
 
bool checkHit( const Ray *r, const Scene* s )
{
	float t;
	int i;
	const __global char *currentOffset = s->objects;
	for( i = 0; i < s->nObjects; ++i ) {
		const __global CommonObject *currentObj = (const __global CommonObject*) currentOffset;
		if( currentObj->type == SPHERE_TYPE ) {
			const __global Sphere *sphere = (const __global Sphere*) currentObj;
			if( hitSphere(r,sphere,&t) && t >= 0.001 ) {
				return true;
			}
			currentOffset += sizeof(Sphere);
		}
		else {
			// TODO: ERROR
		}
	}
	return false;
}

#define REFLECT(i,n) ( (i) - 2*dot((n),(i)) * (n) )


__kernel void rayTracer (__write_only image2d_t img, const int imgW, const int imgH,
			const Camera cam,
			const __global char* objects, const int nObjects,
			const __global Light* lights, const int nLgh,
			const __global Material* materials, const int nMtr )
{
	if( get_global_id(0)>=imgW || get_global_id(1)>=imgH ) {
		return;
	}
	
	// our pixel
	int2 imgCoords = (int2)( get_global_id(0), get_global_id(1) );
	
	// initializing the scene object
	Scene scene;
	{
		scene.objects = objects;
		scene.nObjects = nObjects;
		scene.lights = lights;
		scene.nLights = nLgh;
		scene.materials = materials;
		scene.nMaterials = nMtr;
	}
	
	// initializing lights (TODO: move this into scene)	
	float ambient_coef = 0.2f;
	float diffuse_coef = 1 - ambient_coef;
	
	// finding ray for this pixel
	Ray r;
	{
		r.dir.x = (((float)get_global_id(0))/imgW) - 0.5f;
		r.dir.y = (((float)get_global_id(1))/imgH) - 0.5f;
		r.dir.z = 1;
		r.start = (float3)(0,0,0);
	
		r.start = mult(cam,r.start);
		r.dir = mult(cam,r.dir);
		r.dir = normalize(r.dir - r.start);
	}
	
	// computing the value for this pixel
	float3 color = (float3)(0,0,0);
	float iterCoef = 1;
	int missingIters = 100;
	
	while( iterCoef > 0.01 && missingIters > 0 ) {
		// finding the first surface colliding with the ray r
		// its distance from the ray emitter will be t the
		// hitting surface's index will be index
		float t = 0;
		int offset = checkNearestHit( &r, &scene, &t );
		if( offset == -1 ) {
			// hitting nothing
			break;
		}
		
		// hit something...
		float3 hitPoint = r.start + t*r.dir;
		const __global CommonObject *hitObject = (const __global CommonObject*) &scene.objects[offset];
		Material mat = materials[ hitObject->materialID ];
		color += mat.color * ambient_coef * iterCoef;
		
		// getting the normal of the hit surface
		float3 norm;
		{
			if( hitObject->type == SPHERE_TYPE ) {
				norm = getNormalForSphere( hitPoint, (const __global Sphere*) hitObject );
			}
			else {
				// TODO: ERROR
				write_imagef( img, imgCoords, (float4)(1.0f,1.0f,1.0f,1.0f) );
				return ;
			}
		}
		
		// calculating the intensity of the light according
		// to the lambert model
		float shade;
		int j;
		for( j = 0; j < nLgh; ++j ) {
			// ray to light
			Ray rToLight = { hitPoint, normalize(lights[j].pos-hitPoint) };
			
			// checking whether it's shadowed by something else
			if( checkHit( &rToLight, &scene ) ) {
				// in shadow of another sphere, skipping this light...
				continue;
			}
			
			// computing the shade
			shade = dot(norm, rToLight.dir);
			
			// computing the actual pixel color
			color += mat.color * diffuse_coef * fabs(shade) * lights[j].color * iterCoef;
			
			// applying the blinn effect
			{
				float fViewProjection = dot( r.dir, norm );
				float3 blinnDir = rToLight.dir - r.dir;
				float temp = dot(blinnDir, blinnDir);
				if( temp != 0.0f ) {
					float blinn = 1/sqrt(temp) * max(shade - fViewProjection , 0.0f);
					blinn = iterCoef * pow(blinn, mat.power);
					color += blinn * ( mat.reflection * lights[j].color );
				}
			}
		}
		
		// reflecting the ray
		r.start = hitPoint;
		r.dir = REFLECT( r.dir, norm );
		iterCoef *= sqrt(dot(mat.reflection,mat.reflection));
		-- missingIters;
	}
	
	// normalizing color between 0 and 1
	float exposure = -1.00f;
	color.x = 1.0f - exp(color.x * exposure);
	color.y = 1.0f - exp(color.y * exposure);
	color.z = 1.0f - exp(color.z * exposure);
	
	// actually setting the pixel color
	write_imagef(img, imgCoords, (float4)(color,1.0f));
}

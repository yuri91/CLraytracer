
typedef struct
{
	int type;
	int materialID;
} CommonObject;

typedef struct 
{
	cl_float3 pos;
	cl_float3 color;
}Light ;

typedef struct 
{
	cl_float3 reflection;
	cl_float3 color;
	cl_float power;
}Material;

typedef struct  
{
	cl_float3 start;
	cl_float3 dir;
}Ray;

typedef cl_float16 Camera;

#ifdef CL_CODE
typedef struct
{
	const __global char *objects;
	int nObjects;
	
	const __global Light *lights;
	int nLights;
	const __global Material *materials;
	int nMaterials;
	
	// int ambientLight; // between 0 (fully dark) and 1 (fully bright)
} CLScene;
#endif

#include "cl/sphere.h"



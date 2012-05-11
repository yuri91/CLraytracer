typedef struct 
{
	cl_float3 pos;
	cl_float radius;
	int materialID;
}Sphere;

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



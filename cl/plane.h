
typedef struct 
{
    CommonObject base;
    
    cl_float3 point;
    cl_float3 normal;
} Plane;

#define PLANE_TYPE 2


#ifdef CL_CODE

float3 getNormalForPlane( float3 hitPoint, const __global Plane* p )
{
    return p->normal;
}

bool hitPlane( const Ray *r, const __global Plane *p, float *distance )
{
    
    float3 op = r->start - p->point;
    if( dot(op,r->dir) <=0) return false;
    
    float h = dot(op,p->normal);
    
    *distance = h*h/dot(h*p->normal,r->dir);    
    
    return true;
}

#endif
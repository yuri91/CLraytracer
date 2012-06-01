
typedef struct 
{
	CommonObject base;
	
	cl_float3 pos;
	cl_float radius;
} Sphere;

#define SPHERE_TYPE 1


#ifdef CL_CODE

float3 getNormalForSphere( float3 hitPoint, const __global Sphere *s )
{
	return normalize( hitPoint - s->pos );
}

bool hitSphere( const Ray *r, const __global Sphere *s, float *distance )
{
	float3 dist = s->pos - r->start;
	float v = dot( r->dir, dist );
	float disc = s->radius*s->radius - ( dot(dist,dist) - v*v);
	if( disc < 0.0f ) {
		return false;
	}

	float d = sqrt(disc);
	*distance = v-d;
	if( *distance < 0 ) {
		return false;
	}
	return true;
}

#endif


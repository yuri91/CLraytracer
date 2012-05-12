#ifndef MATRIX_H
#define MATRIX_H
#include <CL/cl_platform.h>


class Matrix4x4
{
public:
	cl_float16 m;
	
	Matrix4x4( ) { }
	Matrix4x4( cl_float16 m ) : m(m) { }
	
	cl_float3 operator*( const cl_float3 &vect );
	Matrix4x4 operator*( const Matrix4x4 &mat );
	
	Matrix4x4 translate( const cl_float3 offset );
	Matrix4x4 scale( const cl_float3 factor );
	Matrix4x4 rotate( const cl_float3 angle );
	
	static Matrix4x4 identity( ) {
		return Matrix4x4( {{1,0,0,0,
				   0,1,0,0,
				   0,0,1,0,
				   0,0,0,1}} );
	}
	
};

inline cl_float3 operator*( cl_float3 vec, cl_float scalar )
{
	cl_float3 r;
	r.s[0] = vec.s[0]*scalar;
	r.s[1] = vec.s[1]*scalar;
	r.s[2] = vec.s[2]*scalar;
	return r;
}


#endif
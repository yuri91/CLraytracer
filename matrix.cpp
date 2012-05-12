#include "matrix.h"
#include <cmath>


cl_float3 Matrix4x4::operator*( const cl_float3 &vect )
{
	cl_float4 vec={0,0,0,1};
	vec.s[0]=vect.s[0];
	vec.s[1]=vect.s[1];
	vec.s[2]=vect.s[2];
	
	cl_float3 ris;
	ris.s[0] = m.s[0]*vec.s[0] + m.s[1]*vec.s[1] + m.s[2]*vec.s[2] + m.s[3]*vec.s[3];
	ris.s[1] = m.s[4]*vec.s[0] + m.s[5]*vec.s[1] + m.s[6]*vec.s[2] + m.s[7]*vec.s[3];
	ris.s[2] = m.s[8]*vec.s[0] + m.s[9]*vec.s[1] + m.s[10]*vec.s[2] + m.s[11]*vec.s[3];
	
	return ris;
}
Matrix4x4 Matrix4x4::operator*( const Matrix4x4 &mat )
{
	Matrix4x4 ris;
	
	for(int i = 0;i<4;++i)
	{
		for(int j = 0;j<4;++j)
		{
			ris.m.s[i*4+j]=0;
			for(int k = 0;k<4;++k)
			{
				ris.m.s[i*4+j] += m.s[i*4+k] * mat.m.s[k*4+j];
			}
		}
	}
	
	return ris;
}

Matrix4x4 Matrix4x4::translate( const cl_float3 offset )
{
	cl_float16 tras = {1,0,0,offset.s[0],
			   0,1,0,offset.s[1],
			   0,0,1,offset.s[2],
			   0,0,0,1};
	return operator*( tras );
}
Matrix4x4 Matrix4x4::scale( const cl_float3 factor )
{
	cl_float16 tras = {factor.s[0],0,0,0,
			   0,factor.s[1],0,0,
			   0,0,factor.s[2],0,
			   0,0,0,1};
	return operator*( tras );
}
Matrix4x4 Matrix4x4::rotate( const cl_float3 angle )
{
	cl_float x =  angle.s[0];
	cl_float z =  angle.s[1];
	cl_float y =  angle.s[2];
	using namespace std;
	cl_float16 tras = { cos(x)*cos(y), sin(z)*sin(x)*cos(y)-cos(z)*sin(y), cos(z)*sin(x)*cos(y)+sin(z)*sin(y), 0,
			    cos(x)*sin(y), sin(z)*sin(x)*sin(y)+cos(z)*cos(y), cos(z)*sin(x)*sin(y)-sin(z)*cos(y), 0,
			    -sin(x),       sin(z)*cos(x),                      cos(z)*cos(x),                      0,
			    0,             0,                                  0,                                  1 };
	return operator*( tras );
}


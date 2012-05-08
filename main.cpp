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


#include <GL/glew.h>
#include <GL/glfw.h>
#include <GL/gl.h>
#include <oglplus/all.hpp>
#include <iostream>

#include "renderer.h"

#define WIDTH 1024
#define HEIGHT 768

int windowCloseCallback( )
{
	static int i = 0;
	return i ++;
}

class GLRenderer
{
public:
	oglplus::Context gl;
	oglplus::VertexArray rect;
	oglplus::Buffer verts;
	oglplus::Program prog;
	oglplus::Texture texture;
	
public:
	GLRenderer( )
	{
		glfwSetWindowTitle("CLraytracer");
	
		// initializing openGL
		{
			using namespace oglplus;
	
			// vertex shader
			{
				oglplus::VertexShader vs;
				vs.Source(" \
					attribute vec3  Position; \
					void main() \
					{ \
						gl_Position	 = vec4(Position,1); \
						gl_TexCoord[0] = ( gl_Position + vec4(1,1,0,0) ) * 0.5; \
					} \
					");
				vs.Compile();
				prog.AttachShader( vs );
			}
		
			// fragment shader
			{
				oglplus::FragmentShader fs;
				fs.Source(" \
					uniform sampler2D fava; \
					void main() \
					{ \
						/*gl_FragColor = vec4(1,0,0,1.0);*/ \
						gl_FragColor = texture2D( fava, gl_TexCoord[0].st ); \
					} \
					");
				fs.Compile();
				prog.AttachShader(fs);
			}
		
			// link and use it
			prog.Link();
			prog.Use();
	
			//oglplus::VertexArray rect;
			//oglplus::Buffer verts;
			GLfloat rect_verts[12] = {
				-1.0f, -1.0f, 0.0f,
				-1.0f, 1.0f, 0.0f,
				1.0f, -1.0f, 0.0f,
				1.0f, 1.0f, 0.0f
				};

			// bind the VBO for the rect vertices
			verts.Bind( Buffer::Target::Array );
			// upload the data
			Buffer::Data(
				Buffer::Target::Array,
				12,
				rect_verts
				);
	
			// setup the vertex attribs array for the vertices
			VertexAttribArray vert_attr( prog, "Position" );
			vert_attr.Setup( 3, DataType::Float );
			vert_attr.Enable();
	
			gl.ClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			gl.ClearDepth(1.0f);
		}
	}
	
	~GLRenderer( )
	{
	}
	
	void setTexture( cl_float3 *data )
	{
		using namespace oglplus;
		
		texture.Bind( Texture::Target::_2D );
		texture.MagFilter( Texture::Target::_2D, TextureMagFilter::Linear );
		texture.MinFilter( Texture::Target::_2D, TextureMinFilter::Linear );
		texture.Image2D( Texture::Target::_2D, 0, PixelDataInternalFormat::RGBA, WIDTH,HEIGHT, 0, PixelDataFormat::RGBA, PixelDataType::Float, data );
	}
	
	void draw( )
	{
		using namespace oglplus;
	
		gl.Clear().ColorBuffer().DepthBuffer();
		gl.DrawArrays( PrimitiveType::TriangleStrip, 0, 4 );
	}
	void mainLoop( )
	{
		while( glfwGetWindowParam(GLFW_OPENED) ) {
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			draw();
			glfwSwapBuffers();
			glfwPollEvents( );
		}
	}
	
};


int main(int argc, char *argv[]) 
{;
	if( ! glfwInit() ) {
		throw( "glfwInit failed" );
	}
	struct on_terminate { ~on_terminate(){ glfwTerminate(); } } _l1;

	if( ! glfwOpenWindow( WIDTH,HEIGHT, 0,0,0,0, 0, 0, GLFW_WINDOW  ) ) {
		throw( "glfwOpenWindow failed" );
	}
	struct on_close { ~on_close(){ glfwCloseWindow(); } } _l2;

	if( glewInit() != GLEW_OK ) {
		throw( "glewInit failed" );
	}
	
	try
	{
		GLRenderer GLr;
		
		
		Renderer r;

		Scene s;

		s.eye = -10;

		s.viewW = 10.24*1.5;
		s.viewH = 7.68*1.5;
		
#define X(i) sin(i*2*M_PI/3)*10*2/sqrt(3)
#define Y(i) cos(i*2*M_PI/3)*10*2/sqrt(3)
		
		Light l1 = {{0,30,-20},{2,2,2}};
		s.lights.push_back(l1);
		
 		Light l2 = {{30,-50,0},{1,1,1}};
 		s.lights.push_back(l2);
		
		Sphere sp1 = {{X(0),Y(0),30},10,0};
		Material m1 = {{.1,.1,.1},{0.5,0.5,0},60};
		s.spheres.push_back(sp1);
		s.materials.push_back(m1);

		Sphere sp2 = {{X(1),Y(1),30},10,1};
		Material m2 = {{.1,.1,.1},{0,0.5,0.5},60};
		s.spheres.push_back(sp2);
		s.materials.push_back(m2);

		Sphere sp3 = {{X(2),Y(2),30},10,2};
		Material m3 = {{.1,.1,.1},{0.5,0,0.5},60};
		s.spheres.push_back(sp3);
		s.materials.push_back(m3);
		
		// 		Light l1 = {{0,240,-100},{2,2,2}};
// 		s.lights.push_back(l1);
// 		
//  		Light l2 = {{640,240,-10000.0},{0.6,0.7,1}};
//  		s.lights.push_back(l2);
// 		
// 		Sphere sp1 = {{233,290,150},100,0};
// 		Material m1 = {{.5,.5,.5},{1,1,0},60};
// 		s.spheres.push_back(sp1);
// 		s.materials.push_back(m1);
// 
// 		Sphere sp2 = {{407,290,150},100,1};
// 		Material m2 = {{.5,.5,.5},{0,1,1},60};
// 		s.spheres.push_back(sp2);
// 		s.materials.push_back(m2);
// 
// 		Sphere sp3 = {{320,140,150},100,2};
// 		Material m3 = {{.5,.5,.5},{1,0,1},60};
// 		s.spheres.push_back(sp3);
// 		s.materials.push_back(m3);

#undef X
#undef Y

		int imgW = WIDTH;
		int imgH = HEIGHT;
		std::vector<cl_float3> ris = r.compute(s,imgW,imgH);
		GLr.setTexture( &ris[0] );
		
		
		GLr.mainLoop();
	}
	catch( oglplus::CompileError &err )
	{
		std::cerr <<
		"Error (in " << err.GLSymbol() << ", " <<
		err.ClassName() << ": '" <<
		err.ObjectDescription() << "'): " <<
		err.what() <<
		" [" << err.File() << ":" << err.Line() << "] ";
		std::cerr << std::endl;
		std::cerr << err.Log() << std::endl;
		err.Cleanup();
	}
	catch( oglplus::Error &err )
	{
		std::cerr <<
		"Error (in " << err.GLSymbol() << ", " <<
		err.ClassName() << ": '" <<
		err.ObjectDescription() << "'): " <<
		err.what() <<
		" [" << err.File() << ":" << err.Line() << "] ";
		std::cerr << std::endl;
		err.Cleanup();
	}
   
    
    return EXIT_SUCCESS;
}

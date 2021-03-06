/*
 *	This file is part of CLraytracer.
 *	CLraytracer is a raytracing based renderer that take advantage of GPU 
 *	parallelism through OpenCL
 *	Copyright (C) 2012  Yuri Iozzelli <y.iozzelli@gmail.com>
 * 
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 * 
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 * 
 *	You should have received a copy of the GNU General Public License along
 *	with this program; if not, write to the Free Software Foundation, Inc.,
 *	51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */


#include "renderer.h"
#include <GL/glx.h>


Renderer::Renderer()
{

	cl::Platform::get(&platforms);

	platforms.front().getDevices(CL_DEVICE_TYPE_GPU,&devices);
	
	// Linux
	cl_context_properties cps[] = {
	    CL_GL_CONTEXT_KHR,
	    (cl_context_properties)glXGetCurrentContext(),
	    CL_GLX_DISPLAY_KHR,
	    (cl_context_properties)glXGetCurrentDisplay(),
	    CL_CONTEXT_PLATFORM,
	    (cl_context_properties)(platforms.front())(),
	    0
	};

	context = cl::Context(devices,cps);
	cmdqueue = cl::CommandQueue(context,devices.front());

	std::unique_ptr<char> source;
	{

		std::ifstream file( "../rayTracer.cl", std::ios_base::in|std::ios_base::ate );
		long file_length = file.tellg();
		file.seekg(0, std::ios_base::beg);
		file.clear();

		source.reset( new char[file_length] );
		file.read( source.get(), file_length );

		sourceCode.push_back(std::make_pair( source.get(), file_length) );
	}


	program = cl::Program(context , sourceCode);
	try
	{
		program.build( devices,"-DCL_CODE -I ../"/*, "-Wall"*/ );
		std::cout << "Building: " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices.front()) << std::endl;
	}
	catch(cl::Error& err)
	{
		std::cerr<<
				 "Building failed, " <<
				 err.what()<<
				 "(" <<
				 err.err()<<
				 ")" <<
				 "\n Retrieving build log "
				 <<"\n Build Log Follows \n"
				 <<program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices.front());
		exit(1);
	}
	std::cout<<"Building ok!"<<std::endl;

	kernel = cl::Kernel(program,"rayTracer" );

}

Renderer::~Renderer()
{

}


void Renderer::compute(Scene& s, int imgW, int imgH,GLuint tex)
{
	int groupSize = 8;
	int rangeW = (imgW/groupSize)*groupSize+groupSize;
	int rangeH = (imgH/groupSize)*groupSize+groupSize;
	
	cl_int result = CL_SUCCESS;
	cl::Event ev;
	
	
	/*cl::Buffer img(context,CL_MEM_WRITE_ONLY ,sizeof(cl_float3)*imgH*imgW);*/ 
	cl::Buffer obj(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, s.objects.size(), &s.objects[0]);
	cl::Buffer lgh(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(Light)*s.lights.size(), &s.lights[0]);
	cl::Buffer mtr(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(Material)*s.materials.size(), &s.materials[0]);
	
	try
	{
	    
		cl::Image2DGL imgGL(context,CL_MEM_WRITE_ONLY,GL_TEXTURE_2D, 0, tex, nullptr);
		
		std::vector<cl::Memory> memObjs;
		memObjs.clear();
		memObjs.push_back(imgGL);
		
		
		
		cl::KernelFunctor func = kernel.bind(cmdqueue,cl::NDRange(rangeW, rangeH), cl::NDRange(groupSize, groupSize));
		
		int nObj = s.nObj;
		int nLgh = s.lights.size();
		int nMtr = s.materials.size();
		
		result = cmdqueue.enqueueAcquireGLObjects(&memObjs, NULL, &ev);
		ev.wait();
	
		func(imgGL,imgW,imgH,
		     s.camera,
		     obj,nObj,
		     lgh,nLgh,
		     mtr,nMtr
		);
		
		result = cmdqueue.enqueueReleaseGLObjects(&memObjs, NULL, &ev);
		ev.wait();
	
		cmdqueue.flush();
// 		std::vector<cl_float3> result(imgW*imgH);
// 	
// 		cmdqueue.enqueueReadBuffer(img,true,0,sizeof(cl_float3)*imgH*imgW,&result[0]);
// 		return result;
	}
	catch(cl::Error& err)
	{
		std::cerr<<
		"kernel execution failed, " <<
		err.what()<<
		"(" <<
		err.err()<<
		")" <<
		std::endl;
		
		exit(1);
	}
}




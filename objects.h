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


#ifndef OBJECTS_H
#define OBJECTS_H
#include <CL/cl_platform.h>

struct Sphere
{
    cl_float3 pos;
    cl_float radius;
    cl_uint materialID;
};

struct Light 
{
    cl_float3 pos;
    cl_float3 color;
};

struct Material
{
    cl_float3 reflection;
    cl_float3 color;
    cl_float power;
};

#endif

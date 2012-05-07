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



#include <iostream>

#include <png++/png.hpp>

#include "renderer.h"


int main(int argc, char *argv[]) 
{;
    Renderer r;
    
    Scene s;
    
    s.eye = -10;
    
    s.viewW = 10;
    s.viewH = 10;
    
    Light l = {{10,15,-10},{1,1,1}};
    s.lights.push_back(l);
    
    Sphere sp1 = {{0,0,70},25,0};
    Material m1 = {{0,0,0},{0,1,0}};
    s.spheres.push_back(sp1);
    s.materials.push_back(m1);
    
    Sphere sp2 = {{-3,0,30},9,1};
    Material m2 = {{0,0,0},{1,0,0}};
    s.spheres.push_back(sp2);
    s.materials.push_back(m2);
    
    
    Sphere sp3 = {{5,2,15},3,2};
    Material m3 = {{0,0,0},{0,0,1}};
    s.spheres.push_back(sp3);
    s.materials.push_back(m3);
    
    
    int imgW = 800;
    int imgH = 800;
    std::vector<cl_float3>  ris = r.compute(s,imgW,imgH);
    
    png::image<png::rgb_pixel> img(imgW,imgH);
    
    int j = 0;
    for(auto i = ris.begin();i!=ris.end();++i,++j)
    {
	png::rgb_pixel p(255*(i->s[0]),255*(i->s[1]),255*(i->s[2])); 
	img.set_pixel(j/imgW,j%imgW,p);
    }
    
    img.write("test.png");
   
    
    return EXIT_SUCCESS;
}

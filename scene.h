#ifndef SCENE_H
#define SCENE_H

#include "objects.h"
#include <vector>

struct Scene
{
    std::vector<Sphere> spheres;
    std::vector<Light> lights;
    std::vector<Material> materials;
    
    cl_float eye;
    cl_float viewW, viewH;
};

#endif // SCENE_H

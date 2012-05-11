#ifndef SCENE_H
#define SCENE_H

#include "objects.h"
#include <vector>

typedef cl_float16 Camera;

struct Scene
{
    std::vector<Sphere> spheres;
    std::vector<Light> lights;
    std::vector<Material> materials;
    
    Camera camera;
};

#endif // SCENE_H

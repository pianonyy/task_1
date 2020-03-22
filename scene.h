#ifndef Scene_h
#define Scene_h

#include "objects.h"

#include <iostream>

#include <list>
#include <vector>
#include <limits>
#include <float.h>
#include "vect.h"




class LightSource {
public:
	Vec3f position;

	double intensity = 100.0;
  LightSource() : position{} { }
	LightSource(const Vec3f &position_, double intensity_= 100.0) : position(position_), intensity(intensity_) {}
};

Vec3f reflect(const Vec3f &I, const Vec3f &N) {
    return I - N*2.f * (I * N);
}

Vec3f refract(const Vec3f &I, const Vec3f &N, const float eta_t, const float eta_i=1.f) { // Snell's law
    float cosi = -std::max(-1.f, std::min(1.f, I*N));
    if (cosi<0) return refract(I, -N, eta_i, eta_t); // if the ray comes from the inside the object, swap the air and the media
    float eta = eta_i / eta_t;
    float k = 1 - eta*eta*(1 - cosi*cosi);
    return k<0 ? Vec3f(1,0,0) : I*eta + N*(eta*cosi - sqrtf(k)); // k<0 = total reflection, no ray to refract. I refract it anyways, this has no physical meaning
}

bool scene_intersect(const Vec3f &orig, const Vec3f &dir, const std::vector<Object*> &objects, Vec3f &hit, Vec3f &N, Material &material) {
    float object_dist = std::numeric_limits<float>::max();
    for (size_t i = 0; i < objects.size(); i++) {
        float dist_i;
        if (objects[i]->ray_intersect(Ray(orig, dir), dist_i) && dist_i < object_dist) {
            object_dist = dist_i;
            hit = (orig + dir * dist_i);

            N = objects[i]->get_normal(hit);
            material = objects[i]->material;
        }
    }

		//this part draws floor on the picture
    float checkerboard_dist = std::numeric_limits<float>::max();
    if (fabs(dir.y)>1e-3)  {
         double d = -(orig.y + 4) / dir.y;
         Vec3f pt = orig + dir*d;
         if (d > 0 && fabs(pt.x) < 10 && pt.z < -10 && pt.z>-30 && d<object_dist) {
             checkerboard_dist = d;
             hit = pt;
             N = Vec3f(0,1,0);
             material.diffuse_color = (int(.5*hit.x+1000) + int(.5*hit.z)) & 1 ? Vec3f(.3, .3, .3) : Vec3f(.3, .2, .1);
         }
     }
    return std::min(object_dist, checkerboard_dist) < 1000;
}

class Scene {
  public:

  std::vector<Object*> objects;
	std::vector<LightSource> light_sources;



  Scene() {}

  void add(Object *object){
		objects.push_back(object);
  }

	void add(LightSource light){
		light_sources.push_back(light);
	}

  std::vector<Object*> get_objects() const{
    return objects;
  }

  std::vector<LightSource> get_lights() const{
    return light_sources;
  }


  Vec3f trace(float x, float y, float z) {
    // This function works as the camera, translating pixels to rays
		Vec3f ray_origin    = Vec3f(0, 0, 0);
		Vec3f ray_direction = Vec3f(x, y, z).normalize();

		return trace_ray(Ray(ray_origin, ray_direction), objects, 4);
	}

	Vec3f trace_ray(const Ray &ray, const std::vector<Object*> &spheres, int depth) {
		Vec3f point, N;
    Material material;

		if (depth > 4 || !scene_intersect(ray.origin, ray.direction, this->objects, point, N, material)) {
        return Vec3f(0.6, 0.2, 0.9); // background color
    }

    Vec3f reflect_dir   = reflect(ray.direction, N).normalize();
    Vec3f refract_dir   = refract(ray.direction, N, material.refractive_index).normalize();
    Vec3f reflect_orig  = reflect_dir*N < 0 ? point - N*1e-3 : point + N*1e-3; // offset the original point to avoid occlusion by the object itself
    Vec3f refract_orig  = refract_dir*N < 0 ? point - N*1e-3 : point + N*1e-3;
    Vec3f reflect_color = trace_ray(Ray(reflect_orig, reflect_dir), this->objects, depth + 1);
    Vec3f refract_color = trace_ray(Ray(refract_orig, refract_dir), this->objects, depth + 1);

    double diffuse_light_intensity = 0, specular_light_intensity = 0;
    for (size_t i = 0; i < (light_sources).size(); i++) {
        Vec3f light_dir      = (this->light_sources[i].position - point).normalize();
        float light_distance = (this->light_sources[i].position - point).length();

        Vec3f shadow_orig = light_dir*N < 0 ? point - N*1e-3 : point + N*1e-3; // checking if the point lies in the shadow of the lights[i]
        Vec3f shadow_pt, shadow_N;
        Material tmpmaterial;
        if (scene_intersect(shadow_orig, light_dir, this->objects, shadow_pt, shadow_N, tmpmaterial) && (shadow_pt-shadow_orig).norm() < light_distance)
            continue;

        diffuse_light_intensity  += this->light_sources[i].intensity * std::max(0.f, light_dir*N);
        specular_light_intensity += powf(std::max(0.f, -reflect(-light_dir, N)*ray.direction), material.specular_exponent)*(this->light_sources)[i].intensity;
    }
    return (material.diffuse_color * diffuse_light_intensity * material.albedo[0] + Vec3f(1., 1., 1.)*specular_light_intensity * material.albedo[1] + reflect_color*material.albedo[2] + refract_color*material.albedo[3]);
  }
};


#endif

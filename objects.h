#ifndef Objects_h
#define Objects_h

#include "vect.h"
#include "ray.h"
#include <cmath>
#include <vector>
#include <iostream>
#include <float.h>
#include "scene.h"

struct Material {
    Material(const float r, const Vec4f &a, const Vec3f &color, const float spec) : refractive_index(r), albedo(a), diffuse_color(color), specular_exponent(spec) {}
    Material() : refractive_index(1), albedo(1,0,0,0), diffuse_color(), specular_exponent() {}
    float refractive_index;
    Vec4f albedo;
    Vec3f diffuse_color;
    float specular_exponent;
};
// abstract base class for objects in our scene
// class Object {
// public:
// 	Vec3f color;
// 	Material texture;
//
// 	Object(Vec3f color, Material texture) : color(color), texture(texture) {}
// 	virtual Vec3f get_normal(const Vec3f& p) const = 0;
// 	virtual bool intersect(const Ray& ray, float& t) const = 0;
// 	const float SELF_AVOID_T = 1e-2;
// };

class Object
{
public:
	Material material;
	Object() = default;
	Object(const Material& m) : material(m) { }
	virtual Vec3f get_normal(const Vec3f& vec) const = 0;
	virtual bool ray_intersect(const Ray& ray, float &t) const = 0;
  const float SELF_AVOID_T = 1e-2;
};

class Sphere : public Object {
	public:
    Vec3f Center;
    float Radius;

    Sphere(Vec3f center, const float radius, Material texture) :
		Object(texture),
		Center(center),
		Radius(radius)
		{}

    Vec3f get_center() const{
    	return Center;
    }

    Vec3f get_normal(const Vec3f& p) const {
			return (p - Center).normalize();
    }

    bool ray_intersect(const Ray& ray, float &t) const {
        Vec3f v = Center - ray.origin;
				float tca = v * ray.direction;
				float d2 = v * v - tca * tca;

				if (d2 > Radius * Radius) return false;
				float thc = sqrtf(Radius * Radius - d2);
				t     = tca - thc;
				float t1 = tca + thc;

				if(t < 0) t = t1;
				if(t < 0) return false;
				return true;
		}
};


class Cylinder : public Object {
    Vec3f center;
    Vec3f direction;
    float radius;
    float height;

public:
    Cylinder(Vec3f center_, Vec3f direction_, float radius_, float height_, Material texture) : center(center_), direction(direction_.normalize()), radius(radius_), height(height_), Object(texture) {}

 	  Vec3f get_center() const {
 		   return center;
    }

 	  Vec3f get_normal(const Vec3f& p) const {
 		   Vec3f to_center = p - center;
 		   return ( (to_center - (to_center * direction) * direction).normalize() );
 	  }

 	  bool ray_intersect(const Ray & ray, float& t) const {
 		  Vec3f rel_origin = ray.origin - center;

   	  const float directions_dot = ray.direction * direction;
      const float a = 1 - directions_dot* directions_dot;
      const float b = 2 * ( (rel_origin * ray.direction) - (rel_origin * direction) * directions_dot );
      const float c = (rel_origin * rel_origin) - (rel_origin* direction) * (rel_origin * direction) - radius * radius;

      float delta = b * b - 4 * a * c;

      if (delta < 0) {
      	t = FLT_MAX; // no intersection, at 'infinity'
      	return false;
      }

      const float sqrt_delta_2a = sqrt(delta) / (2 * a);
      float t1 = (-b) / (2*a);
      const float t2 = t1 + sqrt_delta_2a;
      t1 -= sqrt_delta_2a;

      if (t2 < SELF_AVOID_T) { // the cylinder is behind us
      	t = FLT_MAX; // no intersection, at 'infinity'
      	return false;
      }
      float center_proj = (center * direction);
      float t1_proj = (ray.get_point(t1) * direction);
      if (t1 >= SELF_AVOID_T && t1_proj > center_proj && t1_proj < center_proj+height) {
      	t = t1;
      	return true;
      }
      float t2_proj = (ray.get_point(t2) * direction);
      if (t2 >= SELF_AVOID_T && t2_proj > center_proj && t2_proj < center_proj+height) {
      	t = t2;
      	return true;
      }
      t = FLT_MAX; // no intersection, at 'infinity'
      return false;
 	  }
};

// 	Circle bottom_circle() {
// 		return Circle(center, direction, radius, color, texture);
// 	}
// 	Circle top_circle() {
// 		return Circle(center+direction*height, direction, radius, color, texture);
// 	}
// 	static void create_capped_cylinder(Scene& scene) {
// 		// create a cylinder and 2 circles?
//}



// Cone's still not correct.
/*
class Cone : public Object {
	Vec3f center;
	Vec3f direction;
	float slope;
	float height;

public:
	Cone(Vec3f center_, Vec3f direction_, float slope_, float height_, Color_t color, Texture_t texture = MAT) : center(center_), direction(direction_.normalize()), slope(slope_), height(height_), Object(color, texture) {}

	Vec3f get_center() const {
		return center;
	}

	Vec3f get_normal(const Vec3f& p) const {
		Vec3f to_center = p - center;
		return ((to_center - direction * (to_center*  direction - slope)).normalize());
	}

	bool ray_intersect(const Ray & ray, float& t) const {
		Vec3f rel_origin = ray.origin - center;

		const float directions_dot = ray.direction.dot(direction);
		const float a = 1 - slope*directions_dot * directions_dot;
		const float b = 2 * (rel_origin.dot(ray.direction) - slope*directions_dot * rel_origin.dot(direction));
		const float c = rel_origin.dot(rel_origin) - slope*rel_origin.dot(direction) * rel_origin.dot(direction)-50.0*50.0;

		float delta = b * b - 4 * a * c;

		if (delta < 0) { // was 1e-4, why?
			t = FLT_MAX; // no intersection, at 'infinity'
			return false;
		}

		const float sqrt_delta_2a = sqrt(delta) / (2 * a);
		float t1 = (-b) / (2 * a);
		const float t2 = t1 + sqrt_delta_2a;
		t1 -= sqrt_delta_2a;

		if (t2 < SELF_AVOID_T) { // the cylinder is behind us
			t = FLT_MAX; // no intersection, at 'infinity'
			return false;
		}
		float center_proj = center.dot(direction);
		float t1_proj = ray.get_point(t1).dot(direction);
		if (t1 >= SELF_AVOID_T && t1_proj > center_proj && t1_proj < center_proj + height) {
			t = t1;
			return true;
		}
		float t2_proj = ray.get_point(t2).dot(direction);
		if (t2 >= SELF_AVOID_T && t2_proj > center_proj && t2_proj < center_proj + height) {
			t = t2;
			return true;
		}
		t = FLT_MAX; // no intersection, at 'infinity'
		return false;
	}
};

*/


class Plane : public Object {
protected:
	Vec3f center;
	Vec3f direction;

public:
	Plane(Vec3f center_, Vec3f direction_, Material texture) : center(center_), direction(direction_.normalize()), Object(texture) {}

	Vec3f get_center() const {
		return center;
	}

	Vec3f get_normal(const Vec3f& p) const {
		return direction;
	}

	virtual bool ray_intersect(const Ray& ray, float& t) const {
		float directions_dot_prod = (direction * ray.direction);
		if (directions_dot_prod == 0) {// the plane and ray are parallel
			t = FLT_MAX; // no intersection, at 'infinity'
			return false;
		}
		t = direction * (center - ray.origin) / directions_dot_prod;


		if (t < SELF_AVOID_T) { // the plane is behind the ray
			t = FLT_MAX;
			return false;
		}

		return true;
	}
};



// class Circle : public Plane {
// 	float radius;
// public:
// 	Circle(Vec3f center_, Vec3f direction_, float radius_, Color_t color, Texture_t texture = MAT) : radius(radius_), Plane(center_, direction_, color, texture) {}
//
// 	bool intersect(const Ray & ray, float& t) const {
// 		if (!Plane::intersect(ray, t)) { // the ray doesnt even hit the plane
// 			return false;
// 		}
// 		Vec3f intersect_point = ray.get_point(t);
//
// 		if ((intersect_point - center).norm2() > radius*radius) { // intersects with plane outside circle
// 			t = FLT_MAX;
// 			return false;
// 		}
//
// 		return true;
// 	}
// };
#endif

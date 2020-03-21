#ifndef Ray_h
#define Ray_h

#include "vect.h"
#include <float.h>


class Ray {
public:
    Vec3f origin;
    Vec3f direction;

	Ray(Vec3f ori, Vec3f dir) : origin(ori), direction(dir) {}

	// Vec3f get_point(double t) const{
	// 	return origin + direction * t;
	// }
	// Vec3f reflect_by(const Vec3f& normal) const{
	// 	return direction - normal * normal* direction * 2;
	// }
  //
  // Vec3 get_origin() const {
  //   return origin;
  // }
  //
  // Vec3 get_direction() const {
  //   return direction;
  // }

};
#endif

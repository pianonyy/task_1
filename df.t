diff --git a/main.cpp b/main.cpp
index 7108e08..a12541a 100644
--- a/main.cpp
+++ b/main.cpp
@@ -22,10 +22,10 @@ int main() {
 
   Material glass(1.5, Vec4f(0.0,  0.5, 0.1, 0.8), Vec3f(0.6, 0.7, 0.8),  125.);
 
-  Sphere sphere1 (Vec3f(200, 250, 220), 2, Vec3f(255,100,40), glass);
+  Sphere sphere1 (Vec3f(200, 250, 220), 2, glass);
 
 
-	Lightsource light1 = Lightsource(Vec3f(10,10, 10), 23);
+	LightSource light1 = LightSource(Vec3f(10,10, 10), 23);
   //Lightsource light2 = Lightsource(Vec3f(100,255, 0), 23);
   //Lightsource light3 = Lightsource(Vec3f(255,255, 255), 12);
 
@@ -35,7 +35,7 @@ int main() {
 	scene.add(light1);
 
 //loading picture in ppm format
-  ofstream my_Image ("image.ppm");
+  ofstream my_Image ("image.bmp");
   if (my_Image.is_open ()) {
       my_Image << "P3\n" << Width << " " << Height << " 255\n";
       for (int i = 0; i < Height; i++) {
diff --git a/objects.h b/objects.h
index bf355e3..9bfdd23 100644
--- a/objects.h
+++ b/objects.h
@@ -28,24 +28,36 @@ struct Material {
 // 	const double SELF_AVOID_T = 1e-2;
 // };
 
-class Sphere {
+class Object
+{
+public:
+	Material material;
+	Object() = default;
+	Object(const Material& m) : material(m) { }
+	virtual Vec3f get_normal(const Vec3f& vec) const = 0;
+	virtual bool ray_intersect(const Ray& ray, float &t) const = 0;
+};
+
+class Sphere : public Object {
 	public:
     Vec3f Center;
     float Radius;
-		Material material;
-
 
-    Sphere(Vec3f center, const float radius, Material texture) : Center(center), Radius(radius), material(texture) {}
+    Sphere(Vec3f center, const float radius, Material texture) :
+		Object(texture),
+		Center(center),
+		Radius(radius)
+		{}
 
     Vec3f get_center() const{
     	return Center;
     }
 
     Vec3f get_normal(const Vec3f& p) const {
-			return ((p - Center)*(-1/Radius)).normalize();// *(-1 / Radius);
+			return ((p - Center)*(-1)).normalize();// *(-1 / Radius);
     }
 
-    bool ray_intersect(const Ray& ray, double &t) const {
+    bool ray_intersect(const Ray& ray, float &t) const {
         Vec3f v = Center - ray.origin;
 				float tca = v * ray.direction;
 				float d2 = v * v - tca * tca;
diff --git a/scene.h b/scene.h
index f580f37..881bf05 100644
--- a/scene.h
+++ b/scene.h
@@ -1,4 +1,4 @@
-objects#ifndef Scene_h
+#ifndef Scene_h
 #define Scene_h
 
 #include "objects.h"
@@ -7,18 +7,20 @@ objects#ifndef Scene_h
 
 #include <list>
 #include <vector>
+#include <limits>
 #include <float.h>
 #include "vect.h"
 
 
 
 
-class Lightsource {
+class LightSource {
 public:
 	Vec3f position;
 	// Vec3f color;
 	double intensity = 100;
-	Lightsource(const Vec3f &position_, double intensity_= 100.0) : position(position_), intensity(intensity_) {}
+  LightSource() : position{} { }
+	LightSource(const Vec3f &position_, double intensity_= 100.0) : position(position_), intensity(intensity_) {}
 };
 
 Vec3f reflect(const Vec3f &I, const Vec3f &N) {
@@ -26,22 +28,22 @@ Vec3f reflect(const Vec3f &I, const Vec3f &N) {
 }
 
 Vec3f refract(const Vec3f &I, const Vec3f &N, const float eta_t, const float eta_i=1.f) { // Snell's law
-    float cosi = - std::max(-1.f, std::min(1.f, I*N));
+    float cosi = -std::max(-1.f, std::min(1.f, I*N));
     if (cosi<0) return refract(I, -N, eta_i, eta_t); // if the ray comes from the inside the object, swap the air and the media
     float eta = eta_i / eta_t;
     float k = 1 - eta*eta*(1 - cosi*cosi);
     return k<0 ? Vec3f(1,0,0) : I*eta + N*(eta*cosi - sqrtf(k)); // k<0 = total reflection, no ray to refract. I refract it anyways, this has no physical meaning
 }
 
-bool scene_intersect(const Vec3f &orig, const Vec3f &dir, const std::vector<Sphere> &spheres, Vec3f &hit, Vec3f &N, Material &material) {
-    float spheres_dist = std::numeric_limits<float>::max();
-    for (size_t i = 0; i < spheres.size(); i++) {
+bool scene_intersect(const Vec3f &orig, const Vec3f &dir, const std::vector<Object*> &objects, Vec3f &hit, Vec3f &N, Material &material) {
+    float object_dist = std::numeric_limits<float>::max();
+    for (size_t i = 0; i < objects.size(); i++) {
         float dist_i;
-        if (spheres[i].ray_intersect(orig, dir, dist_i) && dist_i < spheres_dist) {
-            spheres_dist = dist_i;
-            hit = orig + dir * dist_i;
-            N = (hit - spheres[i].Center).normalize();
-            material = spheres[i].material;
+        if (objects[i]->ray_intersect(Ray(orig, dir), dist_i) && dist_i < object_dist) {
+            object_dist = dist_i;
+            hit = (orig + dir * dist_i);
+            N = objects[i]->get_normal(hit);
+            material = objects[i]->material;
         }
     }
 
@@ -49,21 +51,21 @@ bool scene_intersect(const Vec3f &orig, const Vec3f &dir, const std::vector<Sphe
     if (fabs(dir.y)>1e-3)  {
         double d = -(orig.y+4)/dir.y; // the checkerboard plane has equation y = -4
         Vec3f pt = orig + dir*d;
-        if (d>0 && fabs(pt.x)<10 && pt.z<-10 && pt.z>-30 && d<spheres_dist) {
+        if (d>0 && fabs(pt.x)<10 && pt.z<-10 && pt.z>-30 && d<object_dist) {
             checkerboard_dist = d;
             hit = pt;
             N = Vec3f(0,1,0);
             material.diffuse_color = (int(.5*hit.x+1000) + int(.5*hit.z)) & 1 ? Vec3f(.3, .3, .3) : Vec3f(.3, .2, .1);
         }
     }
-    return min(spheres_dist, checkerboard_dist) < 1000;
+    return std::min(object_dist, checkerboard_dist) < 1000;
 }
 
 class Scene {
   public:
 
-  vec <Object*> objects;
-	vec <Lightsource> lightsources;
+  std::vector<Object*> objects;
+	std::vector<LightSource> light_sources;
 
 
 
@@ -73,16 +75,16 @@ class Scene {
 		objects.push_back(object);
   }
 
-	void add(Lightsource light){
-		lightsources.push_back(light);
+	void add(LightSource light){
+		light_sources.push_back(light);
 	}
 
-  vec <Object*> objects() const{
+  std::vector<Object*> get_objects() const{
     return objects;
   }
 
-  vec <Lightsource> get_lights() const{
-    return lightsources;
+  std::vector<LightSource> get_lights() const{
+    return light_sources;
   }
 
 
@@ -91,10 +93,10 @@ class Scene {
 		Vec3f ray_origin    = Vec3f(0, 0, 0);
 		Vec3f ray_direction = Vec3f(x, y, z).normalize();
 
-		return trace_ray(Ray(ray_origin, ray_direction), 4);
+		return trace_ray(Ray(ray_origin, ray_direction), objects, 4);
 	}
 
-	Vec3f trace_ray(const Ray &ray, const vector <Sphere> &spheres, int depth) {
+	Vec3f trace_ray(const Ray &ray, const std::vector<Object*> &spheres, int depth) {
 		Vec3f point, N;
     Material material;
 
@@ -106,13 +108,13 @@ class Scene {
     Vec3f refract_dir   = refract(ray.direction, N, material.refractive_index).normalize();
     Vec3f reflect_orig  = reflect_dir*N < 0 ? point - N*1e-3 : point + N*1e-3; // offset the original point to avoid occlusion by the object itself
     Vec3f refract_orig  = refract_dir*N < 0 ? point - N*1e-3 : point + N*1e-3;
-    Vec3f reflect_color = trace_ray(Ray(reflect_orig, reflect_dir), this->objects, this->lightsources, depth + 1);
-    Vec3f refract_color = trace_ray(Ray(refract_orig, refract_dir), this->objects, this->lightsources, depth + 1);
+    Vec3f reflect_color = trace_ray(Ray(reflect_orig, reflect_dir), this->objects, depth + 1);
+    Vec3f refract_color = trace_ray(Ray(refract_orig, refract_dir), this->objects, depth + 1);
 
     double diffuse_light_intensity = 0, specular_light_intensity = 0;
-    for (size_t i = 0; i < (this->lightsources).size(); i++) {
-        Vec3f light_dir      = (this->lightsources[i].position - point).normalize();
-        float light_distance = (this->lightsources[i].position - point).normalize();
+    for (size_t i = 0; i < (light_sources).size(); i++) {
+        Vec3f light_dir      = (this->light_sources[i].position - point).normalize();
+        float light_distance = (this->light_sources[i].position - point).length();
 
         Vec3f shadow_orig = light_dir*N < 0 ? point - N*1e-3 : point + N*1e-3; // checking if the point lies in the shadow of the lights[i]
         Vec3f shadow_pt, shadow_N;
@@ -120,8 +122,8 @@ class Scene {
         if (scene_intersect(shadow_orig, light_dir, this->objects, shadow_pt, shadow_N, tmpmaterial) && (shadow_pt-shadow_orig).norm() < light_distance)
             continue;
 
-        diffuse_light_intensity  += this->lightsources[i].intensity * std::max(0.f, light_dir*N);
-        specular_light_intensity += powf(std::max(0.f, -reflect(-light_dir, N)*ray.direction), material.specular_exponent)*(this->lightsources)[i].intensity;
+        diffuse_light_intensity  += this->light_sources[i].intensity * std::max(0.f, light_dir*N);
+        specular_light_intensity += powf(std::max(0.f, -reflect(-light_dir, N)*ray.direction), material.specular_exponent)*(this->light_sources)[i].intensity;
     }
     return (material.diffuse_color * diffuse_light_intensity * material.albedo[0] + Vec3f(1., 1., 1.)*specular_light_intensity * material.albedo[1] + reflect_color*material.albedo[2] + refract_color);
   }
diff --git a/vect.h b/vect.h
index 16da3fe..2ce40fa 100644
--- a/vect.h
+++ b/vect.h
@@ -1,6 +1,7 @@
 #ifndef __GEOMETRY_H__
 #define __GEOMETRY_H__
 
+#include <inttypes.h>
 #include <cmath>
 #include <vector>
 #include <cassert>
@@ -10,26 +11,54 @@ template <size_t DIM, typename T> struct vec {
     vec() { for (size_t i=DIM; i--; data_[i] = T()); }
           T& operator[](const size_t i)       { assert(i<DIM); return data_[i]; }
     const T& operator[](const size_t i) const { assert(i<DIM); return data_[i]; }
+    vec(const vec& v)
+    {
+        *this = v;
+    }
+    vec& operator=(const vec& v)
+    {
+        for (size_t i=DIM; i--; data_[i] = v[i]);
+        return *this;
+    }
+    T length() const
+    {
+        T l{};
+        for (size_t i=DIM; i--; l += data_[i] * data_[i]);
+        return sqrt(l);
+    }
 private:
     T data_[DIM];
 };
 
 
-typedef vec<3, float> Vec3f;
 
-typedef vec<4, float> Vec4f;
 
 
 template <typename T> struct vec<3,T> {
+    const size_t DIM = 3;
     vec() : x(T()), y(T()), z(T()) {}
     vec(T X, T Y, T Z) : x(X), y(Y), z(Z) {}
+    vec(const vec& v) : x(v.x), y(v.y), z(v.z) { }
           T& operator[](const size_t i)       { assert(i<3); return i<=0 ? x : (1==i ? y : z); }
     const T& operator[](const size_t i) const { assert(i<3); return i<=0 ? x : (1==i ? y : z); }
     float norm() { return std::sqrt(x*x+y*y+z*z); }
     vec<3,T> & normalize(T l=1) { *this = (*this)*(l/norm()); return *this; }
     T x,y,z;
+    T length() const
+    {
+        T l = x * x + y * y + z * z;
+        return sqrt(l);
+    }
+    vec& operator=(const vec& v)
+    {
+        x = v.x, y = v.y, z = v.z;
+        return *this;
+    }
+
 };
 
+typedef vec<3, float> Vec3f;
+
 template <typename T> struct vec<4,T> {
     vec() : x(T()), y(T()), z(T()), w(T()) {}
     vec(T X, T Y, T Z, T W) : x(X), y(Y), z(Z), w(W) {}
@@ -37,6 +66,7 @@ template <typename T> struct vec<4,T> {
     const T& operator[](const size_t i) const { assert(i<4); return i<=0 ? x : (1==i ? y : (2==i ? z : w)); }
     T x,y,z,w;
 };
+typedef vec<4, float> Vec4f;
 
 template<size_t DIM,typename T> T operator*(const vec<DIM,T>& lhs, const vec<DIM,T>& rhs) {
     T ret = T();

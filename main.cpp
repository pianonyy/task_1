#include "scene.h"
#include "float.h"
#include "objects.h"
#include "vect.h"


#include <iostream>
#include <fstream>

using namespace std;


int main() {

  const int Height = 512;
  const int Width = 512;
  const double fov = M_PI / 3.;
  Vec3f pix_col(0, 0, 0);
  Vec3f pix_col1(0, 0, 0);
  std::vector<Vec3f> framebuffer( Width * Height);

  Scene scene = Scene();


  Material glass(1.5, Vec4f(0.0,  0.5, 0.1, 0.8), Vec3f(0.6, 0.7, 0.8),  125.);
  Material     mirror(1.0, Vec4f(0.0, 10.0, 0.8, 0.0), Vec3f(1.0, 1.0, 1.0), 1425.);
  Material red_rubber(1.0, Vec4f(0.9,  0.1, 0.0, 0.0), Vec3f(0.3, 0.1, 0.1),   10.);
  Material      ivory(1.0, Vec4f(0.6,  0.3, 0.1, 0.0), Vec3f(0.4, 0.4, 0.3),   50.);

  Sphere sphere1 (Vec3f(7,    5,   -18), 2, glass);

  Sphere sphere2 (Vec3f(-3,    0,   -16), 2,     ivory);

	LightSource light1 = LightSource(Vec3f(30, 50, -25), 1.8);
  LightSource light2 = LightSource(Vec3f(30, -50, 25), 1);
  LightSource light3 = LightSource(Vec3f(18,18, -20), 1);

  scene.add(&sphere1);
  scene.add(&sphere2);


	scene.add(light1);
  scene.add(light2);
  scene.add(light3);

//loading picture in ppm format



  for (int i = 0; i < Height; i++) {
    for (int j = 0; j < Width; j++)  {
      double dir_x = (i + 0.5) - Width / 2.;
      double dir_y = - (j + 0.5) + Height / 2.;
      double dir_z = - Height / (2. * tan(fov / 2.));
      pix_col = scene.trace(dir_x, dir_y, dir_z);
      pix_col1 = scene.trace(dir_x,dir_y,dir_z);
      Vec3f pix_average ( ((pix_col.x + pix_col1.x)/2),((pix_col.y + pix_col1.y)/2),((pix_col.z + pix_col1.z)/2) );
      framebuffer[i + j * Width] = pix_average;

    }
  }

  std::ofstream ofs; // save the framebuffer to file
  ofs.open("./out.ppm",std::ios::binary);
  ofs << "P6\n" << Width << " " << Height << "\n255\n";
  for (size_t i = 0; i < Height*Width; ++i) {
      Vec3f &c = framebuffer[i];
      float max = std::max(c[0], std::max(c[1], c[2]));
      if (max>1) c = c*(1. / max);
      for (size_t j = 0; j < 3; j++) {
          ofs << (char)(255 * std::max(0.f, std::min(1.f, framebuffer[i][j])));
      }
  }
  ofs.close();
  return 0;
}

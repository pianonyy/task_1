#include "scene.h"
#include "float.h"
#include "objects.h"
#include "vect.h"


#include <iostream>
#include <fstream>

using namespace std;


int main() {

  const int Height = 500;
  const int Width = 500;
  const double fov = M_PI / 3.;
  Vec3f pix_col(0, 0, 0);

  Scene scene = Scene();


  Material glass(1.5, Vec4f(0.0,  0.5, 0.1, 0.8), Vec3f(0.6, 0.7, 0.8),  125.);

  Sphere sphere1 (Vec3f(200, 250, 220), 2, glass);


	LightSource light1 = LightSource(Vec3f(10,10, 10), 23);
  //Lightsource light2 = Lightsource(Vec3f(100,255, 0), 23);
  //Lightsource light3 = Lightsource(Vec3f(255,255, 255), 12);

  scene.add(&sphere1);


	scene.add(light1);

//loading picture in ppm format
  ofstream my_Image ("image.ppm");
  if (my_Image.is_open ()) {
      my_Image << "P3\n" << Width << " " << Height << " 255\n";
      for (int i = 0; i < Height; i++) {
        for (int j = 0; j < Width; j++)  {
          double dir_x = (i + 0.5) - Width / 2.;
          double dir_y = - (j + 0.5) + Height / 2.;
          double dir_z = - Height / (2. * tan(fov / 2.));
          pix_col = scene.trace(dir_x, dir_y, dir_z);
          my_Image <<pix_col.x << ' ' << pix_col.y << ' ' << pix_col.z << "\n";
        }
      }
      my_Image.close();
  }
  else
    cout << "Could not open the file";

  return 0;
}

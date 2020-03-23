#include "scene.h"
#include "float.h"
#include "objects.h"
#include "vect.h"


#include <iostream>
#include <fstream>
#include <unordered_map>

using namespace std;


int main(int argc, const char** argv) {

  std::unordered_map<std::string, std::string> cmdLineParams;

  for(int i=0; i<argc; i++)
  {
    std::string key(argv[i]);

    if(key.size() > 0 && key[0]=='-')
    {
      if(i != argc-1) // not last argument
      {
        cmdLineParams[key] = argv[i+1];
        i++;
      }
      else
        cmdLineParams[key] = "";
    }
  }

  std::string outFilePath = "zout.ppm";
  if(cmdLineParams.find("-out") != cmdLineParams.end())
    outFilePath = cmdLineParams["-out"];

  int sceneId = 0;
  if(cmdLineParams.find("-scene") != cmdLineParams.end())
    sceneId = atoi(cmdLineParams["-scene"].c_str());

  uint32_t color = 0;
  if(sceneId == 1)
    color = 1;
  else if(sceneId == 2)
    color = 2;
  else if(sceneId == 3)
    color = 3;

  const int Height = 500;
  const int Width = 500;
  const double fov = M_PI / 3.;
  Vec3f pix_col (0, 0, 0);
  //Vec3f pix_col1(0, 0, 0);
  std::vector<Vec3f> framebuffer( Width * Height);
  std::vector<Vec3f> framebuffer_out( Width * Height);

  Scene scene = Scene();


  Material glass(1.5, Vec4f(0.0,  0.5, 0.1, 0.8), Vec3f(0.6, 0.7, 0.8),  125.);
  Material     mirror(1.0, Vec4f(0.0, 10.0, 0.8, 0.0), Vec3f(1.0, 1.0, 1.0), 1425.);
  Material red_rubber(1.0, Vec4f(0.9,  0.1, 0.0, 0.0), Vec3f(0.3, 0.1, 0.1),   50.);
  Material      ivory(1.0, Vec4f(0.6,  0.3, 0.1, 0.0), Vec3f(0.4, 0.4, 0.3),   50.);

  Sphere sphere1 (Vec3f(7,    5,   -18), 2, mirror);
  Cylinder cylinder1 (Vec3f(-3,    0,   -16), Vec3f(0.1,    0.9,   0.8), 3., 4., red_rubber);
  Sphere sphere2 (Vec3f(-3,    0,   -16), 2,     mirror);
  Plane plane1 (Vec3f(-7.0,    -7.0,   3.0), Vec3f(0.0, 1.0, 0.0 ), glass);

  Sphere sphere3 (Vec3f(0.0, 1.5, 0.5), 1.5,     glass);
	LightSource light1 = LightSource(Vec3f(-30, 50,  25), 5.8);
  LightSource light2 = LightSource(Vec3f( 30, 50, -25), 2);
  LightSource light3 = LightSource(Vec3f( 18, 18, -20), 1);

  scene.add(&sphere1);
  scene.add(&cylinder1);
  scene.add(&sphere3);
  //scene.add(&plane1);

	scene.add(light1);
  //scene.add(light2);
  //scene.add(light3);

//loading picture in ppm format


  float eps = 0.9;
  for (int i = 0; i < Height; i++) {
    for (int j = 0; j < Width; j++)  {
      float dir_x = 0.0;
      float dir_y = 0.0;
      float dir_z = 0.0;

      dir_x += (i + 0.5) - Width / 2.;
      dir_y += - (j + 0.5) + Height / 2.;
      dir_z += - Height / (2. * tan(fov / 2.));

      pix_col = scene.trace(dir_x , dir_y  , dir_z);
      framebuffer[i + j * Width] = pix_col;
    }
  }

  //smooth picture
  int ker[3][3];
  for (int i = 0; i < 3; i++){
    for (int j = 0; j < 3; j++){
      if ((i == 0 && j == 0) || (i == 0 && j == 2) || (i == 2 && j == 0) || (i == 2 && j == 2))
        ker[i][j] = 1;
      else
        ker[i][j] = 0;
    }
  }

  for (int i = 0; i < Height; i++) {
    for (int j = 0; j < Width; j++) {
      float rSum = 0.0;
      float gSum = 0.0;
      float bSum = 0.0;
      float kSum = 0.0;
      for(int k = 0; k < 3; k++){
        for(int p = 0; p < 3; p++){
          int pixel_pos_x = i + (k - (3 / 2));
          int pixel_pos_y = j + (p - (3 / 3));
          if ((pixel_pos_x < 0) || (pixel_pos_x >= Width) || (pixel_pos_x < 0) || (pixel_pos_x >= Height))
            continue;
          float red = framebuffer[3 * (Width * pixel_pos_y + pixel_pos_x)].x;
          float green = framebuffer[3 * (Width * pixel_pos_y + pixel_pos_x)].y;
          float blue = framebuffer[3 * (Width * pixel_pos_y + pixel_pos_x)].z;

          rSum += red * ker[i][j];
          gSum += green * ker[i][j];
          bSum += blue * ker[i][j];

          kSum += ker[i][j];
        }
      }
      if (kSum <= 0) kSum = 1;

      rSum /= kSum;
      if (rSum < 0) rSum = 0;
      if (rSum > 255) rSum = 255;

      gSum /= kSum;
      if (gSum < 0) gSum = 0;
      if (gSum > 255) gSum = 255;

      bSum /= kSum;
      if (bSum < 0) bSum = 0;
      if (bSum > 255) bSum = 255;

      framebuffer_out[3 * (Width * j + i)].x = rSum;
      framebuffer_out[3 * (Width * j + i)].y = gSum;
      framebuffer_out[3 * (Width * j + i)].z = bSum;
    }
  }


  std::ofstream ofs; // save the framebuffer to file
  ofs.open(outFilePath,std::ios::binary);
  ofs << "P6\n" << Width << " " << Height << "\n255\n";
  for (size_t i = 0; i < Height * Width; ++i) {
      Vec3f &c = framebuffer_out[i];
      float max = std::max(c[0], std::max(c[1], c[2]));
      if (max>1) c = c*(1. / max);
      for (size_t j = 0; j < 3; j++) {
          ofs << (char)(255 * std::max(0.f, std::min(1.f, framebuffer_out[i][j])));
      }
  }
  ofs.close();
  return 0;
}

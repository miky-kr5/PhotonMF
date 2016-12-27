#include <iostream>
#include <iomanip>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <omp.h>
#include <glm/glm.hpp>

#include "ray.hpp"
#include "figure.hpp"
#include "sphere.hpp"
#include "plane.hpp"
#include "light.hpp"
#include "tracer.hpp"

using namespace std;
using namespace glm;

#define MAX_RECURSION 9

static const char * OUT_FILE = "output.ppm";

static char * input_file;
static int g_samples = 25;
static float g_fov = 90.0f;
static int g_w = 640;
static int g_h = 480;
static vec3 ** image;

int main(int argc, char ** argv) {
  FILE * out;
  Sphere * s;
  Plane * p;
  Light * l;
  Ray r;
  vec2 sample;
  vector<Figure *> figures;
  vector<Light *> lights;
  Tracer tracer;
  int total;
  int current = 0;

  if(argc < 2 || argc == 3 || argc > 7) {
    cerr << "USAGE: " << argv[0] << " IN FILE [OUT FILE [HEIGHT WIDTH [SAMPLES [FIELD OF VIEW]]]]" << endl;
    return EXIT_FAILURE;
  }

  input_file = argv[1];

  if(argc >= 5) {
    g_h = atoi(argv[3]);
    if (g_h <= 0) {
      cerr << "USAGE: " << argv[0] << " IN FILE [OUT FILE [HEIGHT WIDTH [SAMPLES [FIELD OF VIEW]]]]" << endl;
      cerr << "HEIGHT must be positive" << endl;
      return EXIT_FAILURE;
    }

    g_w = atoi(argv[4]);
    if (g_w <= 0) {
      cerr << "USAGE: " << argv[0] << " IN FILE [OUT FILE [HEIGHT WIDTH [SAMPLES [FIELD OF VIEW]]]]" << endl;
      cerr << "WIDTH must be positive" << endl;
      return EXIT_FAILURE;
    }
    
    if(argc >= 6) {
      g_samples = atoi(argv[5]);
      if (g_samples <= 0) {
	  cerr << "USAGE: " << argv[0] << " IN FILE [OUT FILE [HEIGHT WIDTH [SAMPLES [FIELD OF VIEW]]]]" << endl;
	  cerr << "SAMPLES must be greater than 1" << endl;
	  return EXIT_FAILURE;
      }
      
      if(argc >= 7) {
	g_fov = atof(argv[6]);
	if (g_fov <= 0) {
	  cerr << "USAGE: " << argv[0] << " IN FILE [OUT FILE [HEIGHT WIDTH [SAMPLES [FIELD OF VIEW]]]]" << endl;
	  cerr << "FIELD OF VIEW must be greater than 1.0" << endl;
	  return EXIT_FAILURE;
	}
      }
    }
  }

  out = fopen(argc >= 3 ? argv[2] : OUT_FILE, "wb");

  image = new vec3*[g_h];
  for (int i = 0; i < g_h; i++) {
    image[i] = new vec3[g_w];
  }

  s = new Sphere(1.0f, 1.0f, -2.0f, 0.5f);
  s->set_color(1.0f, 0.0f, 0.0f);
  figures.push_back(static_cast<Figure *>(s));

  s = new Sphere(-1.0f, 1.0f, -2.0f, 0.5f);
  s->set_color(0.0f, 1.0f, 0.0f);
  figures.push_back(static_cast<Figure *>(s));

  s = new Sphere(1.0f, -1.0f, -2.0f, 0.5f);
  s->set_color(0.0f, 0.0f, 1.0f);
  figures.push_back(static_cast<Figure *>(s));

  s = new Sphere(-1.0f, -1.0f, -2.0f, 0.5f);
  s->set_color(1.0f, 0.0f, 1.0f);
  figures.push_back(static_cast<Figure *>(s));

  s = new Sphere(0.0f, 0.0f, -2.0f, 1.0f);
  s->set_color(1.0f, 1.0f, 0.0f);
  figures.push_back(static_cast<Figure *>(s));

  p = new Plane(vec3(0.0f, -1.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
  p->set_color(1.0f, 0.5f, 0.4f);
  figures.push_back(static_cast<Figure *>(p));

  s = new Sphere(0.0f, 0.0f, -1.0f, 0.25f);
  s->set_color(1.0f, 1.0f, 1.0f);
  figures.push_back(static_cast<Figure *>(s));

  s = new Sphere(-1.5f, 0.0f, -2.0f, 0.5f);
  s->set_color(1.0f, 1.0f, 1.0f);
  figures.push_back(static_cast<Figure *>(s));

  s = new Sphere(1.5f, 0.0f, -2.0f, 0.5f);
  s->set_color(1.0f, 1.0f, 1.0f);
  figures.push_back(static_cast<Figure *>(s));

  s = new Sphere(0.0f, 1.5f, -2.0f, 0.5f);
  s->set_color(1.0f, 1.0f, 1.0f);
  figures.push_back(static_cast<Figure *>(s));

  l = new Light();
  l->m_position = normalize(vec3(1.0f, 1.0f, 1.0f));
  l->m_diffuse = vec3(0.0f, 1.0f, 1.0f);
  lights.push_back(l);

  l = new Light();
  l->m_position = normalize(vec3(-1.0f, 1.0f, 1.0f));
  l->m_diffuse = vec3(1.0f, 1.0f, 0.0f);
  lights.push_back(l);

  l = new Light();
  l->m_position = normalize(vec3(0.0f, 1.0f, -1.0f));
  l->m_diffuse = vec3(1.0f, 0.0f, 1.0f);
  lights.push_back(l);

  tracer = Tracer(g_h, g_w, g_fov);

  total = g_h * g_w * g_samples;

#pragma omp parallel for schedule(dynamic, 1) private(r, sample) shared(current)
  for (int i = 0; i < g_h; i++) {
    for (int j = 0; j < g_w; j++) {
      for (int k = 0; k < g_samples; k++) {
	sample = tracer.sample_pixel(i, j);
	r = Ray(normalize(vec3(sample, -1.0f) - vec3(0.0f, 0.0f, 0.0f)), vec3(0.0f, 0.0f, 0.0f));
	image[i][j] += tracer.trace_ray(r, figures, lights, MAX_RECURSION);
#pragma omp critical
	{
	  current++;
	}
      }
      image[i][j] /= g_samples;
    }
#pragma omp critical
    {
      cout << "\r" << setw(3) << static_cast<int>((static_cast<float>(current) / static_cast<float>(total)) * 100.0f) << "% done";
    }
  }
  cout << endl;

  for (size_t i = 0; i < figures.size(); i++) {
    delete figures[i];
  }
  figures.clear();

  for (size_t i = 0; i < figures.size(); i++) {
    delete lights[i];
  }
  lights.clear();

  fprintf(out, "P6 %d %d %d ", g_w, g_h, 255);
  for (int i = 0; i < g_h; i++) {
    for (int j = 0; j < g_w; j++) {
      fputc(static_cast<int>(image[i][j].r * 255.0f), out);
      fputc(static_cast<int>(image[i][j].g * 255.0f), out);
      fputc(static_cast<int>(image[i][j].b * 255.0f), out);
    }
  }
  fclose(out);

  for (int i = 0; i < g_h; i++)
    delete[] image[i];
  delete[] image;

  return EXIT_SUCCESS;
}

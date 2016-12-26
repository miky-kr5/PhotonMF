#include <iostream>
#include <vector>
#include <limits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>

#include <omp.h>
#include <glm/glm.hpp>

#include "ray.hpp"
#include "figure.hpp"
#include "sphere.hpp"

using namespace std;
using namespace glm;

static const vec3 BCKG_COLOR = vec3(0.16f, 0.66f, 0.72f);
static const char * OUT_FILE = "output.ppm";

static char * input_file;
static int g_samples = 25;
static float g_fov = 90.0f;
static int g_w = 640;
static int g_h = 480;
static float g_aspect_ratio = static_cast<float>(g_w) / g_h;
static vec3 ** image;

float random01();
vec2 sample_pixel(int i, int j);

int main(int argc, char ** argv) {
  FILE * out;
  float t;
  float _t;
  Sphere * s;
  Figure * _f;
  Ray r;
  vec2 sample;
  vector<Figure *> figures;

  if(argc < 2 || argc == 3 || argc > 6) {
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
  s->set_color(0.5f, 0.5f, 0.5f);
  figures.push_back(static_cast<Figure *>(s));

  s = new Sphere(0.0f, 0.0f, -2.0f, 1.0f);
  s->set_color(1.0f, 1.0f, 0.0f);
  figures.push_back(static_cast<Figure *>(s));

#pragma omp parallel for schedule(dynamic, 1) private(r, sample, _f, t, _t)
  for (int i = 0; i < g_h; i++) {
    for (int j = 0; j < g_w; j++) {
      for (int k = 0; k < g_samples; k++) {
	sample = sample_pixel(i, j);
	r = Ray(normalize(vec3(sample, -1.0f) - vec3(0.0f, 0.0f, 0.0f)), vec3(0.0f, 0.0f, 0.0f));
	t = numeric_limits<float>::max();
	_f = NULL;

	for (size_t f = 0; f < figures.size(); f++) {
	  if (figures[f]->intersect(r, _t) && _t < t) {
	    t = _t;
	    _f = figures[f];
	  }
	}

	if (_f != NULL) {
	  image[i][j] += vec3(_f->color);
	} else {
	  image[i][j] += vec3(BCKG_COLOR);
	}
      }

      image[i][j] /= g_samples;
    }
  }

  for (size_t f = 0; f < figures.size(); f++) {
    delete static_cast<Sphere *>(figures[f]);
  }
  figures.clear();

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

inline float random01() {
  return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}

vec2 sample_pixel(int i, int j) {
  float pxNDC;
  float pyNDC;
  float pxS;
  float pyS;
  pyNDC = (static_cast<float>(i) + random01()) / g_h;
  pyS = (1.0f - (2.0f * pyNDC)) * tan(radians(g_fov) / 2);
  pxNDC = (static_cast<float>(j) + random01()) / g_w;
  pxS = (2.0f * pxNDC) - 1.0f;
  pxS *= g_aspect_ratio * tan(radians(g_fov) / 2);

  return vec2(pxS, pyS);
}

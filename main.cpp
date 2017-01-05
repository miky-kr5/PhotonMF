#include <iostream>
#include <iomanip>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <omp.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "ray.hpp"
#include "figure.hpp"
#include "sphere.hpp"
#include "plane.hpp"
#include "disk.hpp"
#include "light.hpp"
#include "directional_light.hpp"
#include "point_light.hpp"
#include "tracer.hpp"
#include "path_tracer.hpp"

using namespace std;
using namespace glm;

static const char * OUT_FILE = "output.ppm";

static char * input_file;
static int g_samples = 25;
static float g_fov = 90.0f;
static int g_w = 640;
static int g_h = 480;
static vec3 ** image;

static void scene_1(vector<Figure *> & vf, vector<Light *> & vl, mat4x4 & i_model_view);
static void scene_2(vector<Figure *> & vf, vector<Light *> & vl, mat4x4 & i_model_view);
static void scene_3(vector<Figure *> & vf, vector<Light *> & vl, mat4x4 & i_model_view);

int main(int argc, char ** argv) {
  FILE * out;
  Ray r;
  vec2 sample;
  vector<Figure *> figures;
  vector<Light *> lights;
  Tracer * tracer;
  size_t total;
  size_t current = 0;
  mat4x4 i_model_view;
  vec4 dir, orig;

  if(argc < 2 || argc > 7) {
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

  scene_2(figures, lights, i_model_view);

  tracer = static_cast<Tracer *>(new PathTracer(g_h, g_w, g_fov, true));

  total = g_h * g_w * g_samples;

#pragma omp parallel for schedule(dynamic, 1) private(r, sample, dir, orig) shared(current)
  for (int i = 0; i < g_h; i++) {
    for (int j = 0; j < g_w; j++) {
      for (int k = 0; k < g_samples; k++) {
	sample = tracer->sample_pixel(i, j);
	dir = i_model_view * normalize(vec4(sample, -1.0f, 1.0f) - vec4(0.0f, 0.0f, 0.0f, 1.0f));
	orig = i_model_view * vec4(0.0f, 0.0f, 0.0f, 1.0f);
	r = Ray(dir.x, dir.y, dir.z, orig.x, orig.y, orig.z);
	image[i][j] += tracer->trace_ray(r, figures, lights, 0);
#pragma omp critical
	{
	  current++;
	}
      }
      image[i][j] /= g_samples;
    }
#pragma omp critical
    {
      cout << "\r" << setw(3) << static_cast<size_t>((static_cast<double>(current) / static_cast<double>(total)) * 100.0) << "% done";
    }
  }
  cout << endl;

  delete tracer;

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
      fputc(static_cast<int>(pow(image[i][j].r, 1.0f / 2.2f) * 255.0f), out);
      fputc(static_cast<int>(pow(image[i][j].g, 1.0f / 2.2f) * 255.0f), out);
      fputc(static_cast<int>(pow(image[i][j].b, 1.0f / 2.2f) * 255.0f), out);
    }
  }
  fclose(out);

  for (int i = 0; i < g_h; i++)
    delete[] image[i];
  delete[] image;

  return EXIT_SUCCESS;
}

static void scene_1(vector<Figure *> & vf, vector<Light *> & vl, mat4x4 & i_model_view) {
  Sphere * s;
  Plane * p;
  Disk * d;
  DirectionalLight * l;

  s = new Sphere(1.0f, 1.0f, -2.0f, 0.5f);
  s->m_mat.m_diffuse = vec3(1.0f, 0.0f, 0.0f);
  vf.push_back(static_cast<Figure *>(s));

  s = new Sphere(-1.0f, 1.0f, -2.0f, 0.5f);
  s->m_mat.m_diffuse = vec3(0.0f, 1.0f, 0.0f);
  vf.push_back(static_cast<Figure *>(s));

  s = new Sphere(1.0f, -1.0f, -2.0f, 0.5f);
  s->m_mat.m_diffuse = vec3(0.0f, 0.0f, 1.0f);
  vf.push_back(static_cast<Figure *>(s));

  s = new Sphere(-1.0f, -1.0f, -2.0f, 0.5f);
  s->m_mat.m_diffuse = vec3(1.0f, 0.0f, 1.0f);
  vf.push_back(static_cast<Figure *>(s));

  s = new Sphere(0.0f, 0.0f, -2.0f, 1.0f);
  s->m_mat.m_diffuse = vec3(1.0f, 1.0f, 0.0f);
  vf.push_back(static_cast<Figure *>(s));

  p = new Plane(vec3(0.0f, -1.5f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
  p->m_mat.m_diffuse = vec3(1.0f, 0.5f, 0.4f);
  vf.push_back(static_cast<Figure *>(p));

  s = new Sphere(-1.5f, 0.0f, -2.0f, 0.5f);
  s->m_mat.m_diffuse = vec3(1.0f, 1.0f, 1.0f);
  s->m_mat.m_rho = 0.3f;
  vf.push_back(static_cast<Figure *>(s));

  s = new Sphere(1.5f, 0.0f, -2.0f, 0.5f);
  s->m_mat.m_diffuse = vec3(1.0f, 1.0f, 1.0f);
  s->m_mat.m_rho = 0.08f;
  s->m_mat.m_refract = true;
  s->m_mat.m_ref_index = 1.1f;
  vf.push_back(static_cast<Figure *>(s));

  s = new Sphere(0.0f, 1.5f, -2.0f, 0.5f);
  s->m_mat.m_diffuse = vec3(1.0f, 1.0f, 1.0f);
  s->m_mat.m_rho = 0.5f;
  vf.push_back(static_cast<Figure *>(s));

  s = new Sphere(0.0f, 0.0f, -1.0f, 0.25f);
  s->m_mat.m_diffuse = vec3(1.0f, 1.0f, 1.0f);
  s->m_mat.m_rho = 0.1f;
  vf.push_back(static_cast<Figure *>(s));

  d = new Disk(vec3(-0.0f, -0.0f, -0.5f), vec3(0.0f, 0.0f, 0.1f), 0.25f);
  d->m_mat.m_diffuse = vec3(1.0f, 0.0f, 0.0f);
  d->m_mat.m_rho = 0.3f;
  d->m_mat.m_refract = true;
  d->m_mat.m_ref_index = 1.33f;
  vf.push_back(static_cast<Figure *>(d));

  l = new DirectionalLight();
  l->m_position = normalize(vec3(1.0f, 1.0f, 1.0f));
  l->m_diffuse = vec3(0.0f, 1.0f, 1.0f);
  vl.push_back(static_cast<Light *>(l));

  l = new DirectionalLight();
  l->m_position = normalize(vec3(-1.0f, 1.0f, 1.0f));
  l->m_diffuse = vec3(1.0f, 1.0f, 0.0f);
  vl.push_back(static_cast<Light *>(l));

  l = new DirectionalLight();
  l->m_position = normalize(vec3(0.0f, 1.0f, -1.0f));
  l->m_diffuse = vec3(1.0f, 0.0f, 1.0f);
  vl.push_back(static_cast<Light *>(l));
}

static void scene_2(vector<Figure *> & vf, vector<Light *> & vl, mat4x4 & i_model_view) {
  Sphere * s;
  Plane * p;
  Disk * d;
  PointLight * l;

  s = new Sphere(0.2f, 0.0f, -0.75f, 0.25f);
  s->m_mat.m_diffuse = vec3(1.0f);
  vf.push_back(static_cast<Figure *>(s));

  p = new Plane(vec3(0.0f, -1.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
  p->m_mat.m_diffuse = vec3(0.0f, 1.0f, 0.0f);
  vf.push_back(static_cast<Figure *>(p));

  p = new Plane(vec3(-2.0f, 0.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f));
  p->m_mat.m_diffuse = vec3(1.0f, 0.0f, 0.0f);
  vf.push_back(static_cast<Figure *>(p));

  p = new Plane(vec3(2.0f, 0.0f, 0.0f), vec3(-1.0f, 0.0f, 0.0f));
  p->m_mat.m_diffuse = vec3(0.0f, 0.0f, 1.0f);
  vf.push_back(static_cast<Figure *>(p));

  p = new Plane(vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f));
  p->m_mat.m_diffuse = vec3(1.0f, 1.0f, 1.0f);
  vf.push_back(static_cast<Figure *>(p));

  p = new Plane(vec3(0.0f, 0.0f, -2.0f), vec3(0.0f, 0.0f, 1.0f));
  p->m_mat.m_diffuse = vec3(1.0f, 0.0f, 1.0f);
  vf.push_back(static_cast<Figure *>(p));

  p = new Plane(vec3(0.0f, 0.0f, 1.1f), vec3(0.0f, 0.0f, -1.0f));
  p->m_mat.m_diffuse = vec3(1.0f, 1.0f, 1.0f);
  vf.push_back(static_cast<Figure *>(p));

  s = new Sphere(-0.5f, -0.5f, -1.5f, 0.5f);
  s->m_mat.m_diffuse = vec3(1.0f, 1.0f, 0.0f);
  s->m_mat.m_rho = 0.9f;
  vf.push_back(static_cast<Figure *>(s));

  s = new Sphere(-0.5f, -0.5f, 0.6f, 0.5f);
  s->m_mat.m_diffuse = vec3(1.0f, 1.0f, 0.0f);
  s->m_mat.m_rho = 0.1f;
  s->m_mat.m_refract = true;
  s->m_mat.m_ref_index = 1.33f;
  vf.push_back(static_cast<Figure *>(s));

  d = new Disk(vec3(-0.25f, 1.0f, -1.0f), vec3(1.0f, 0.0f, 0.0f), 0.25f);
  d->m_mat.m_diffuse = vec3(1.0f);
  vf.push_back(static_cast<Figure *>(d));

  d = new Disk(vec3(0.25f, 1.0f, -1.0f), vec3(-1.0f, 0.0f, 0.0f), 0.25f);
  d->m_mat.m_diffuse = vec3(1.0f);
  vf.push_back(static_cast<Figure *>(d));

  d = new Disk(vec3(0.0f, 1.0f, -1.25f), vec3(0.0f, 0.0f, 1.0f), 0.25f);
  d->m_mat.m_diffuse = vec3(1.0f);
  vf.push_back(static_cast<Figure *>(d));

  d = new Disk(vec3(0.0f, 1.0f, -0.75f), vec3(0.0f, 0.0f, -1.0f), 0.25f);
  d->m_mat.m_diffuse = vec3(1.0f);
  vf.push_back(static_cast<Figure *>(d));

  l = new PointLight();
  l->m_position = vec3(0.0f, 0.9f, -1.0f);
  l->m_diffuse = vec3(1.0f);
  vl.push_back(static_cast<Light *>(l));
}

static void scene_3(vector<Figure *> & vf, vector<Light *> & vl, mat4x4 & i_model_view) {
  Sphere * s;
  Plane * p;
  DirectionalLight * l;
  vec3 eye = vec3(0.0f, 1.5f, 0.0f);
  vec3 center = vec3(0.0f, 0.0f, -2.0f);
  vec3 left = vec3(-1.0f, 0.0f, 0.0f);
  vec3 up = cross(center - eye, left);

  s = new Sphere(0.0f, -0.15f, -2.0f, 1.0f);
  s->m_mat.m_diffuse = vec3(1.0f, 0.5f, 0.0f);
  s->m_mat.m_specular = vec3(0.3f);
  s->m_mat.m_shininess = 5.0f;
  s->m_mat.m_rho = 0.4f;
  s->m_mat.m_refract = true;
  s->m_mat.m_ref_index = 1.33f;
  vf.push_back(static_cast<Figure *>(s));

  s = new Sphere(0.0f, -0.15f, -2.0f, 0.5f);
  s->m_mat.m_diffuse = vec3(0.0f);
  s->m_mat.m_specular = vec3(0.0f);
  s->m_mat.m_rho = 0.0f;
  s->m_mat.m_refract = true;
  s->m_mat.m_ref_index = 2.6f;
  vf.push_back(static_cast<Figure *>(s));

  s = new Sphere(2.0f, 0.0f, -2.0f, 1.0f);
  s->m_mat.m_diffuse = vec3(1.0f, 0.0f, 1.0f);
  s->m_mat.m_rho = 1.0f;
  vf.push_back(static_cast<Figure *>(s));

  s = new Sphere(-1.0f, 0.25f, -3.25f, 1.0f);
  s->m_mat.m_diffuse = vec3(1.0f, 1.0f, 0.0f);
  s->m_mat.m_shininess = 20.0f;
  vf.push_back(static_cast<Figure *>(s));

  p = new Plane(vec3(0.0f, -1.5f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
  p->m_mat.m_diffuse = vec3(1.0f);
  p->m_mat.m_specular = vec3(0.0f);
  vf.push_back(static_cast<Figure *>(p));

  l = new DirectionalLight();
  l->m_position = normalize(vec3(-1.0f, 1.0f, -1.0f));
  l->m_diffuse = vec3(1.0f, 1.0f, 1.0f);
  l->m_specular = vec3(0.0f, 1.0f, 0.0f);
  vl.push_back(static_cast<Light *>(l));

  l = new DirectionalLight();
  l->m_position = normalize(vec3(0.0f, 1.0f, 1.0f));
  l->m_diffuse = vec3(1.0f, 0.0f, 0.0f);
  l->m_specular = vec3(1.0f, 0.0f, 0.0f);
  vl.push_back(static_cast<Light *>(l));

  l = new DirectionalLight();
  l->m_position = normalize(vec3(1.0f, 1.0f, -1.0f));
  l->m_diffuse = vec3(0.0f, 0.0f, 1.0f);
  l->m_specular = vec3(0.0f, 0.0f, 1.0f);
  vl.push_back(static_cast<Light *>(l));

  l = new DirectionalLight();
  l->m_position = normalize(vec3(1.0f, 0.0f, 1.0f));
  l->m_diffuse = vec3(0.5f);
  vl.push_back(static_cast<Light *>(l));

  i_model_view = inverse(lookAt(eye, center, up));
}

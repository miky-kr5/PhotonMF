#include <iostream>
#include <iomanip>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

#include <omp.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <FreeImage.h>

#include "ray.hpp"
#include "figure.hpp"
#include "sphere.hpp"
#include "plane.hpp"
#include "disk.hpp"
#include "light.hpp"
#include "directional_light.hpp"
#include "point_light.hpp"
#include "spot_light.hpp"
#include "tracer.hpp"
#include "path_tracer.hpp"
#include "whitted_tracer.hpp"
#include "brdf.hpp"
#include "phong_brdf.hpp"

using namespace std;
using namespace glm;

////////////////////////////////////////////
// Function prototypes.
////////////////////////////////////////////
static void scene_1(vector<Figure *> & vf, vector<Light *> & vl, mat4x4 & i_model_view);
static void scene_2(vector<Figure *> & vf, vector<Light *> & vl, mat4x4 & i_model_view);
static void scene_3(vector<Figure *> & vf, vector<Light *> & vl, mat4x4 & i_model_view);
static void scene_4(vector<Figure *> & vf, vector<Light *> & vl, mat4x4 & i_model_view);
static void print_usage(char ** const argv);
static void parse_args(int argc, char ** const argv);

////////////////////////////////////////////
// Constants.
////////////////////////////////////////////
static const char * OUT_FILE = "output.png";

////////////////////////////////////////////
// Global variables.
////////////////////////////////////////////
typedef enum TRACERS { NONE, WHITTED, MONTE_CARLO, JENSEN } tracer_t;

static char * g_input_file;
static char * g_out_file_name = NULL;
static int g_samples = 25;
static float g_fov = 45.f;
static int g_w = 640;
static int g_h = 480;
static vec3 ** image;
static tracer_t g_tracer = NONE;
static unsigned int g_max_depth = 5;

////////////////////////////////////////////
// Main function.
////////////////////////////////////////////
int main(int argc, char ** argv) {
  Ray r;
  vec2 sample;
  vector<Figure *> figures;
  vector<Light *> lights;
  Tracer * tracer;
  size_t total;
  size_t current = 0;
  mat4x4 i_model_view;
  vec4 dir, orig;
  FIBITMAP * output_bitmap;
  FREE_IMAGE_FORMAT fif;
  BYTE * bits;
  int bpp;

  parse_args(argc, argv);
  
  // Initialize everything.
  FreeImage_Initialise();
  
  image = new vec3*[g_h];
  for (int i = 0; i < g_h; i++) {
    image[i] = new vec3[g_w];
  }
  
  scene_2(figures, lights, i_model_view);

  // Create the tracer object.
  if (g_tracer == WHITTED)
    tracer = static_cast<Tracer *>(new WhittedTracer(g_h, g_w, g_fov, g_max_depth));
  else if(g_tracer == MONTE_CARLO)
    tracer = static_cast<Tracer *>(new PathTracer(g_h, g_w, g_fov, g_max_depth));
  else if(g_tracer == JENSEN) {
    cerr << "Photon mapping coming soon." << endl;
    return EXIT_FAILURE;
  } else {
    cerr << "Must specify a ray tracer with \"-t\"." << endl;
    print_usage(argv);
    return EXIT_FAILURE;
  }

  // Generate the image.
  total = g_h * g_w * g_samples;
#pragma omp parallel for schedule(dynamic, 1) private(r, sample, dir, orig) shared(current)
  for (int i = 0; i < g_h; i++) {
    for (int j = 0; j < g_w; j++) {
      for (int k = 0; k < g_samples; k++) {
	sample = tracer->sample_pixel(i, j);
	dir = i_model_view * normalize(vec4(sample, -0.5f, 1.0f) - vec4(0.0f, 0.0f, 0.0f, 1.0f));
	orig = i_model_view * vec4(0.0f, 0.0f, 0.0f, 1.0f);
	r = Ray(dir.x, dir.y, dir.z, orig.x, orig.y, orig.z);
	image[i][j] += tracer->trace_ray(r, figures, lights, 0);
#pragma omp atomic
	current++;
      }
      image[i][j] /= g_samples;
    }
#pragma omp critical
    cout << "\r" << setw(3) << static_cast<size_t>((static_cast<double>(current) / static_cast<double>(total)) * 100.0) << "% done";
  }
  cout << endl;

  // Copy the pixels to the output bitmap.  
  output_bitmap = FreeImage_Allocate(g_w, g_h, 24, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK);
  bpp = FreeImage_GetLine(output_bitmap) / FreeImage_GetWidth(output_bitmap);
  for (unsigned int y = 0; y < FreeImage_GetHeight(output_bitmap); y++) {
    bits = FreeImage_GetScanLine(output_bitmap, y);
    for (unsigned int x = 0; x < FreeImage_GetWidth(output_bitmap); x++) {
      bits[FI_RGBA_RED] = static_cast<BYTE>(pow(image[g_h - 1 - y][x].r, 1.0f / 2.2f) * 255.0f);
      bits[FI_RGBA_GREEN] = static_cast<BYTE>(pow(image[g_h - 1 - y][x].g, 1.0f / 2.2f) * 255.0f);
      bits[FI_RGBA_BLUE] = static_cast<BYTE>(pow(image[g_h - 1 - y][x].b, 1.0f / 2.2f) * 255.0f);
      bits += bpp;
    }
  }

  // Save the output image.
  fif = FreeImage_GetFIFFromFilename(g_out_file_name != NULL ? g_out_file_name : OUT_FILE);
  FreeImage_Save(fif, output_bitmap, g_out_file_name != NULL ? g_out_file_name : OUT_FILE);
  FreeImage_Unload(output_bitmap);

  // Clean up.
  if (g_out_file_name != NULL)
    free(g_out_file_name);

  delete tracer;

  for (size_t i = 0; i < figures.size(); i++) {
    delete figures[i];
  }
  figures.clear();

  for (size_t i = 0; i < figures.size(); i++) {
    delete lights[i];
  }
  lights.clear();

  for (int i = 0; i < g_h; i++)
    delete[] image[i];
  delete[] image;

  FreeImage_DeInitialise();
  
  return EXIT_SUCCESS;
}

////////////////////////////////////////////
// Helper functions.
////////////////////////////////////////////
void print_usage(char ** const argv) {
  cerr << "USAGE: " << argv[0] << " [OPTIONS]... FILE" << endl;
  cerr << "Renders the scene specified by the scene file FILE." << endl << endl;
  cerr << "Mandatory options: " << endl;
  cerr << "  -t\tRay tracing method to use. Valid values: " << endl;
  cerr << "    \twhitted     Classic Whitted ray tracing." << endl;
  cerr << "    \tmonte_carlo Monte Carlo path tracing." << endl;
  cerr << "    \tjensen      Photon mapping. " << endl << endl;
  cerr << "Extra options:" << endl;
  cerr << "  -o\tOutput image file name with extension." << endl;
  cerr << "    \tDefaults to \"output.png\"." << endl;
  cerr << "  -f\tField of view to use in degrees." << endl;
  cerr << "    \tDefaults to 45.0 degrees." << endl;
  cerr << "  -s\tNumber of samples per pixel." << endl;
  cerr << "    \tDefaults to 25 samples." << endl;
  cerr << "  -w\tImage size in pixels as \"WIDTHxHEIGHT\"." << endl;
  cerr << "    \tDefaults to 640x480 pixels." << endl;
  cerr << "  -r\tMaxmimum recursion depth." << endl;
  cerr << "    \tDefaults to 5." << endl;
}

void parse_args(int argc, char ** const argv) {
  int opt;
  int x_pos;

  // Check command line arguments.
  if(argc == 1) {
    print_usage(argv);
    exit(EXIT_FAILURE);
  }

  while((opt = getopt(argc, argv, ":t:s:w:f:o:r:")) != -1) {
    switch (opt) {
    case 't':
      if (strcmp("whitted", optarg) == 0 )
	g_tracer = WHITTED;
      else if(strcmp("monte_carlo", optarg) == 0 || strcmp("montecarlo", optarg) == 0)
	g_tracer = MONTE_CARLO;
      else if(strcmp("jensen", optarg) == 0)
	g_tracer = JENSEN;
      else {
	cerr << "Invalid ray tracer: " << optarg << endl;
	print_usage(argv);
	exit(EXIT_FAILURE);
      }

      break;

    case 'w':
      for (x_pos = 0; optarg[x_pos]; x_pos++)
	if (optarg[x_pos] == 'x')
	  break;

      if (optarg[x_pos] == '\0') {
	cerr << "Invalid screen resolution: " << optarg << endl;
	print_usage(argv);
	exit(EXIT_FAILURE);
      } else {
	optarg[x_pos] = '\0';
	g_w = atoi(optarg);
	g_h = atoi(&optarg[x_pos + 1]);
	if (g_w <= 0 || g_h <= 0) {
	  cerr << "Invalid screen resolution: " << optarg << endl;
	  print_usage(argv);
	  exit(EXIT_FAILURE);
	}
      }

      break;

    case 's':
      g_samples = atoi(optarg);
      if (g_samples <= 0) {
	cerr << "Samples per pixel must be a positive integer." << endl;
	print_usage(argv);
	exit(EXIT_FAILURE);
      }

      break;

    case 'o':
      g_out_file_name = (char*)malloc((strlen(optarg) + 1) * sizeof(char));
      strcpy(g_out_file_name, optarg);

      break;

    case 'f':
      g_fov = atof(optarg);
      if (g_fov < 1.0f) {
	cerr << "FoV must be greater than or equal to 1.0 degrees." << endl;
	print_usage(argv);
	exit(EXIT_FAILURE);
      }
      
      break;

    case 'r':
      g_max_depth = static_cast<unsigned int>(abs(atoi(optarg)));
      if (g_max_depth == 0) {
	cerr << "Recursion depth must be a positive integer." << endl;
	print_usage(argv);
	exit(EXIT_FAILURE);
      }

      break;
      
    case ':':
      cerr << "Option \"-" << static_cast<char>(optopt) << "\" requires an argument." << endl;
      print_usage(argv);
      exit(EXIT_FAILURE);
      
      break;

    case '?':
    default:
      cerr << "Unrecognized option: \"-" << static_cast<char>(optopt) << "\"." << endl;
    }
  }
    
  g_input_file = argv[optind];
}

void scene_1(vector<Figure *> & vf, vector<Light *> & vl, mat4x4 & i_model_view) {
  Sphere * s;
  Plane * p;
  Disk * d;
  DirectionalLight * l;

  s = new Sphere(1.0f, 1.0f, -2.0f, 0.5f);
  s->m_mat->m_diffuse = vec3(1.0f, 0.0f, 0.0f);
  vf.push_back(static_cast<Figure *>(s));

  s = new Sphere(-1.0f, 1.0f, -2.0f, 0.5f);
  s->m_mat->m_diffuse = vec3(0.0f, 1.0f, 0.0f);
  vf.push_back(static_cast<Figure *>(s));

  s = new Sphere(1.0f, -1.0f, -2.0f, 0.5f);
  s->m_mat->m_diffuse = vec3(0.0f, 0.0f, 1.0f);
  vf.push_back(static_cast<Figure *>(s));

  s = new Sphere(-1.0f, -1.0f, -2.0f, 0.5f);
  s->m_mat->m_diffuse = vec3(1.0f, 0.0f, 1.0f);
  vf.push_back(static_cast<Figure *>(s));

  s = new Sphere(0.0f, 0.0f, -2.0f, 1.0f);
  s->m_mat->m_diffuse = vec3(1.0f, 1.0f, 0.0f);
  vf.push_back(static_cast<Figure *>(s));

  p = new Plane(vec3(0.0f, -1.5f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
  p->m_mat->m_diffuse = vec3(1.0f, 0.5f, 0.4f);
  vf.push_back(static_cast<Figure *>(p));

  s = new Sphere(-1.5f, 0.0f, -2.0f, 0.5f);
  s->m_mat->m_diffuse = vec3(1.0f, 1.0f, 1.0f);
  s->m_mat->m_rho = 0.3f;
  vf.push_back(static_cast<Figure *>(s));

  s = new Sphere(1.5f, 0.0f, -2.0f, 0.5f);
  s->m_mat->m_diffuse = vec3(1.0f, 1.0f, 1.0f);
  s->m_mat->m_rho = 0.08f;
  s->m_mat->m_refract = true;
  s->m_mat->m_ref_index = 1.1f;
  vf.push_back(static_cast<Figure *>(s));

  s = new Sphere(0.0f, 1.5f, -2.0f, 0.5f);
  s->m_mat->m_diffuse = vec3(1.0f, 1.0f, 1.0f);
  s->m_mat->m_rho = 0.5f;
  vf.push_back(static_cast<Figure *>(s));

  s = new Sphere(0.0f, 0.0f, -1.0f, 0.25f);
  s->m_mat->m_diffuse = vec3(1.0f, 1.0f, 1.0f);
  s->m_mat->m_rho = 0.1f;
  vf.push_back(static_cast<Figure *>(s));

  d = new Disk(vec3(-0.0f, -0.0f, -0.5f), vec3(0.0f, 0.0f, 0.1f), 0.25f);
  d->m_mat->m_diffuse = vec3(1.0f, 0.0f, 0.0f);
  d->m_mat->m_rho = 0.3f;
  d->m_mat->m_refract = true;
  d->m_mat->m_ref_index = 1.33f;
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

void scene_2(vector<Figure *> & vf, vector<Light *> & vl, mat4x4 & i_model_view) {
  Sphere * s;
  Plane * p;
  Disk * d;
  PointLight * l;
 
  s = new Sphere(0.2f, 0.0f, -0.75f, 0.25f);
  s->m_mat->m_diffuse = vec3(1.0f);
  s->m_mat->m_rho = 0.2f;
  vf.push_back(static_cast<Figure *>(s));

  p = new Plane(vec3(0.0f, -1.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
  p->m_mat->m_diffuse = vec3(0.0f, 1.0f, 0.0f);
  vf.push_back(static_cast<Figure *>(p));

  p = new Plane(vec3(-2.0f, 0.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f));
  p->m_mat->m_diffuse = vec3(1.0f, 0.0f, 0.0f);
  vf.push_back(static_cast<Figure *>(p));

  p = new Plane(vec3(2.0f, 0.0f, 0.0f), vec3(-1.0f, 0.0f, 0.0f));
  p->m_mat->m_diffuse = vec3(0.0f, 0.0f, 1.0f);
  vf.push_back(static_cast<Figure *>(p));

  p = new Plane(vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f));
  p->m_mat->m_diffuse = vec3(0.0f, 1.0f, 1.0f);
  vf.push_back(static_cast<Figure *>(p));

  p = new Plane(vec3(0.0f, 0.0f, -2.0f), vec3(0.0f, 0.0f, 1.0f));
  p->m_mat->m_diffuse = vec3(1.0f, 0.0f, 1.0f);
  vf.push_back(static_cast<Figure *>(p));

  p = new Plane(vec3(0.0f, 0.0f, 1.1f), vec3(0.0f, 0.0f, -1.0f));
  p->m_mat->m_diffuse = vec3(1.0f, 1.0f, 0.0f);
  vf.push_back(static_cast<Figure *>(p));

  s = new Sphere(-0.5f, -0.5f, -1.5f, 0.5f);
  s->m_mat->m_diffuse = vec3(0.0f);
  s->m_mat->m_rho = 1.0f;
  vf.push_back(static_cast<Figure *>(s));

  s = new Sphere(-0.5f, -0.5f, 0.6f, 0.5f);
  s->m_mat->m_diffuse = vec3(1.0f, 1.0f, 0.0f);
  s->m_mat->m_refract = true;
  s->m_mat->m_ref_index = 1.33f;
  vf.push_back(static_cast<Figure *>(s));

  d = new Disk(vec3(-0.25f, 1.0f, -1.0f), vec3(1.0f, 0.0f, 0.0f), 0.25f);
  d->m_mat->m_diffuse = vec3(1.0f);
  vf.push_back(static_cast<Figure *>(d));

  d = new Disk(vec3(0.25f, 1.0f, -1.0f), vec3(-1.0f, 0.0f, 0.0f), 0.25f);
  d->m_mat->m_diffuse = vec3(1.0f);
  vf.push_back(static_cast<Figure *>(d));

  d = new Disk(vec3(0.0f, 1.0f, -1.25f), vec3(0.0f, 0.0f, 1.0f), 0.25f);
  d->m_mat->m_diffuse = vec3(1.0f);
  vf.push_back(static_cast<Figure *>(d));

  d = new Disk(vec3(0.0f, 1.0f, -0.75f), vec3(0.0f, 0.0f, -1.0f), 0.25f);
  d->m_mat->m_diffuse = vec3(1.0f);
  vf.push_back(static_cast<Figure *>(d));

  l = new PointLight();
  l->m_position = vec3(0.0f, 0.9f, -1.0f);
  l->m_diffuse = vec3(1.0f);
  vl.push_back(static_cast<Light *>(l));
}

void scene_3(vector<Figure *> & vf, vector<Light *> & vl, mat4x4 & i_model_view) {
  Sphere * s;
  Plane * p;
  DirectionalLight * l;
  vec3 eye = vec3(0.0f, 1.5f, 0.0f);
  vec3 center = vec3(0.0f, 0.0f, -2.0f);
  vec3 left = vec3(-1.0f, 0.0f, 0.0f);
  vec3 up = cross(center - eye, left);

  s = new Sphere(0.0f, -0.15f, -2.0f, 1.0f);
  s->m_mat->m_diffuse = vec3(1.0f, 0.5f, 0.0f);
  s->m_mat->m_specular = vec3(0.3f);
  s->m_mat->m_shininess = 5.0f;
  s->m_mat->m_rho = 0.4f;
  s->m_mat->m_refract = true;
  s->m_mat->m_ref_index = 1.33f;
  vf.push_back(static_cast<Figure *>(s));

  s = new Sphere(0.0f, -0.15f, -2.0f, 0.5f);
  s->m_mat->m_diffuse = vec3(0.0f);
  s->m_mat->m_specular = vec3(0.0f);
  s->m_mat->m_rho = 0.0f;
  s->m_mat->m_refract = true;
  s->m_mat->m_ref_index = 2.6f;
  vf.push_back(static_cast<Figure *>(s));

  s = new Sphere(2.0f, 0.0f, -2.0f, 1.0f);
  s->m_mat->m_diffuse = vec3(1.0f, 0.0f, 1.0f);
  s->m_mat->m_rho = 1.0f;
  vf.push_back(static_cast<Figure *>(s));

  s = new Sphere(-1.0f, 0.25f, -3.25f, 1.0f);
  s->m_mat->m_diffuse = vec3(1.0f, 1.0f, 0.0f);
  s->m_mat->m_shininess = 20.0f;
  vf.push_back(static_cast<Figure *>(s));

  p = new Plane(vec3(0.0f, -1.5f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
  p->m_mat->m_diffuse = vec3(1.0f);
  p->m_mat->m_specular = vec3(0.0f);
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

void scene_4(vector<Figure *> & vf, vector<Light *> & vl, mat4x4 & i_model_view) {
  Sphere * s;
  Plane * p;

  s = new Sphere(0.0f, 0.0f, -2.0f, 1.0f);
  s->m_mat->m_diffuse = vec3(1.0f, 1.0f, 0.0f);
  vf.push_back(static_cast<Figure *>(s));

  p = new Plane(vec3(0.0f, -1.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
  p->m_mat->m_diffuse = vec3(1.0f);
  p->m_mat->m_specular = vec3(0.0f);
  vf.push_back(static_cast<Figure *>(p));
}

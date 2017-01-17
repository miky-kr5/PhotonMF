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

#include "camera.hpp"
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
#include "hsa_brdf.hpp"
#include "environment.hpp"

using namespace std;
using namespace glm;

////////////////////////////////////////////
// Defines.
////////////////////////////////////////////
#define ANSI_BOLD_YELLOW "\x1b[1;33m"
#define ANSI_RESET_STYLE "\x1b[m"

////////////////////////////////////////////
// Function prototypes.
////////////////////////////////////////////
static void scene_1(vector<Figure *> & vf, vector<Light *> & vl, Environment * & e, Camera * c);
static void scene_2(vector<Figure *> & vf, vector<Light *> & vl, Environment * & e, Camera * c);
static void scene_3(vector<Figure *> & vf, vector<Light *> & vl, Environment * & e, Camera * c);
static void scene_4(vector<Figure *> & vf, vector<Light *> & vl, Environment * & e, Camera * c);
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

static char * g_input_file = NULL;
static char * g_out_file_name = NULL;
static int g_samples = 25;
static float g_fov = 45.f;
static int g_w = 640;
static int g_h = 480;
static vec3 ** image;
static tracer_t g_tracer = NONE;
static unsigned int g_max_depth = 5;
static float g_gamma = 2.2f;
static float g_exposure = 0.0f;

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
  FIBITMAP * input_bitmap;
  FIBITMAP * output_bitmap;
  FREE_IMAGE_FORMAT fif;
  BYTE * bits;
  FIRGBF *pixel;
  int pitch;
  Camera  * cam;
  Environment * env = NULL;

  parse_args(argc, argv);
  
  // Initialize everything.
  FreeImage_Initialise();

  cam = new Camera(g_h, g_w, g_fov);
  
  image = new vec3*[g_h];
  for (int i = 0; i < g_h; i++) {
    image[i] = new vec3[g_w];
  }
  
  scene_3(figures, lights, env, cam);

  // Create the tracer object.
  cout << "Rendering the input file: " << ANSI_BOLD_YELLOW << g_input_file << ANSI_RESET_STYLE << endl;
  cout << "The scene contains: " << endl;
  cout << "  " << ANSI_BOLD_YELLOW << figures.size() << ANSI_RESET_STYLE << (figures.size() != 1 ? " figures." : " figure.") << endl;
  cout << "  " << ANSI_BOLD_YELLOW << lights.size() << ANSI_RESET_STYLE << " light "  << (lights.size() != 1 ? "sources." : "source.") << endl;
  cout << "Output image resolution is " << ANSI_BOLD_YELLOW << g_w << "x" << g_h << ANSI_RESET_STYLE << " pixels." << endl;
  cout << "Using " << ANSI_BOLD_YELLOW << g_samples << ANSI_RESET_STYLE << " samples per pixel." << endl;
  cout << "Maximum ray tree depth is " << ANSI_BOLD_YELLOW << g_max_depth << ANSI_RESET_STYLE << "." << endl;

  if (g_tracer == WHITTED) {
    cout << "Using " << ANSI_BOLD_YELLOW << "Whitted" << ANSI_RESET_STYLE << " ray tracing." << endl;
    tracer = static_cast<Tracer *>(new WhittedTracer(g_max_depth));
  } else if(g_tracer == MONTE_CARLO) {
    cout << "Using " << ANSI_BOLD_YELLOW << "Monte Carlo" << ANSI_RESET_STYLE << " path tracing." << endl;
    tracer = static_cast<Tracer *>(new PathTracer(g_max_depth));
  } else if(g_tracer == JENSEN) {
    cerr << "Photon mapping coming soon." << endl;
    return EXIT_FAILURE;
  } else {
    cerr << "Must specify a ray tracer with \"-t\"." << endl;
    print_usage(argv);
    return EXIT_FAILURE;
  }
  
  // Generate the image.
  total = g_h * g_w * g_samples;
  cout << "Tracing a total of " << ANSI_BOLD_YELLOW << total << ANSI_RESET_STYLE << " primary rays:" << endl;
#pragma omp parallel for schedule(dynamic, 1) private(r, sample) shared(current)
  for (int i = 0; i < g_h; i++) {
    for (int j = 0; j < g_w; j++) {
      for (int k = 0; k < g_samples; k++) {
	sample = cam->sample_pixel(i, j);
	r = Ray(normalize(vec3(sample, -0.5f) - vec3(0.0f)), vec3(0.0f));
	cam->view_to_world(r);
	image[i][j] += tracer->trace_ray(r, figures, lights, env, 0);
#pragma omp atomic
	current++;
      }
      image[i][j] /= g_samples;
    }
#pragma omp critical
    cout << "\r" << setw(3) << static_cast<size_t>((static_cast<double>(current) / static_cast<double>(total)) * 100.0) << "% done.";
  }
  cout << endl;

  // Copy the pixels to the output bitmap.
  cout << "Saving output image." << endl;
  input_bitmap = FreeImage_AllocateT(FIT_RGBF, g_w, g_h, 96);
  pitch = FreeImage_GetPitch(input_bitmap);
  bits = (BYTE*)FreeImage_GetBits(input_bitmap);
  for (unsigned int y = 0; y < FreeImage_GetHeight(input_bitmap); y++) {
    pixel = (FIRGBF*)bits;
    for (unsigned int x = 0; x < FreeImage_GetWidth(input_bitmap); x++) {
      pixel[x].red = image[g_h - 1 - y][x].r;
      pixel[x].green = image[g_h - 1 - y][x].g;
      pixel[x].blue = image[g_h - 1 - y][x].b;
    }
    bits += pitch;
  }

  output_bitmap = FreeImage_ToneMapping(input_bitmap, FITMO_DRAGO03, g_gamma, g_exposure);
  
  // Save the output image.
  fif = FreeImage_GetFIFFromFilename(g_out_file_name != NULL ? g_out_file_name : OUT_FILE);
  FreeImage_Save(fif, output_bitmap, g_out_file_name != NULL ? g_out_file_name : OUT_FILE);
  FreeImage_Unload(input_bitmap);
  FreeImage_Unload(output_bitmap);

  // Clean up.
  if (g_out_file_name != NULL)
    free(g_out_file_name);

  delete cam;
  delete tracer;
  if (env != NULL)
    delete env;

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
  cerr << "  -g\tGamma correction value (>= 0)." << endl;
  cerr << "    \tDefaults to 2.2" << endl;
  cerr << "  -e\tExposure scale factor (in [-8, 8])." << endl;
  cerr << "    \tDefaults to 0.0 (no correction)." << endl;
}

void parse_args(int argc, char ** const argv) {
  int opt;
  int x_pos;

  // Check command line arguments.
  if(argc == 1) {
    print_usage(argv);
    exit(EXIT_FAILURE);
  }

  while((opt = getopt(argc, argv, "-:t:s:w:f:o:r:g:e:")) != -1) {
    switch (opt) {
    case 1:
      g_input_file = (char *)malloc((strlen(optarg) + 1) * sizeof(char));
      strcpy(g_input_file, optarg);
      break;

    case 'g':
      g_gamma = atof(optarg);
      g_gamma = g_gamma < 0.0f ? 0.0f : g_gamma;
      break;

    case 'e':
      g_exposure = atof(optarg);
      g_exposure = clamp(g_exposure, -8.0f, 8.0f);
      break;
      
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

  if (g_input_file == NULL) {
    cerr << "Must specify an input file." << endl;
    print_usage(argv);
    exit(EXIT_FAILURE);
  }
}

void scene_1(vector<Figure *> & vf, vector<Light *> & vl, Environment * & e, Camera * c) {
  Sphere * s;
  Plane * p;
  Disk * d;
  DirectionalLight * l;

  e = new Environment(NULL, false, vec3(0.7f, 0.4f, 0.05f));
  
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

void scene_2(vector<Figure *> & vf, vector<Light *> & vl, Environment * & e, Camera * c) {
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
  p->m_mat->m_specular = vec3(0.0f);
  vf.push_back(static_cast<Figure *>(p));

  p = new Plane(vec3(-2.0f, 0.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f));
  p->m_mat->m_diffuse = vec3(1.0f, 0.0f, 0.0f);
  p->m_mat->m_specular = vec3(0.0f);
  vf.push_back(static_cast<Figure *>(p));

  p = new Plane(vec3(2.0f, 0.0f, 0.0f), vec3(-1.0f, 0.0f, 0.0f));
  p->m_mat->m_diffuse = vec3(0.0f, 0.0f, 1.0f);
  p->m_mat->m_specular = vec3(0.0f);
  vf.push_back(static_cast<Figure *>(p));

  p = new Plane(vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f));
  p->m_mat->m_diffuse = vec3(0.0f, 1.0f, 1.0f);
  p->m_mat->m_specular = vec3(0.0f);
  vf.push_back(static_cast<Figure *>(p));

  p = new Plane(vec3(0.0f, 0.0f, -2.0f), vec3(0.0f, 0.0f, 1.0f));
  p->m_mat->m_diffuse = vec3(1.0f, 0.0f, 1.0f);
  p->m_mat->m_specular = vec3(0.0f);
  vf.push_back(static_cast<Figure *>(p));

  p = new Plane(vec3(0.0f, 0.0f, 1.1f), vec3(0.0f, 0.0f, -1.0f));
  p->m_mat->m_diffuse = vec3(1.0f, 1.0f, 0.0f);
  p->m_mat->m_specular = vec3(0.0f);
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

void scene_3(vector<Figure *> & vf, vector<Light *> & vl, Environment * & e, Camera * c) {
  Sphere * s;
  Disk * d;
  // SpotLight * l;
  // DirectionalLight * l2;
  vec3 eye = vec3(0.0f, 1.5f, 1.0f);
  vec3 center = vec3(0.0f, 0.0f, -2.0f);
  vec3 left = vec3(-1.0f, 0.0f, 0.0f);

  e = new Environment("textures/pisa.hdr");
  
  c->m_eye = eye;
  c->m_look = center;
  c->m_up = cross(normalize(center - eye), left);
  c->translate(vec3(1.0f, 0.0f, 0.0f));
  //c->roll(15.0f);

  // s = new Sphere(0.0f, -0.15f, -2.0f, 1.0f);
  // s->m_mat->m_diffuse = vec3(1.0f, 0.5f, 0.0f);
  // s->m_mat->m_specular = vec3(0.3f);
  // s->m_mat->m_shininess = 5.0f;
  // s->m_mat->m_rho = 0.4f;
  // s->m_mat->m_refract = true;
  // s->m_mat->m_ref_index = 1.33f;
  // vf.push_back(static_cast<Figure *>(s));

  // s = new Sphere(0.0f, -0.15f, -2.0f, 0.5f);
  // s->m_mat->m_diffuse = vec3(0.0f);
  // s->m_mat->m_specular = vec3(0.0f);
  // s->m_mat->m_rho = 0.0f;
  // s->m_mat->m_refract = true;
  // s->m_mat->m_ref_index = 2.6f;
  // vf.push_back(static_cast<Figure *>(s));

  s = new Sphere(2.0f, 0.0f, -2.0f, 1.5f, new HeidrichSeidelAnisotropicBRDF(vec3(0.0f, 1.0f, 0.0f)));
  s->m_mat->m_diffuse = vec3(1.0f, 1.0f, 0.0f);
  s->m_mat->m_shininess = 128.0f;
  vf.push_back(static_cast<Figure *>(s));

  s = new Sphere(-1.0f, 0.0f, -3.25f, 1.5f);
  s->m_mat->m_diffuse = vec3(1.0f, 0.0f, 1.0f);
  s->m_mat->m_rho = 0.4f;
  vf.push_back(static_cast<Figure *>(s));

  s = new Sphere(1.0f, 0.0f, -3.25f, 1.5f);
  s->m_mat->m_diffuse = vec3(1.0f);
  s->m_mat->m_rho = 0.4f;
  vf.push_back(static_cast<Figure *>(s));
  
  d = new Disk(vec3(1.0f, -1.5f, -3.25f), vec3(0.0f, 1.0f, 0.0f), 3.0f);
  d->m_mat->m_diffuse = vec3(0.0f, 0.5f, 0.5f);
  d->m_mat->m_specular = vec3(0.0f);
  vf.push_back(static_cast<Figure *>(d));
  
  // l = new SpotLight();
  // l->m_position = normalize(vec3(-2.0f, 1.5f, -1.0f));
  // l->m_diffuse = vec3(1.0f, 1.0f, 0.0f);
  // l->m_spot_dir = normalize(vec3(0.5f, 0.0f, -2.5f) - vec3(-2.0f, 1.5f, -1.0f));
  // l->m_spot_cutoff = 89.0f;
  // l->m_spot_exponent = 10.0f;
  // vl.push_back(static_cast<Light *>(l));

  // l2 = new DirectionalLight();
  // l2->m_position = normalize(vec3(-1.0f, 0.7f, 1.0f));
  // l2->m_diffuse = vec3(1.0f, 1.0f, 1.0f);
  // vl.push_back(static_cast<Light *>(l2));

  // l2 = new DirectionalLight();
  // l2->m_position = normalize(vec3(-0.5f, 0.7f, 1.0f));
  // l2->m_diffuse = vec3(0.0f, 0.0f, 1.0f);
  // l2->m_specular = vec3(0.0f, 0.0f, 1.0f);
  // vl.push_back(static_cast<Light *>(l2));

  // l = new DirectionalLight();
  // l->m_position = normalize(vec3(1.0f, 0.0f, 1.0f));
  // l->m_diffuse = vec3(0.5f);
  // vl.push_back(static_cast<Light *>(l));
}

void scene_4(vector<Figure *> & vf, vector<Light *> & vl, Environment * & e, Camera * c) {
  Sphere * s;
  Plane * p;

  e = new Environment("textures/pisa.hdr");
  
  s = new Sphere(0.0f, 0.0f, -2.0f, 1.0f);
  s->m_mat->m_diffuse = vec3(1.0f);
  s->m_mat->m_rho = 0.3f;
  vf.push_back(static_cast<Figure *>(s));

  p = new Plane(vec3(0.0f, -1.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
  p->m_mat->m_diffuse = vec3(1.0f);
  p->m_mat->m_specular = vec3(0.0f);
  vf.push_back(static_cast<Figure *>(p));
}

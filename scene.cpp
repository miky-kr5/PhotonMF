#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <cstdlib>

#include <glm/glm.hpp>
#include <json_spirit_reader.h>
#include <json_spirit_value.h>

#include "scene.hpp"

using std::cerr;
using std::endl;
using std::string;
using std::ostringstream;
using std::ifstream;
using std::ios;
using std::streamsize;
using glm::vec3;
using glm::normalize;
using glm::cross;
using json_spirit::read;
using json_spirit::Value;
using json_spirit::Error_position;
using json_spirit::Object;
using json_spirit::Array;

static const string ENV_KEY = "environment";
static const string CAM_KEY = "camera";
static const string SPH_KEY = "sphere";
static const string PLN_KEY = "plane";
static const string DSK_KEY = "disk";
static const string MSH_KEY = "mesh";
static const string DLT_KEY = "directional_light";
static const string PLT_KEY = "point_light";
static const string SLT_KEY = "spot_light";
static const string SAL_KEY = "sphere_area_light";
static const string PAL_KEY = "planar_area_light";

static const string ENV_TEX_KEY = "texture";
static const string ENV_LPB_KEY = "light_probe";
static const string ENV_COL_KEY = "color";

static const string CAM_EYE_KEY = "eye";
static const string CAM_CNT_KEY = "look";
static const string CAM_LFT_KEY = "left";
static const string CAM_UPV_KEY = "up";

static const string FIG_POS_KEY = "position";
static const string FIG_MAT_KEY = "material";
static const string FIG_RAD_KEY = "radius";
static const string FIG_NOR_KEY = "normal";

static const string PLN_PNT_KEY = "point";

static const string MLT_EMS_KEY = "emission";
static const string MLT_DIF_KEY = "diffuse";
static const string MLT_SPC_KEY = "specular";
static const string MLT_RHO_KEY = "rho";
static const string MLT_SHN_KEY = "shininess";
static const string MLT_RFI_KEY = "ref_index";
static const string MLT_BRF_KEY = "transmissive";
static const string MLT_BRD_KEY = "brdf";

static void read_vector(Value & val, vec3 & vec) throw(SceneError);
static void read_environment(Value & v, Environment * & e) throw(SceneError);
static void read_camera(Value & v, Camera * & c) throw(SceneError);

Scene::Scene(const char * file_name, int h, int w, float fov) throw(SceneError) {
  ostringstream oss;
  ifstream ifs(file_name, ios::in);
  Value val;
  Object top_level;

  m_cam = NULL;
  m_env = NULL;
  
  if (ifs.is_open()) {
    try {
      read_or_throw(ifs, val);
    } catch (Error_position & e) {
      ifs.close();
      oss << "Failed to parse the input file: " << endl << "Reason: " << e.reason_ << endl << "Line: " << e.line_ << endl << "Column: " << e.column_;
      throw SceneError(oss.str());
    }

    ifs.close();

    top_level = val.get_value<Object>();

    try {
      for(Object::iterator it = top_level.begin(); it != top_level.end(); it++) {
	if ((*it).name_ == ENV_KEY)
	  read_environment((*it).value_, m_env);

	else if ((*it).name_ == CAM_KEY)
	  read_camera((*it).value_, m_cam);

	else if ((*it).name_ == SPH_KEY) {

	} else if ((*it).name_ == PLN_KEY) {

	} else if ((*it).name_ == DSK_KEY) {

	} else if ((*it).name_ == DLT_KEY) {

	} else if ((*it).name_ == PLT_KEY) {

	} else if ((*it).name_ == SLT_KEY) {

	} else
	  cerr << "Unrecognized key \"" << (*it).name_ << "\" in the input file." << endl;
      }
    } catch (SceneError & e) {
      throw e;
    }
    
    // If there were no camera and/or environment defined, create some default ones.
    if (m_cam == NULL)
      m_cam = new Camera();
    if (m_env == NULL)
      m_env = new Environment();
    
  } else
    throw SceneError("Could not open the input file.");
}

Scene::~Scene() {
  delete m_env;
  delete m_cam;

  for (size_t i = 0; i < m_figures.size(); i++) {
    delete m_figures[i];
  }
  m_figures.clear();

  for (size_t i = 0; i < m_lights.size(); i++) {
    delete m_lights[i];
  }
  m_lights.clear();
}

inline void read_vector(Value & val, vec3 & vec) throw(SceneError) {
  Array a = val.get_value<Array>();

  if (a.size() < 3)
    throw SceneError("Vector value must have 3 elements.");

  vec = vec3(a[0].get_value<double>(), a[1].get_value<double>(), a[2].get_value<double>());
}

void read_environment(Value & v, Environment * & e) throw(SceneError) {
  string t_name = "";
  bool l_probe = false, has_tex = false, has_color = false;
  vec3 color;
  Object env_obj = v.get_value<Object>();

  for(Object::iterator it = env_obj.begin(); it != env_obj.end(); it++) {
    if ((*it).name_ == ENV_TEX_KEY)
      t_name = (*it).value_.get_value<string>();

    else if ((*it).name_ == ENV_LPB_KEY)
      l_probe = (*it).value_.get_value<bool>();

    else if ((*it).name_ == ENV_COL_KEY)
      try {
	read_vector((*it).value_, color);
      } catch (SceneError & e) {
	throw e;
      }

    else
      cerr << "Unrecognized key \"" << (*it).name_ << "\" in input file." << endl;
  }

  if (!has_tex && !has_color)
    throw SceneError("Environment must specify either a texture or color.");

  e = new Environment(has_tex ? t_name.c_str() : NULL , l_probe, color);
}

void read_camera(Value & v, Camera * & c) throw(SceneError) {
  bool has_up = false, has_left = false, has_eye = false, has_look = false;
  vec3 eye, look, left, up;
  Object cam_obj = v.get_value<Object>();

  for(Object::iterator it = cam_obj.begin(); it != cam_obj.end(); it++) {
    if ((*it).name_ == CAM_EYE_KEY) {
      read_vector((*it).value_, eye);
      has_eye = true;

    } else if ((*it).name_ == CAM_CNT_KEY) {
      read_vector((*it).value_, look);
      has_look = true;

    } else if ((*it).name_ == CAM_LFT_KEY) {
      read_vector((*it).value_, left);
      has_left = true;

    } else if ((*it).name_ == CAM_UPV_KEY) {
      read_vector((*it).value_, up);
      has_up = true;

    } else
      cerr << "Unrecognized key \"" << (*it).name_ << "\" in input file." << endl;
  }

  if (!has_eye)
    throw SceneError("Must specify an eye position for the camera.");

  if (!has_look)
    throw SceneError("Must specify a look position for the camera.");
  
  if (has_up)
    c = new Camera(eye, look, up);
  else if(!has_up && has_left) {
    up = cross(normalize(look - eye), left);
    c = new Camera(eye, look, up);
  } else
    throw SceneError("Must specify either an up or left vector for the camera.");
}

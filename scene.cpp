#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <cassert>

#include <glm/glm.hpp>
#include <json_spirit_reader.h>

#include "scene.hpp"
#include "brdf.hpp"
#include "phong_brdf.hpp"
#include "hsa_brdf.hpp"
#include "sphere.hpp"
#include "plane.hpp"
#include "disk.hpp"
#include "directional_light.hpp"
#include "point_light.hpp"
#include "spot_light.hpp"

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
static const string CAM_RLL_KEY = "roll";
static const string CAM_PTC_KEY = "pitch";
static const string CAM_YAW_KEY = "yaw";

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

static const string BRD_PHN_KEY = "phong";
static const string BRD_HSA_KEY = "heidrich-seidel";

static const string GEO_TRN_KEY = "translation";
static const string GEO_SCL_KEY = "scaling";
static const string GEO_ROT_KEY = "rotation";

static const string LGT_DIR_KEY = "direction";
static const string LGT_CAT_KEY = "const_attenuation";
static const string LGT_LAT_KEY = "linear_attenuation";
static const string LGT_QAT_KEY = "quad_attenuation";
static const string LGT_SPD_KEY = "spot_direction";
static const string LGT_SPC_KEY = "spot_cutoff";
static const string LGT_SPE_KEY = "spot_exponent";

Scene::Scene(const char * file_name, int h, int w, float fov) {
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
      for (Object::iterator it = top_level.begin(); it != top_level.end(); it++) {
	if ((*it).name_ == ENV_KEY)
	  read_environment((*it).value_);

	else if ((*it).name_ == CAM_KEY)
	  read_camera((*it).value_);

	else if ((*it).name_ == SPH_KEY)
	  m_figures.push_back(read_sphere((*it).value_));

	else if ((*it).name_ == PLN_KEY)
	  m_figures.push_back(read_plane((*it).value_));

	else if ((*it).name_ == DSK_KEY)
	  m_figures.push_back(read_disk((*it).value_));

	else if ((*it).name_ == DLT_KEY)
	  m_lights.push_back(read_light((*it).value_, DIRECTIONAL));

	else if ((*it).name_ == PLT_KEY)
	  m_lights.push_back(read_light((*it).value_, POINT));

	else if ((*it).name_ == SLT_KEY)
	  m_lights.push_back(read_light((*it).value_, SPOT));

	else
	  cerr << "Unrecognized key \"" << (*it).name_ << "\" in the input file." << endl;
      }
    } catch (SceneError & e) {
      throw e;
    } catch (runtime_error & r) {
      throw SceneError("Type error in input file.");
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

void Scene::read_vector(Value & val, vec3 & vec) {
  Array a = val.get_value<Array>();

  if (a.size() < 3)
    throw SceneError("Vector value must have 3 elements.");

  vec = vec3(a[0].get_value<double>(), a[1].get_value<double>(), a[2].get_value<double>());
}

void Scene::read_environment(Value & v) {
  string t_name = "";
  bool l_probe = false, has_tex = false, has_color = false;
  vec3 color = vec3(1.0f);
  Object env_obj = v.get_value<Object>();

  for (Object::iterator it = env_obj.begin(); it != env_obj.end(); it++) {
    if ((*it).name_ == ENV_TEX_KEY) {
      t_name = (*it).value_.get_value<string>();
      has_tex = true;

    } else if ((*it).name_ == ENV_LPB_KEY)
      l_probe = (*it).value_.get_value<bool>();

    else if ((*it).name_ == ENV_COL_KEY) {
      try {
	read_vector((*it).value_, color);
      } catch (SceneError & e) {
	throw e;
      }
      has_color = true;
    }

    else
      cerr << "Unrecognized key \"" << (*it).name_ << "\" in environment." << endl;
  }

  if (!has_tex && !has_color)
    throw SceneError("Environment must specify either a texture or color.");
  
  m_env = new Environment(has_tex ? t_name.c_str() : NULL , l_probe, color);
}

void Scene::read_camera(Value & v) {
  bool has_up = false, has_left = false, has_eye = false, has_look = false;
  vec3 eye, look, left, up, translation;
  float pitch = 0.0f, yaw = 0.0f, roll = 0.0f;
  Object cam_obj = v.get_value<Object>();

  for (Object::iterator it = cam_obj.begin(); it != cam_obj.end(); it++) {
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

    } else if ((*it).name_ == GEO_TRN_KEY)
      read_vector((*it).value_, translation);

    else if ((*it).name_ == CAM_RLL_KEY)
      roll = static_cast<float>((*it).value_.get_value<double>());

    else if ((*it).name_ == CAM_PTC_KEY)
      pitch = static_cast<float>((*it).value_.get_value<double>());

    else if ((*it).name_ == CAM_YAW_KEY)
      yaw = static_cast<float>((*it).value_.get_value<double>());
	
    else
      cerr << "Unrecognized key \"" << (*it).name_ << "\" in camera." << endl;
  }

  if (!has_eye || !has_look)
    throw SceneError("Must specify an eye and look positions for the camera.");
  
  if (has_up)
    m_cam = new Camera(eye, look, up);
  else if(!has_up && has_left) {
    up = cross(normalize(look - eye), left);
    m_cam = new Camera(eye, look, up);
  } else
    throw SceneError("Must specify either an up or left vector for the camera.");

  m_cam->pitch(pitch);
  m_cam->yaw(yaw);
  m_cam->roll(roll);
  m_cam->translate(translation);
}

Material * Scene::read_material(Value & v) {
  vec3 emission = vec3(0.0f), diffuse = vec3(1.0f), specular = vec3(1.0f);
  bool transmissive = false;
  float rho = 0.0f, ref_index = 1.0f, shininess = 89.0f;
  Material * mat = NULL;
  Object mat_obj = v.get_value<Object>();

  for (Object::iterator it = mat_obj.begin(); it != mat_obj.end(); it++) {
    if ((*it).name_ == MLT_EMS_KEY) {
      read_vector((*it).value_, emission);

    } else if ((*it).name_ == MLT_DIF_KEY) {
      read_vector((*it).value_, diffuse);

    } else if ((*it).name_ == MLT_SPC_KEY) {
      read_vector((*it).value_, specular);

    } else if ((*it).name_ == MLT_RHO_KEY) {
      rho = static_cast<float>((*it).value_.get_value<double>());

    } else if ((*it).name_ == MLT_SHN_KEY) {
      shininess = static_cast<float>((*it).value_.get_value<double>());

    } else if ((*it).name_ == MLT_RFI_KEY) {
      ref_index = static_cast<float>((*it).value_.get_value<double>());

    } else if ((*it).name_ == MLT_BRF_KEY) {
      transmissive = (*it).value_.get_value<bool>();

    } else if ((*it).name_ == MLT_BRD_KEY) {
      if ((*it).value_.get_value<string>() == BRD_PHN_KEY)
	mat = new Material(new PhongBRDF());
      else if ((*it).value_.get_value<string>() == BRD_HSA_KEY)
	mat = new Material(new HeidrichSeidelAnisotropicBRDF(vec3(0.0f, 1.0f, 0.0f)));
      else
	throw SceneError("Unrecognized BRDF in material.");
 
    } else
      cerr << "Unrecognized key \"" << (*it).name_ << "\" in material." << endl;
  }

  if (mat == NULL)
    mat = new Material();
  mat->m_emission = emission;
  mat->m_diffuse = diffuse;
  mat->m_specular = specular;
  mat->m_rho = rho;
  mat->m_ref_index = ref_index;
  mat->m_shininess = shininess;
  mat->m_refract = transmissive;
  
  return mat;
}

Figure * Scene::read_sphere(Value &v) {
  bool has_position = false, has_radius = false;
  vec3 position;
  float radius = 1.0f;
  Material * mat = NULL;
  Object sph_obj = v.get_value<Object>();

  for (Object::iterator it = sph_obj.begin(); it != sph_obj.end(); it++) {
    if ((*it).name_ == FIG_POS_KEY) {
      read_vector((*it).value_, position);
      has_position = true;

    } else if ((*it).name_ == FIG_MAT_KEY) {
      try {
	mat = read_material((*it).value_);
      } catch (SceneError & e) {
	throw e;
      }

    } else if ((*it).name_ == FIG_RAD_KEY) {
      radius = static_cast<float>((*it).value_.get_value<double>());

      if (radius <= 0.0f)
	throw SceneError("Sphere radius must be greater than 0.");

      has_radius = true;

    } else
      cerr << "Unrecognized key \"" << (*it).name_ << "\" in sphere." << endl;
  }

  if (!has_position || !has_radius)
    throw SceneError("Sphere must specify a position and radius.");
  
  return static_cast<Figure *>(new Sphere(position, radius, mat));
}

Figure * Scene::read_plane(Value &v) {
  bool has_position = false, has_normal = false;
  vec3 position, normal = vec3(0.0f, 1.0f, 0.0f);
  Material * mat = NULL;
  Object pln_obj = v.get_value<Object>();

  for (Object::iterator it = pln_obj.begin(); it != pln_obj.end(); it++) {
    if ((*it).name_ == FIG_POS_KEY || (*it).name_ == PLN_PNT_KEY) {
      read_vector((*it).value_, position);
      has_position = true;

    } else if ((*it).name_ == FIG_MAT_KEY) {
      try {
	mat = read_material((*it).value_);
      } catch (SceneError & e) {
	throw e;
      }

    } else if ((*it).name_ == FIG_NOR_KEY) {
      read_vector((*it).value_, normal);
      has_normal = true;

    } else
      cerr << "Unrecognized key \"" << (*it).name_ << "\" in plane." << endl;
  }

  if (!has_position || !has_normal)
    throw SceneError("Plane must specify a point and normal vector.");
  
  return static_cast<Figure *>(new Plane(position, normal, mat));
}

Figure * Scene::read_disk(Value &v) {
  bool has_position = false, has_normal = false, has_radius = false;
  vec3 position, normal = vec3(0.0f, 1.0f, 0.0f);
  float radius = 1.0f;
  Material * mat = NULL;
  Object dsk_obj = v.get_value<Object>();

  for (Object::iterator it = dsk_obj.begin(); it != dsk_obj.end(); it++) {
    if ((*it).name_ == FIG_POS_KEY || (*it).name_ == PLN_PNT_KEY) {
      read_vector((*it).value_, position);
      has_position = true;

    } else if ((*it).name_ == FIG_MAT_KEY) {
      try {
	mat = read_material((*it).value_);
      } catch (SceneError & e) {
	throw e;
      }

    } else if ((*it).name_ == FIG_NOR_KEY) {
      read_vector((*it).value_, normal);
      has_normal = true;

    } else if ((*it).name_ == FIG_RAD_KEY) {
      radius = static_cast<float>((*it).value_.get_value<double>());

      if (radius <= 0.0f)
	throw SceneError("Disk radius must be greater than 0.");

      has_radius = true;
      
    } else
      cerr << "Unrecognized key \"" << (*it).name_ << "\" in disk." << endl;
  }

  if (!has_position || !has_normal || !has_radius)
    throw SceneError("Disk must specify a point, a normal vector and a radius.");
  
  return static_cast<Figure *>(new Disk(position, normal, radius, mat));
}

Light * Scene::read_light(Value & v, light_type_t t) {
  vec3 position, diffuse = vec3(1.0f), specular = vec3(1.0f), spot_dir = vec3(0.0f, -1.0f, 0.0f);
  float const_att = 1.0f, lin_att = 0.0f, quad_att = 0.0f, spot_cutoff = 45.0f, spot_exp = 0.0f;
  Object lght_obj = v.get_value<Object>();

  for (Object::iterator it = lght_obj.begin(); it != lght_obj.end(); it++) {
    if ((*it).name_ == FIG_POS_KEY || (*it).name_ == LGT_DIR_KEY)
      read_vector((*it).value_, position);

    else if((*it).name_ == MLT_DIF_KEY)
      read_vector((*it).value_, diffuse);

    else if((*it).name_ == MLT_SPC_KEY)
      read_vector((*it).value_, specular);

    else if((*it).name_ == LGT_SPD_KEY)
      read_vector((*it).value_, spot_dir);

    else if ((*it).name_ == LGT_CAT_KEY)
      const_att = static_cast<float>((*it).value_.get_value<double>());

    else if ((*it).name_ == LGT_LAT_KEY)
      lin_att = static_cast<float>((*it).value_.get_value<double>());

    else if ((*it).name_ == LGT_QAT_KEY)
      quad_att = static_cast<float>((*it).value_.get_value<double>());

    else if ((*it).name_ == LGT_SPC_KEY)
      spot_cutoff = static_cast<float>((*it).value_.get_value<double>());

    else if ((*it).name_ == LGT_SPE_KEY)
      spot_exp = static_cast<float>((*it).value_.get_value<double>());
  }
  
  if (t == DIRECTIONAL)
    return static_cast<Light *>(new DirectionalLight(position, diffuse, specular));
  else if (t == POINT)
    return static_cast<Light *>(new PointLight(position, diffuse, specular, const_att, lin_att, quad_att));
  else if (t == SPOT)
    return static_cast<Light *>(new SpotLight(position, diffuse, specular, const_att, lin_att, quad_att, spot_cutoff, spot_exp, spot_dir));
  else
    throw SceneError("Unknown light type.");
  return NULL;
}

#pragma once
#ifndef SCENE_HPP
#define SCENE_HPP

#include <string>
#include <vector>
#include <stdexcept>

#include <json_spirit_value.h>

#include "camera.hpp"
#include "figure.hpp"
#include "light.hpp"
#include "material.hpp"
#include "environment.hpp"

using std::string;
using std::vector;
using std::runtime_error;
using json_spirit::Value;

typedef enum LIGHT_TYPE { DIRECTIONAL, POINT, SPOT } light_type_t;

class SceneError: public runtime_error {
public:
  explicit SceneError(const string & what_arg): runtime_error(what_arg) { }
};

class Scene {
public:
  vector<Figure *> m_figures;
  vector<Light *> m_lights;
  Environment * m_env;
  Camera * m_cam;

  Scene(const char * file_name, int h = 480, int w = 640, float fov = 90.0f) throw(SceneError);
  ~Scene();

private:
  void read_vector(Value & val, vec3 & vec) throw(SceneError);
  void read_environment(Value & v) throw(SceneError);
  void read_camera(Value & v) throw(SceneError);
  Material * read_material(Value & v) throw(SceneError);
  Figure * read_sphere(Value & v) throw(SceneError);
  Figure * read_plane(Value & v) throw(SceneError);
  Figure * read_disk(Value & v) throw(SceneError);
  Light * read_light(Value & v, light_type_t t) throw(SceneError);
};

#endif

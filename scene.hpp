#pragma once
#ifndef SCENE_HPP
#define SCENE_HPP

#include <string>
#include <vector>
#include <stdexcept>

#include "camera.hpp"
#include "figure.hpp"
#include "light.hpp"
#include "environment.hpp"

using std::string;
using std::vector;
using std::runtime_error;

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
};

#endif

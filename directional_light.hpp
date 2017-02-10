#pragma once
#ifndef DIRECTIONAL_LIGHT_HPP
#define DIRECTIONAL_LIGHT_HPP

#include <glm/glm.hpp>

#include "light.hpp"

using glm::normalize;

class DirectionalLight: public InfinitesimalLight {
public:
  DirectionalLight(): InfinitesimalLight() { }

  DirectionalLight(vec3 _p, vec3 _d, vec3 _s): InfinitesimalLight(normalize(_p), _d, _s) { }

  virtual vec3 direction(vec3 point) const;
  virtual float distance(vec3 point) const;
  virtual vec3 diffuse(vec3 normal, Ray & r, vec3 i_pos, Material & m) const;
  virtual vec3 specular(vec3 normal, Ray & r, vec3 i_pos, Material & m) const;
};

#endif

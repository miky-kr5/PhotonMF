#pragma once
#ifndef DIRECTIONAL_LIGHT_HPP
#define DIRECTIONAL_LIGHT_HPP

#include "light.hpp"

class DirectionalLight: public Light {
public:
  DirectionalLight(): Light() { }

  DirectionalLight(BRDF * _brdf, vec3 _p, vec3 _d, vec3 _s): Light(_brdf, _p, _d, _s) { }

  virtual vec3 direction(vec3 point);
  virtual float distance(vec3 point);
  virtual vec3 diffuse(vec3 normal, Ray & r, vec3 i_pos, Material & m) const;
  virtual vec3 specular(vec3 normal, Ray & r, vec3 i_pos, Material & m) const;
};

#endif

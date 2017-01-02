#pragma once
#ifndef DIRECTIONAL_LIGHT_HPP
#define DIRECTIONAL_LIGHT_HPP

#include "light.hpp"

class DirectionalLight: public Light {
public:
  DirectionalLight() {
    m_position = vec3(0.0f);
    m_diffuse = vec3(1.0f);
    m_specular = vec3(1.0f);
  }

  DirectionalLight(vec3 _p, vec3 _d, vec3 _s) {
    m_position = _p;
    m_diffuse = _d;
    m_specular = _s;
  }

  virtual vec3 shade(vec3 normal, Ray & r, Material & m) const;
};

#endif
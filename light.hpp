#pragma once
#ifndef LIGHT_HPP
#define LIGHT_HPP

#include <glm/vec3.hpp>

#include "brdf.hpp"
#include "phong_brdf.hpp"
#include "ray.hpp"
#include "material.hpp"

using glm::vec3;

class Light {
public:
  BRDF * m_brdf;
  vec3 m_position;
  vec3 m_diffuse;
  vec3 m_specular;

  Light(): m_brdf(static_cast<BRDF *>(new PhongBRDF())), m_position(vec3(0.0f)), m_diffuse(vec3(1.0f)), m_specular(vec3(1.0f)) { }

  Light(BRDF * _brdf, vec3 _p, vec3 _d, vec3 _s): m_brdf(_brdf), m_position(_p), m_diffuse(_d), m_specular(_s) { }
  
  virtual ~Light() {
    delete m_brdf;
  }

  virtual vec3 direction(vec3 point) = 0;
  virtual float distance(vec3 point) = 0;
  virtual vec3 diffuse(vec3 normal, Ray & r, vec3 i_pos, Material & m) const = 0;
  virtual vec3 specular(vec3 normal, Ray & r, vec3 i_pos, Material & m) const = 0;
};

#endif

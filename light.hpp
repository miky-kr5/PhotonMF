#pragma once
#ifndef LIGHT_HPP
#define LIGHT_HPP

#include <glm/glm.hpp>

#include "figure.hpp"
#include "material.hpp"
#include "ray.hpp"

using glm::vec3;
using glm::length;

class Light {
public:
  typedef enum LIGHT_TYPE { INFINITESIMAL = 0, AREA } ltype_t;

  vec3 m_position;
  vec3 m_diffuse;
  vec3 m_specular;

  Light(): m_position(vec3(0.0f)), m_diffuse(vec3(1.0f)), m_specular(vec3(1.0f)) { }

  Light(ltype_t _t): m_position(vec3(0.0f)), m_diffuse(vec3(1.0f)), m_specular(vec3(1.0f)), m_type(_t) { }

  Light(vec3 _p, vec3 _d, vec3 _s, ltype_t _t): m_position(_p), m_diffuse(_d), m_specular(_s), m_type(_t) { }
  
  virtual ~Light() { }

  virtual ltype_t light_type() {
    return m_type;
  }
  
  virtual vec3 direction(vec3 point) const = 0;
  virtual float distance(vec3 point) const = 0;
  virtual vec3 diffuse(vec3 normal, Ray & r, vec3 i_pos, Material & m) const = 0;
  virtual vec3 specular(vec3 normal, Ray & r, vec3 i_pos, Material & m) const = 0;

protected:
  ltype_t m_type;
};

class InfinitesimalLight: public Light {
public:
  InfinitesimalLight(): Light(INFINITESIMAL) { }
  
  InfinitesimalLight(vec3 _p, vec3 _d, vec3 _s): Light(_p, _d, _s, INFINITESIMAL) { }
  
  virtual vec3 direction(vec3 point) const = 0;
  virtual float distance(vec3 point) const = 0;
  virtual vec3 diffuse(vec3 normal, Ray & r, vec3 i_pos, Material & m) const = 0;
  virtual vec3 specular(vec3 normal, Ray & r, vec3 i_pos, Material & m) const = 0;
};

class AreaLight: public Light {
public:
  Figure * m_figure;

  AreaLight(): Light(AREA), m_figure(NULL), m_last_sample(vec3()), m_n_at_last_sample(vec3()) { }

  AreaLight(Figure * _f): Light(AREA), m_figure(_f), m_last_sample(vec3()), m_n_at_last_sample(vec3()) { }

  virtual vec3 direction(vec3 point) const {
    return m_last_sample - point;
  }

  virtual float distance(vec3 point) const {
    return length(m_last_sample - point);
  }

  virtual vec3 diffuse(vec3 normal, Ray & r, vec3 i_pos, Material & m) const = 0;
  virtual vec3 specular(vec3 normal, Ray & r, vec3 i_pos, Material & m) const = 0;
  virtual void sample_at_surface(vec3 point) = 0;

protected:
  vec3 m_last_sample;
  vec3 m_n_at_last_sample;
};

#endif

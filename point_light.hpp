#pragma once
#ifndef POINT_LIGHT_HPP
#define POINT_LIGHT_HPP

#include "light.hpp"

class PointLight: public Light {
public:
  float m_const_att;
  float m_lin_att;
  float m_quad_att;

  PointLight(): m_const_att(1.0f), m_lin_att(0.0f), m_quad_att(0.0f) {
    m_position = vec3(0.0f);
    m_diffuse = vec3(1.0f);
    m_specular = vec3(1.0f);
  }

  PointLight(vec3 _p, vec3 _d, vec3 _s, float _c, float _l, float _q): m_const_att(_c), m_lin_att(_l), m_quad_att(_q) {
    m_position = _p;
    m_diffuse = _d;
    m_specular = _s;
  }

  virtual vec3 direction(vec3 point);
  virtual float distance(vec3 point);
  virtual vec3 diffuse(vec3 normal, Ray & r, float t, Material & m) const;
  virtual vec3 specular(vec3 normal, Ray & r, float t, Material & m) const;
};

#endif

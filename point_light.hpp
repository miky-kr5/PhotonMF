#pragma once
#ifndef POINT_LIGHT_HPP
#define POINT_LIGHT_HPP

#include "light.hpp"

class PointLight: public InfinitesimalLight {
public:
  float m_const_att;
  float m_lin_att;
  float m_quad_att;

  PointLight(): InfinitesimalLight(), m_const_att(1.0f), m_lin_att(0.0f), m_quad_att(0.0f) { }

  PointLight(vec3 _p, vec3 _d, vec3 _s, float _c, float _l, float _q): InfinitesimalLight(_p, _d, _s), m_const_att(_c), m_lin_att(_l), m_quad_att(_q) { }

  virtual vec3 direction(vec3 point) const;
  virtual float distance(vec3 point) const;
  virtual vec3 diffuse(vec3 normal, Ray & r, vec3 i_pos, Material & m) const;
  virtual vec3 specular(vec3 normal, Ray & r, vec3 i_pos, Material & m) const;
};

#endif

#pragma once
#ifndef SPHERE_AREA_LIGHT_HPP
#define SPHERE_AREA_LIGHT_HPP

#include "light.hpp"
#include "sphere.hpp"

class SphereAreaLight: public AreaLight {
public:
  float m_const_att;
  float m_lin_att;
  float m_quad_att;

  SphereAreaLight(Sphere * _s, float _c = 1.0, float _l = 0.0, float _q = 0.0):
    AreaLight(static_cast<Figure *>(_s)),
    m_const_att(_c),
    m_lin_att(_l),
    m_quad_att(_q)
  { }

  virtual vec3 diffuse(vec3 normal, Ray & r, vec3 i_pos, Material & m) const;
  virtual vec3 specular(vec3 normal, Ray & r, vec3 i_pos, Material & m) const;
  virtual void sample_at_surface(vec3 point);
};

#endif

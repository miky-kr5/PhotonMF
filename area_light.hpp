#pragma once
#ifndef AREA_LIGHT_HPP
#define AREA_LIGHT_HPP

#include "light.hpp"

using glm::length;
using glm::normalize;
using glm::dot;

class AreaLight: public Light {
public:
  float m_const_att;
  float m_lin_att;
  float m_quad_att;
  Figure * m_figure;

  AreaLight():
    Light(AREA),
    m_const_att(1.0),
    m_lin_att(0.0),
    m_quad_att(0.0),
    m_figure(NULL),
    m_last_sample(vec3()),
    m_n_at_last_sample(vec3())
  { }

  AreaLight(Figure * _f, float _c = 1.0, float _l = 0.0, float _q = 0.0):
    Light(AREA),
    m_const_att(_c),
    m_lin_att(_l),
    m_quad_att(_q),
    m_figure(_f),
    m_last_sample(vec3()),
    m_n_at_last_sample(vec3())
  { }

  virtual vec3 direction(vec3 point) const {
    return normalize(m_last_sample - point);
  }

  virtual float distance(vec3 point) const {
    return length(m_last_sample - point);
  }

  vec3 diffuse(vec3 normal, Ray & r, vec3 i_pos, Material & m) const {
    float d, att, ln_dot_d, d2, g;
    vec3 l_dir, ref;

    l_dir = normalize(direction(i_pos));
    ln_dot_d = dot(m_n_at_last_sample, l_dir);
    if (ln_dot_d > 0.0f) {
      d2 = distance(i_pos);
      d2 *= d2;
      g = ln_dot_d / d2;
      d = distance(i_pos);
      att = 1.0f / (m_const_att + (m_lin_att * d) + (m_quad_att * (d * d)));
      return (att * m.m_brdf->diffuse(l_dir, normal, r, i_pos, m_figure->m_mat->m_emission) * g) / m_figure->pdf();

    } else
      return vec3(0.0f);
  }

  vec3 specular(vec3 normal, Ray & r, vec3 i_pos, Material & m) const {
    float d, att, ln_dot_d;
    vec3 l_dir, ref;

    l_dir = normalize(direction(i_pos));
    ln_dot_d = dot(m_n_at_last_sample, l_dir);
    if (ln_dot_d > 0.0f) {
      d = distance(i_pos);
      att = 1.0f / (m_const_att + (m_lin_att * d) + (m_quad_att * (d * d)));
      return (att * m.m_brdf->specular(l_dir, normal, r, i_pos, m_figure->m_mat->m_emission, m.m_shininess)) / m_figure->pdf();

    } else
      return vec3(0.0f);
  }

  virtual vec3 normal_at_last_sample() {
    return m_n_at_last_sample;
  }

  virtual vec3 sample_at_surface() = 0;

protected:
  vec3 m_last_sample;
  vec3 m_n_at_last_sample;
};

#endif

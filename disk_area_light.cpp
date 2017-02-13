#include <iostream>
#include <cassert>

#include "disk_area_light.hpp"
#include "ray.hpp"

using glm::normalize;
using glm::dot;

vec3 DiskAreaLight::diffuse(vec3 normal, Ray & r, vec3 i_pos, Material & m) const {
float d, att, ln_dot_d, d2, g;
  vec3 l_dir, ref;

  l_dir = normalize(direction(i_pos));
  ln_dot_d = dot(-m_n_at_last_sample, l_dir);
  if (ln_dot_d > 0.0f) {
    d2 = distance(i_pos);
    d2 *= d2;
    g = ln_dot_d / d2;
    d = distance(i_pos);
    att = 1.0f / (m_const_att + (m_lin_att * d) + (m_quad_att * (d * d)));
    return (att * m.m_brdf->diffuse(l_dir, normal, r, i_pos, m_diffuse) * g) / m_figure->pdf();

  } else
    return vec3(0.0f);
}

vec3 DiskAreaLight::specular(vec3 normal, Ray & r, vec3 i_pos, Material & m) const {
  float d, att, ln_dot_d;
  vec3 l_dir, ref;

  l_dir = normalize(direction(i_pos));
  ln_dot_d = dot(-m_n_at_last_sample, l_dir);
  if (ln_dot_d > 0.0f) {
    d = distance(i_pos);
    att = 1.0f / (m_const_att + (m_lin_att * d) + (m_quad_att * (d * d)));
    return (att * m.m_brdf->specular(l_dir, normal, r, i_pos, m_specular, m.m_shininess)) / m_figure->pdf();

  } else
    return vec3(0.0f);
}

void DiskAreaLight::sample_at_surface(vec3 point) {
  Disk * d = static_cast<Disk *>(m_figure);
  m_last_sample = m_figure->sample_at_surface();
  m_n_at_last_sample = d->m_normal;
}

#include <iostream>
#include <cassert>

#include "disk_area_light.hpp"
#include "ray.hpp"

using glm::normalize;

const float BIAS = 0.000001f;

vec3 DiskAreaLight::diffuse(vec3 normal, Ray & r, vec3 i_pos, Material & m) const {
  float d, att;
  vec3 l_dir, ref;

  l_dir = normalize(direction(i_pos));
  d = distance(i_pos);
  att = 1.0f / (m_const_att + (m_lin_att * d) + (m_quad_att * (d * d)));

  return (att * m.m_brdf->diffuse(l_dir, normal, r, i_pos, m_diffuse)) / m_figure->pdf();
}

vec3 DiskAreaLight::specular(vec3 normal, Ray & r, vec3 i_pos, Material & m) const {
float d, att;
  vec3 l_dir, ref;

  l_dir = normalize(direction(i_pos));
  d = distance(i_pos);
  att = 1.0f / (m_const_att + (m_lin_att * d) + (m_quad_att * (d * d)));

  return (att * m.m_brdf->specular(l_dir, normal, r, i_pos, m_specular, m.m_shininess)) / m_figure->pdf();
}

void DiskAreaLight::sample_at_surface(vec3 point) {
  Disk * d = static_cast<Disk *>(m_figure);
  m_last_sample = m_figure->sample_at_surface();
  m_n_at_last_sample = d->m_normal;
}

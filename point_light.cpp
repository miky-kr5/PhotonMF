#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "point_light.hpp"

using glm::pi;
using glm::reflect;
using glm::length;
using glm::normalize;
using glm::dot;
using glm::pow;
using glm::max;

vec3 PointLight::direction(vec3 point) const {
  return normalize(m_position - point);
}

float PointLight::distance(vec3 point) const {
  return length(m_position - point);
}

vec3 PointLight::diffuse(vec3 normal, Ray & r, vec3 i_pos, Material & m) const {
  float d, att;
  vec3 l_dir, ref;

  l_dir = direction(i_pos);
  d = distance(i_pos);
  att = 1.0f / (m_const_att + (m_lin_att * d) + (m_quad_att * (d * d)));

  return att * m.m_brdf->diffuse(l_dir, normal, r, i_pos, m_diffuse);
}

vec3 PointLight::specular(vec3 normal, Ray & r, vec3 i_pos, Material & m) const {
  float d, att;
  vec3 l_dir, ref;

  l_dir = direction(i_pos);
  d = distance(i_pos);
  att = 1.0f / (m_const_att + (m_lin_att * d) + (m_quad_att * (d * d)));

  return att * m.m_brdf->specular(l_dir, normal, r, i_pos, m_specular, m.m_shininess);
}

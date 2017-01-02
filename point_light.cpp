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

inline vec3 PointLight::direction(vec3 point) {
  return normalize(m_position - point);
}

inline float PointLight::distance(vec3 point) {
  return length(m_position - point);
}

vec3 PointLight::shade(vec3 normal, Ray & r, float t, Material & m) const {
  float d, att, n_dot_l, r_dot_l;
  vec3 color, i_pos, l_dir, ref;

  i_pos = r.m_origin + t * r.m_direction;
  l_dir = m_position - i_pos;
  d = length(l_dir);
  l_dir = normalize(l_dir);
  att = 1.0f / (m_const_att + (m_lin_att * d) + (m_quad_att * (d * d)));

  n_dot_l = max(dot(normal, l_dir), 0.0f);
  color += att * (m.m_diffuse / pi<float>()) * m_diffuse * n_dot_l;

  ref = reflect(l_dir, normal);
  r_dot_l = pow(max(dot(ref, r.m_direction), 0.0f), m.m_shininess);
  color += att * m.m_specular * m_specular * r_dot_l;

  return color;
}

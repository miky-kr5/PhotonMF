#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "spot_light.hpp"

using glm::pi;
using glm::reflect;
using glm::length;
using glm::normalize;
using glm::dot;
using glm::pow;
using glm::max;
using glm::acos;
using glm::radians;

vec3 SpotLight::diffuse(vec3 normal, Ray & r, vec3 i_pos, Material & m) const {
  float d, att, spot_effect;
  vec3 l_dir, ref;

  l_dir = direction(i_pos);
  d = distance(i_pos);
  spot_effect = dot(m_spot_dir, -l_dir);
  
  if (acos(spot_effect) < radians(m_spot_cutoff)) {
    spot_effect = pow(spot_effect, m_spot_exponent);
    att = spot_effect / (m_const_att + (m_lin_att * d) + (m_quad_att * (d * d)));
    return att * m.m_brdf->diffuse(l_dir, normal, r, i_pos, m_diffuse);

  } else
    return vec3(0.0f);
}

vec3 SpotLight::specular(vec3 normal, Ray & r, vec3 i_pos, Material & m) const {
  float d, att, spot_effect;
  vec3 l_dir, ref;

  l_dir = direction(i_pos);
  d = distance(i_pos);
  spot_effect = dot(m_spot_dir, -l_dir);

  if (acos(spot_effect) < radians(m_spot_cutoff)) {
    spot_effect = pow(spot_effect, m_spot_exponent);
    att = spot_effect / (m_const_att + (m_lin_att * d) + (m_quad_att * (d * d)));
    return att * m.m_brdf->specular(l_dir, normal, r, i_pos, m_specular, m.m_shininess);
  } else
    return vec3(0.0f);
}

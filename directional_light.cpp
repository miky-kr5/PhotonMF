#include <limits>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "directional_light.hpp"

using std::numeric_limits;
using glm::pi;
using glm::reflect;
using glm::dot;
using glm::pow;
using glm::max;

vec3 DirectionalLight::direction(vec3 point) const {
  return m_position;
}

float DirectionalLight::distance(vec3 point) const {
  return numeric_limits<float>::max();
}

vec3 DirectionalLight::diffuse(vec3 normal, Ray & r, vec3 i_pos, Material & m) const {
  return m.m_brdf->diffuse(m_position, normal, r, i_pos, m_diffuse);
}

vec3 DirectionalLight::specular(vec3 normal, Ray & r, vec3 i_pos, Material & m) const {
  return m.m_brdf->specular(m_position, normal, r, i_pos, m_specular, m.m_shininess);
}

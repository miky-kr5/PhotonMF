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

inline vec3 DirectionalLight::direction(vec3 point) {
  return m_position;
}

inline float DirectionalLight::distance(vec3 point) {
  return numeric_limits<float>::max();
}

vec3 DirectionalLight::shade(vec3 normal, Ray & r, float t, Material & m) const {
  float n_dot_l, r_dot_l;
  vec3 color, ref;

  n_dot_l = max(dot(normal, m_position), 0.0f);
  color += (m.m_diffuse / pi<float>()) * m_diffuse * n_dot_l;

  ref = reflect(m_position, normal);
  r_dot_l = pow(max(dot(ref, r.m_direction), 0.0f), m.m_shininess);
  color += m.m_specular * m_specular * r_dot_l;

  return color;
}

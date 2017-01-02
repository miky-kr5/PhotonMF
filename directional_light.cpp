#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "directional_light.hpp"

using glm::pi;
using glm::reflect;
using glm::dot;

vec3 DirectionalLight::shade(vec3 normal, Ray & r, Material & m) const {
  float n_dot_l, r_dot_l;
  vec3 color, ref;

  //color = m.m_ambient * m_ambient;

  n_dot_l = max(dot(normal, m_position), 0.0);
  color += (m.m_diffuse / pi<float>()) * m_diffuse * n_dot_l;

  ref = reflect(m_position, normal);
  r_dot_l = pow(max(dot(ref, r.m_direction), 0.0f), m.m_shininess);
  color += m.m_specular * m_specular * r_dot_l;

  return color;
}

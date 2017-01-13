#include "glm/glm.hpp"

#include "hsa_brdf.hpp"

using glm::reflect;
using glm::pow;
using glm::max;
using glm::dot;

vec3 HeidrichSeidelAnisotropicBRDF::diffuse(vec3 light_dir, vec3 surface_normal, Ray & incident_ray, vec3 intersection_point, vec3 light_diff_color) const {
  vec3 T = normalize(thread_dir + (dot(-thread_dir, surface_normal) * surface_normal));
  float k_diff = glm::sqrt(1 - max(dot(light_dir, T), 0.0f));
  float n_dot_l = max(dot(surface_normal, light_dir), 0.0f);
  return light_diff_color * k_diff * n_dot_l;
}

vec3 HeidrichSeidelAnisotropicBRDF::specular(vec3 light_dir, vec3 surface_normal, Ray & incident_ray, vec3 intersection_point, vec3 light_spec_color, float shininess) const {
  vec3 T = normalize(thread_dir + (dot(-thread_dir, surface_normal) * surface_normal));
  float n_dot_l = max(dot(surface_normal, light_dir), 0.0f);
  float l_dot_t = max(dot(-light_dir, T), 0.0f);
  float v_dot_t = max(dot(-incident_ray.m_direction, T), 0.0f);
  float k_spec = pow(glm::sqrt(1.0f - (l_dot_t * l_dot_t)) * glm::sqrt(1.0f - (v_dot_t * v_dot_t)) - (l_dot_t * v_dot_t), shininess);
  return n_dot_l * ((light_spec_color * k_spec) + PhongBRDF::specular(light_dir, surface_normal, incident_ray, intersection_point, light_spec_color, shininess));
}

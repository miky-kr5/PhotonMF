#include "glm/glm.hpp"

#include "phong_brdf.hpp"

using glm::reflect;
using glm::pow;
using glm::max;
using glm::dot;

vec3 PhongBRDF::diffuse(vec3 light_dir, vec3 surface_normal, Ray & incident_ray, vec3 intersection_point, vec3 light_diff_color) const {
  float n_dot_l = max(dot(surface_normal, light_dir), 0.0f);
  return light_diff_color * n_dot_l;
}

vec3 PhongBRDF::specular(vec3 light_dir, vec3 surface_normal, Ray & incident_ray, vec3 intersection_point, vec3 light_spec_color, float shininess) const {
  vec3 ref = reflect(light_dir, surface_normal);
  float r_dot_l = pow(max(dot(ref, incident_ray.m_direction), 0.0f), shininess);
  return light_spec_color * r_dot_l;
}

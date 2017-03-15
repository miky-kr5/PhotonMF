#include <iostream>
#include <glm/gtc/constants.hpp>

#include "sphere.hpp"
#include "sampling.hpp"

using namespace glm;

bool Sphere::intersect(Ray & r, float & t) const {
  float t1, t2;
  float d;

  float a = (r.m_direction.x * r.m_direction.x) +
    (r.m_direction.y * r.m_direction.y) +
    (r.m_direction.z * r.m_direction.z);

  float b = (2 * r.m_direction.x * (r.m_origin.x - m_center.x)) +
    (2 * r.m_direction.y * (r.m_origin.y - m_center.y)) +
    (2 * r.m_direction.z * (r.m_origin.z - m_center.z));

  float c = (m_center.x * m_center.x) +
    (m_center.y * m_center.y) +
    (m_center.z * m_center.z) +
    (r.m_origin.x * r.m_origin.x) +
    (r.m_origin.y * r.m_origin.y) +
    (r.m_origin.z * r.m_origin.z) -
    2 * ((m_center.x * r.m_origin.x) + (m_center.y * r.m_origin.y) + (m_center.z * r.m_origin.z)) - (m_radius * m_radius);

  d = (b * b) - (4 * a * c);

  if (d >= 0.0f) {
    t1 = (-b - sqrt(d)) / (2 * a); 
    t2 = (-b + sqrt(d)) / (2 * a);
    t = t1 < t2 ? t1 : t2;
    return t >= 0.0f;

  } else
    return false;
}

vec3 Sphere::normal_at_int(Ray & r, float & t) const {
  vec3 i = vec3(r.m_origin + (t * r.m_direction));
  return normalize(vec3((i - m_center) / m_radius));
}

vec3 Sphere::sample_at_surface() const {
  return sample_sphere(m_center, m_radius);
}

void Sphere::calculate_inv_area() {
  m_inv_area = 1.0f / (4.0 * pi<float>() * (m_radius * m_radius));
}

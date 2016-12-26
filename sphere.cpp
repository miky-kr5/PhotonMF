#include <cmath>

#include "sphere.hpp"

using glm::normalize;

bool Sphere::intersect(Ray & r, float & t, vec3 & n) const {
  vec3 i;
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
    t = (-b - sqrt(d)) / (2 * a);
    i = vec3(r.m_origin + (t * r.m_direction));
    n = normalize(vec3((i - m_center) / m_radius));
  }

  return d >= 0.0f;
}

#include "plane.hpp"

#define TOL 1e-6

using glm::abs;
using glm::dot;

bool Plane::intersect(Ray & r, float & t) const {
  float d = dot(r.m_direction, m_normal);

  if (abs(d) > TOL) {
    t = dot(m_normal, (m_point - r.m_origin)) / d;
    return t >= 0.0f;
  }

   return false;
}

vec3 Plane::normal_at_int(Ray & r, float & t) const {
  return vec3(m_normal);
}

vec3 Plane::sample_at_surface() const {
  return vec3(0.0f);
}

void Plane::calculate_inv_area() {
  m_inv_area = 0.0f;
}

#include <cmath>

#include "plane.hpp"

#define TOL 1e-6

using std::abs;
using glm::dot;

bool Plane::intersect(Ray & r, float & t, vec3 & n) const {
  float d = dot(r.m_direction, m_normal);

  if (abs(d) > TOL) {
    t = dot(m_normal, (m_point - r.m_origin)) / d;
    n = vec3(m_normal);
    return t >= 0.0f;
  }

   return false;
}

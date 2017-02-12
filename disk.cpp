#include <iostream>
#include <cassert>

#include "disk.hpp"
#include "sampling.hpp"

using glm::vec2;
using glm::cos;
using glm::sin;
using glm::dot;
using glm::distance;
using glm::cross;
using glm::abs;

bool Disk::intersect(Ray & r, float & t) const {
  float _t;
  vec3 i_pos, i_vec;

  if (Plane::intersect(r, _t)) {
    i_pos = r.m_origin + (_t * r.m_direction);
    i_vec = i_pos - m_point;
    t = _t;
    return dot(i_vec, i_vec) <= (m_radius * m_radius);
  }

   return false;
}

vec3 Disk::sample_at_surface() const {
  float theta = random01() * pi2;
  float r = random01() * m_radius;
  vec3 nt, nb;
  create_coords_system(m_normal, nt, nb);
  float x = m_point.x + (r * cos(theta) * nt.x) + (r * sin(theta) * nb.x);
  float y = m_point.y + (r * cos(theta) * nt.y) + (r * sin(theta) * nb.y);
  float z = m_point.z + (r * cos(theta) * nt.z) + (r * sin(theta) * nb.z);

  return vec3(x, y, z);
}

void Disk::calculate_inv_area() {
  m_inv_area = 1.0f / pi<float>() * (m_radius * m_radius);
}

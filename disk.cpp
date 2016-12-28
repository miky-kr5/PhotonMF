#include "disk.hpp"

using glm::dot;

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

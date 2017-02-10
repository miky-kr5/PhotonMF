#include <glm/gtc/constants.hpp>

#include "disk.hpp"
#include "sampling.hpp"

using glm::cos;
using glm::sin;
using glm::dot;
using glm::pi;

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
  float theta = random01() * (2.0f * pi<float>());
  float r = glm::sqrt(random01() * m_radius);
  float x = r * cos(theta);
  float y = r * sin(theta);
  float z = 0.0f;
  vec3 sample = vec3(x, y, z);
  rotate_sample(sample, m_normal);
  return sample;
}

void Disk::calculate_inv_area() {
  m_inv_area = 1.0f / pi<float>() * (m_radius * m_radius);
}

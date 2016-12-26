#pragma once
#ifndef RAY_HPP
#define RAY_HPP

#include <glm/vec3.hpp>

using glm::vec3;

class Ray {
public:
  vec3 m_direction;
  vec3 m_origin;

  Ray(): m_direction(vec3(0.0f, 0.0f, -1.0f)), m_origin(vec3(0.0f)) { }
  Ray(float dx, float dy, float dz, float ox, float oy, float oz): m_direction(vec3(dx, dy, dz)), m_origin(vec3(ox, oy, oz)) {}
  Ray(vec3 _d, vec3 _o): m_direction(_d), m_origin(_o) { }
};

#endif

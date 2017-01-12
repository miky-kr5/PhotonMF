#pragma once
#ifndef PLANE_HPP
#define PLANE_HPP

#include <glm/glm.hpp>

#include "figure.hpp"

using glm::vec3;
using glm::normalize;

class Plane : public Figure {
public:
  vec3 m_point;
  vec3 m_normal;
  
  Plane(BRDF * _brdf = NULL): Figure(_brdf), m_point(vec3(0.0f)), m_normal(vec3(0.0f, 0.0f, 1.0f)) { }

  Plane(float x, float y, float z, float nx, float ny, float nz, BRDF * _brdf = NULL): Figure(_brdf), m_point(vec3(x, y, z)), m_normal(normalize(vec3(nx, ny, nz))) { }

  Plane(vec3 _p, vec3 _n, BRDF * _brdf = NULL): Figure(_brdf), m_point(_p), m_normal(normalize(_n)) { }

  virtual ~Plane() { }

  virtual bool intersect(Ray & r, float & t) const;

  virtual vec3 normal_at_int(Ray & r, float & t) const;
};


#endif

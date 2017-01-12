#pragma once
#ifndef SPHERE_HPP
#define SPHERE_HPP

#include <glm/glm.hpp>

#include "figure.hpp"

using glm::vec3;

class Sphere : public Figure {
public:
  vec3 m_center;
  float m_radius;
  
  Sphere(BRDF * _brdf = NULL): Figure(_brdf), m_center(vec3(0.0f)), m_radius(0.5f) { }

  Sphere(float x, float y, float z, float r, BRDF * _brdf = NULL): Figure(_brdf), m_center(vec3(x, y, z)), m_radius(r) { }

  Sphere(vec3 _c, float r, BRDF * _brdf = NULL): Figure(_brdf), m_center(_c), m_radius(r) { }

  virtual ~Sphere() { }

  virtual bool intersect(Ray & r, float & t) const;

  virtual vec3 normal_at_int(Ray & r, float & t) const;
};

#endif

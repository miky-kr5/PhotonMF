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
  
  Sphere(): m_center(vec3(0.0f)), m_radius(0.5f) { }

  Sphere(float x, float y, float z, float r): m_center(vec3(x, y, z)), m_radius(r) { }

  Sphere(vec3 _c, float r): m_center(_c), m_radius(r) { }

  virtual ~Sphere() { }

  virtual bool intersect(Ray & r, float & t, vec3 & n) const;
};

#endif

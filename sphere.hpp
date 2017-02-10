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
  
  Sphere(Material * mat = NULL): Figure(mat), m_center(vec3(0.0f)), m_radius(0.5f) {
    calculate_inv_area();
  }

  Sphere(float x, float y, float z, float r, Material * mat = NULL): Figure(mat), m_center(vec3(x, y, z)), m_radius(r) {
    calculate_inv_area();
  }

  Sphere(vec3 _c, float r, Material * mat = NULL): Figure(mat), m_center(_c), m_radius(r) {
    calculate_inv_area();
  }

  virtual ~Sphere() { }

  virtual bool intersect(Ray & r, float & t) const;
  virtual vec3 normal_at_int(Ray & r, float & t) const;
  virtual vec3 sample_at_surface() const;

private:
  virtual void calculate_inv_area();
};

#endif

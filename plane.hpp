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
  
  Plane(Material * mat = NULL): Figure(mat), m_point(vec3(0.0f)), m_normal(vec3(0.0f, 0.0f, 1.0f)) {
    calculate_inv_area();
  }

  Plane(float x, float y, float z, float nx, float ny, float nz, Material * mat = NULL):
    Figure(mat),
    m_point(vec3(x, y, z)),
    m_normal(normalize(vec3(nx, ny, nz)))
  {
    calculate_inv_area();
  }

  Plane(vec3 _p, vec3 _n, Material * mat = NULL): Figure(mat), m_point(_p), m_normal(normalize(_n)) { }

  virtual ~Plane() { }

  virtual bool intersect(Ray & r, float & t) const;
  virtual vec3 normal_at_int(Ray & r, float & t) const;
  virtual vec3 sample_at_surface() const;

protected:
  virtual void calculate_inv_area();
};


#endif

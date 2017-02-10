#pragma once
#ifndef FIGURE_HPP
#define FIGURE_HPP

#include <glm/vec3.hpp>

#include "ray.hpp"
#include "material.hpp"

using glm::vec3;

class Figure {
public:
  Material * m_mat;

  Figure(Material * mat = NULL): m_inv_area(0.0f) {
    m_mat = mat == NULL ? new Material() : mat;
  }
  
  virtual ~Figure() {
    delete m_mat;
  }

  virtual float pdf() const {
    return m_inv_area;
  }

  virtual bool intersect(Ray & r, float & t) const = 0;
  virtual vec3 normal_at_int(Ray & r, float & t) const = 0;
  virtual vec3 sample_at_surface() const = 0;

protected:
  float m_inv_area;

  virtual void calculate_inv_area() = 0;
};

#endif

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

  Figure(BRDF * brdf = NULL) {
    m_mat = new Material(brdf);
  }
  
  virtual ~Figure() {
    delete m_mat;
  }

  virtual bool intersect(Ray & r, float & t) const = 0;
  virtual vec3 normal_at_int(Ray & r, float & t) const = 0;
};

#endif

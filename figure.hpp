#pragma once
#ifndef FIGURE_HPP
#define FIGURE_HPP

#include <glm/vec3.hpp>

#include "ray.hpp"

class Figure {
public:
  vec3 color;

  virtual ~Figure() { }

  virtual bool intersect(Ray & r, float & t) const = 0;

  virtual void set_color(float r, float g, float b) {
    color = vec3(r, g, b);
  }

  virtual void set_color(vec3 rgb) {
    color = vec3(rgb);
  }
};

#endif

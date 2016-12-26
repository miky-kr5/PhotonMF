#pragma once
#ifndef TRACER_HPP
#define TRACER_HPP

#include <vector>

#include <glm/glm.hpp>

#include "figure.hpp"
#include "light.hpp"
#include "ray.hpp"

using std::vector;
using glm::vec3;
using glm::vec2;

class Tracer {
public:
  int m_h;
  int m_w;
  float m_fov;
  float m_a_ratio;

  Tracer();
  Tracer(int w, int h, float fov);

  vec2 sample_pixel(int i, int j) const;
  vec3 trace_ray(Ray & r, vector<Figure *> & f, vector<Light *> & l, unsigned int rec_level) const;
};

#endif

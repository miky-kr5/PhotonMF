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

  Tracer(): m_h(480), m_w(640), m_fov(90.0f), m_a_ratio(640.0f / 480.0f) { }

  Tracer(int h, int w, float fov): m_h(h), m_w(w), m_fov(fov) {
    m_a_ratio = static_cast<float>(w) / static_cast<float>(h);
  };

  vec2 sample_pixel(int i, int j) const;
  vec3 trace_ray(Ray & r, vector<Figure *> & v_figures, vector<Light *> & v_lights, unsigned int rec_level) const;
};

#endif

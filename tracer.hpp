#pragma once
#ifndef TRACER_HPP
#define TRACER_HPP

#include <vector>

#include <glm/glm.hpp>

#include "figure.hpp"
#include "light.hpp"
#include "ray.hpp"

using std::vector;
using glm::vec2;
using glm::vec3;

#define MAX_RECURSION 3
#define BIAS 0.000001f

extern const vec3 BCKG_COLOR;

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

  virtual ~Tracer() { }

  vec2 sample_pixel(int i, int j) const;
  virtual vec3 trace_ray(Ray & r, vector<Figure *> & v_figures, vector<Light *> & v_lights, unsigned int rec_level) const = 0;

protected:
  float random01() const;
  float fresnel(const vec3 & i, const vec3 & n, const float ir1, const float ir2) const;
  void create_coords_system(const vec3 &n, vec3 &nt, vec3 &nb) const;
  vec3 sample_hemisphere(const float r1, const float r2) const;
  void rotate_sample(vec3 & sample, const vec3 & n) const;
};

#endif

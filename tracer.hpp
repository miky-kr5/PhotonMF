#pragma once
#ifndef TRACER_HPP
#define TRACER_HPP

#include <vector>

#include <glm/glm.hpp>

#include "ray.hpp"
#include "scene.hpp"

using std::vector;
using glm::vec2;
using glm::vec3;

extern const float BIAS;

extern const vec3 BCKG_COLOR;

class Tracer {
public:
  unsigned int m_max_depth;

  Tracer(): m_max_depth(5) { }

  Tracer(unsigned int max_depth): m_max_depth(max_depth) { }

  virtual ~Tracer() { }

  virtual vec3 trace_ray(Ray & r, Scene * s, unsigned int rec_level) const = 0;

protected:
  float fresnel(const vec3 & i, const vec3 & n, const float ir1, const float ir2) const;
};

#endif

#pragma once
#ifndef PATH_TRACER_HPP
#define PATH_TRACER_HPP

#include "tracer.hpp"

class PathTracer: public Tracer {
public:
  PathTracer(): Tracer() { }

  PathTracer(int h, int w, float fov, unsigned int max_depth): Tracer(h, w, fov, max_depth) { };

  virtual ~PathTracer();

  virtual vec3 trace_ray(Ray & r, vector<Figure *> & v_figures, vector<Light *> & v_lights, unsigned int rec_level) const;
};

#endif

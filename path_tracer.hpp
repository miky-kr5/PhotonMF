#pragma once
#ifndef PATH_TRACER_HPP
#define PATH_TRACER_HPP

#include "tracer.hpp"

class PathTracer: public Tracer {
public:
  PathTracer(): Tracer() { }

  PathTracer(unsigned int max_depth): Tracer(max_depth) { };

  virtual ~PathTracer();

  virtual vec3 trace_ray(Ray & r, Scene * s, unsigned int rec_level) const;
};

#endif

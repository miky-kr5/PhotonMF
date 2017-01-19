#pragma once
#ifndef WHITTED_TRACER_HPP
#define WHITTED_TRACER_HPP

#include "tracer.hpp"

class WhittedTracer: public Tracer {
public:
  WhittedTracer(): Tracer() { }

  WhittedTracer(unsigned int max_depth): Tracer(max_depth) { };

  virtual ~WhittedTracer();

  virtual vec3 trace_ray(Ray & r, Scene * s, unsigned int rec_level) const;
};

#endif

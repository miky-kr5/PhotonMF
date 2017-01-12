#pragma once
#ifndef WHITTED_TRACER_HPP
#define WHITTED_TRACER_HPP

#include "tracer.hpp"

class WhittedTracer: public Tracer {
public:
  WhittedTracer(): Tracer() { }

  WhittedTracer(int h, int w, float fov): Tracer(h, w, fov) { };

  virtual ~WhittedTracer();

  virtual vec3 trace_ray(Ray & r, vector<Figure *> & v_figures, vector<Light *> & v_lights, unsigned int rec_level) const;
};

#endif

#pragma once
#ifndef PATH_TRACER_HPP
#define PATH_TRACER_HPP

#include "tracer.hpp"

class PathTracer: public Tracer {
public:
  bool indirect_l;

  PathTracer(): Tracer(), indirect_l(false) { }

  PathTracer(int h, int w, float fov, bool il): Tracer(h, w, fov), indirect_l(il) { };

  virtual ~PathTracer();

  virtual vec3 trace_ray(Ray & r, vector<Figure *> & v_figures, vector<Light *> & v_lights, unsigned int rec_level) const;
};

#endif

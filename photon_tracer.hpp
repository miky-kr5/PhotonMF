#pragma once
#ifndef PHOTON_TRACER_HPP
#define PHOTON_TRACER_HPP

#include "tracer.hpp"
#include "kd_tree.hpp"

class PhotonTracer: public Tracer {
public:
  PhotonTracer(): Tracer() { }

  PhotonTracer(unsigned int max_depth): Tracer(max_depth) { };

  virtual ~PhotonTracer();

  virtual vec3 trace_ray(Ray & r, Scene * s, unsigned int rec_level) const;

  void build_photon_map(kdTree & photon_map, Scene * s, const unsigned int rec_level, const size_t n_photons_per_ligth = 10000, const bool specular = false) const;
};

#endif

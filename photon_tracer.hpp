#pragma once
#ifndef PHOTON_TRACER_HPP
#define PHOTON_TRACER_HPP

#include "tracer.hpp"
#include "kd_tree.hpp"

class PhotonTracer: public Tracer {
public:
  PhotonTracer(): Tracer(), m_h_radius(0.5f) { }
  PhotonTracer(unsigned int max_depth, float _r = 0.5f): Tracer(max_depth), m_h_radius(_r) { };

  virtual ~PhotonTracer();
  virtual vec3 trace_ray(Ray & r, Scene * s, unsigned int rec_level) const;

  void build_photon_map(Scene * s, const size_t n_photons_per_ligth = 10000, const bool specular = false);

private:
  float m_h_radius;
  kdTree m_photon_map;
  vec3 trace_photon(Ray &r, Scene * s, const unsigned int rec_level, const bool specular = false);
};

#endif

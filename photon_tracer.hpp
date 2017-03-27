#pragma once
#ifndef PHOTON_TRACER_HPP
#define PHOTON_TRACER_HPP

#include "tracer.hpp"
#include "kd_tree.hpp"

class PhotonTracer: public Tracer {
public:
  PhotonTracer(): Tracer(), m_h_radius(0.5f), m_cone_filter_k(1.0f) { }
  PhotonTracer(unsigned int max_depth, float _r = 0.5f, float _k = 1.0f): Tracer(max_depth), m_h_radius(_r), m_cone_filter_k(_k < 1.0f ? 1.0f : _k) { };

  virtual ~PhotonTracer();
  virtual vec3 trace_ray(Ray & r, Scene * s, unsigned int rec_level) const;

  void photon_tracing(Scene * s, const size_t n_photons_per_ligth = 10000, const bool specular = false);
  void build_photon_map(const char * photons_file, const bool caustics = false);
  void build_photon_map(const bool caustics = false);
private:
  float m_h_radius;
  float m_cone_filter_k;
  kdTree m_photon_map;
  kdTree m_caustics_map;
  void trace_photon(Photon & ph, Scene * s, const unsigned int rec_level);
};

#endif

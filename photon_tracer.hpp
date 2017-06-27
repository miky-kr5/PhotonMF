#pragma once
#ifndef PHOTON_TRACER_HPP
#define PHOTON_TRACER_HPP

#include "tracer.hpp"
//#include "kd_tree.hpp"
#include "photonmap.hpp"
#include "rgbe.hpp"

struct Vec3
{
  float x;
  float y;
  float z;

  Vec3(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f): x(_x), y(_y), z(_z) { }

  Vec3(const Vec3 & other) = default;

  glm::vec3 toVec3() {
    return glm::vec3(x, y, z);
  }
  
  inline bool equalFloat(const float x, const float y)
  {
    return (x - std::numeric_limits<float>::epsilon() <= y) && (x + std::numeric_limits<float>::epsilon() >= y);
  }

  inline bool operator<=(const Vec3 b)
  {
    return (x < b.x || (equalFloat(x, b.x) && (y < b.y || (equalFloat(y, b.y) && z <= b.z))));
  }

  inline bool operator>=(const Vec3 b)
  {
    return (x > b.x || (equalFloat(x, b.x) && (y > b.y || (equalFloat(y, b.y) && z >= b.z))));
  }

  inline friend std::ostream& operator<<(std::ostream& out, const Vec3& v)
  {
    return out << "X:" << v.x << " Y:" << v.y << " Z:" << v.z;
  }
};

struct PhotonAux
{
  Vec3 position;
  Vec3 direction;
  float ref_index;
  unsigned char radiance[4];
  float r, g, b;

  PhotonAux(Vec3 _p = Vec3(), Vec3 _d = Vec3(), float red = 0.0f, float green = 0.0f, float blue = 0.0f, float _r = 1.0f):
    position(_p),
    direction(_d),
    ref_index(_r),
    r(red),
    g(green),
    b(blue)
  {
    float2rgbe(radiance, red, green, blue);
  }

  inline void getColor(float & red, float & green, float & blue) {
    rgbe2float(red, green, blue, radiance);
  }
};

class PhotonTracer: public Tracer {  
public:
  PhotonTracer():
    Tracer(), m_h_radius(0.5f),
    m_cone_filter_k(1.0f),
    m_photon_map(7000000),
    m_max_s_photons(5000)
  { }
  
  PhotonTracer(unsigned int max_depth, float _r = 0.5f, float _k = 1.0f, const int max_photons = 7000000, const int max_search = 5000):
    Tracer(max_depth),
    m_h_radius(_r),
    m_cone_filter_k(_k < 1.0f ? 1.0f : _k),
    m_photon_map(max_photons),
    m_max_s_photons(max_search)
  { };

  virtual ~PhotonTracer();
  virtual vec3 trace_ray(Ray & r, Scene * s, unsigned int rec_level) const;

  void photon_tracing(Scene * s, const size_t n_photons_per_ligth = 10000, const bool specular = false);
  void build_photon_map(const char * photons_file, const bool caustics = false);
  void build_photon_map(const bool caustics = false);

private:
  float m_h_radius;
  float m_cone_filter_k;
  /*kdTree m_photon_map;
    kdTree m_caustics_map;*/
  PhotonMap m_photon_map;
  int m_max_s_photons;
  void trace_photon(PhotonAux & ph, Scene * s, const unsigned int rec_level);
};

#endif

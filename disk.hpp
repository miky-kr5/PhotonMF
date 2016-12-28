#pragma once
#ifndef DISK_HPP
#define DISK_HPP

#include <glm/glm.hpp>

#include "plane.hpp"

using glm::vec3;
using glm::normalize;

class Disk : public Plane {
public:
  float m_radius;
  
  Disk(): m_radius(1.0f) {
    rho = 0.0f;
    m_point = vec3(0.0f);
    m_normal = vec3(0.0f, 1.0f, 0.0f);
  }

  Disk(float x, float y, float z, float nx, float ny, float nz, float _r): m_radius(_r) {
    rho = 0.0f;
    m_point = vec3(x, y, z);
    m_normal = normalize(vec3(nx, ny, nz));
  }

  Disk(vec3 _p, vec3 _n, float _r): m_radius(_r) {
    rho = 0.0f;
    m_point = _p;
    m_normal = normalize(_n);
  }

  virtual ~Disk() { }

  virtual bool intersect(Ray & r, float & t) const;
};


#endif

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
  
  Disk(BRDF * _brdf = NULL): Plane(_brdf), m_radius(1.0f) { }

  Disk(float x, float y, float z, float nx, float ny, float nz, float _r, BRDF * _brdf = NULL): Plane(x, y, z, nx, ny, nz, _brdf), m_radius(_r) { }

  Disk(vec3 _p, vec3 _n, float _r, BRDF * _brdf = NULL): Plane(_p, _n, _brdf), m_radius(_r) { }

  virtual ~Disk() { }

  virtual bool intersect(Ray & r, float & t) const;
};


#endif

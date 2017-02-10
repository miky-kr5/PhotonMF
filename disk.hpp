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
  
  Disk(Material * mat = NULL): Plane(mat), m_radius(1.0f) {
    calculate_inv_area();
  }

  Disk(float x, float y, float z, float nx, float ny, float nz, float _r, Material * mat = NULL): Plane(x, y, z, nx, ny, nz, mat), m_radius(_r) {
    calculate_inv_area();
  }

  Disk(vec3 _p, vec3 _n, float _r, Material * mat = NULL): Plane(_p, _n, mat), m_radius(_r) {
    calculate_inv_area();
  }

  virtual ~Disk() { }

  virtual bool intersect(Ray & r, float & t) const;
  virtual vec3 sample_at_surface() const;

protected:
  virtual void calculate_inv_area();
};


#endif

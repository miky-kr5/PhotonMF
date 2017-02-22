#pragma once
#ifndef DISK_AREA_LIGHT_HPP
#define DISK_AREA_LIGHT_HPP

#include "area_light.hpp"
#include "disk.hpp"

class DiskAreaLight: public AreaLight {
public:
  DiskAreaLight(Disk * _s, float _c = 1.0, float _l = 0.0, float _q = 0.0): AreaLight(static_cast<Figure *>(_s), _c, _l, _q) { }

  virtual vec3 sample_at_surface();
};

#endif

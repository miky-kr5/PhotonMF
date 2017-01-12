#pragma once
#ifndef PHONG_BRDF_HPP
#define PHONG_BRDF_HPP

#include "brdf.hpp"

class PhongBRDF: public BRDF {
public:
  PhongBRDF() { }
  virtual ~PhongBRDF() { }
  
  virtual vec3 diffuse(vec3 light_dir, vec3 surface_normal, Ray & incident_ray, vec3 light_diff_color) const;
  virtual vec3 specular(vec3 light_dir, vec3 surface_normal, Ray & incident_ray, vec3 light_spec_color, float shininess) const;
};

#endif

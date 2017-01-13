#pragma once
#ifndef HSA_BRDF_HPP
#define HSA_BRDF_HPP

#include "phong_brdf.hpp"

using glm::normalize;

class HeidrichSeidelAnisotropicBRDF: public PhongBRDF {
public:
  vec3 thread_dir;

  HeidrichSeidelAnisotropicBRDF(vec3 _d): PhongBRDF(), thread_dir(normalize(_d)) { }

  virtual ~HeidrichSeidelAnisotropicBRDF() { }
  
  virtual vec3 diffuse(vec3 light_dir, vec3 surface_normal, Ray & incident_ray, vec3 intersection_point, vec3 light_diff_color) const;
  virtual vec3 specular(vec3 light_dir, vec3 surface_normal, Ray & incident_ray, vec3 intersection_point, vec3 light_spec_color, float shininess) const;
};

#endif

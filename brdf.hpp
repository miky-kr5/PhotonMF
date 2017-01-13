#pragma once
#ifndef BRDF_HPP
#define BRDF_HPP

#include <glm/vec3.hpp>

#include "ray.hpp"

using glm::vec3;

class BRDF {
public:
  virtual ~BRDF() { }
  
  virtual vec3 diffuse(vec3 light_dir, vec3 surface_normal, Ray & incident_ray, vec3 intersection_point, vec3 light_diff_color) const = 0;
  virtual vec3 specular(vec3 light_dir, vec3 surface_normal, Ray & incident_ray, vec3 intersection_point, vec3 light_spec_color, float shininess) const = 0;
};

#endif

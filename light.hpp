#pragma once
#ifndef LIGHT_HPP
#define LIGHT_HPP

#include <glm/vec3.hpp>

#include "ray.hpp"
#include "material.hpp"

using glm::vec3;

class Light {
public:
  vec3 m_position;
  vec3 m_diffuse;
  vec3 m_specular;

  virtual ~Light() { }

  virtual vec3 direction(vec3 point) = 0;
  virtual float distance(vec3 point) = 0;
  virtual vec3 shade(vec3 normal, Ray & r, float t, Material & m) const = 0;
};

#endif

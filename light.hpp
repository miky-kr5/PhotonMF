#pragma once
#ifndef LIGHT_HPP
#define LIGHT_HPP

#include <glm/vec3.hpp>

class Light {
public:
  vec3 position;
  vec3 diffuse;
  vec3 specular;
  vec3 ambient;

  Light(): position(vec3(0.0f)), diffuse(vec3(1.0f)), specular(vec3(1.0f)), ambient(vec3(0.1f)) { }
  Light(vec3 _p, vec3 _d, vec3 _s, vec3 _a): position(_p), diffuse(_d), specular(_s), ambient(_a) { }
};

#endif

#pragma once
#ifndef LIGHT_HPP
#define LIGHT_HPP

#include <glm/vec3.hpp>

using glm::vec3;

class Light {
public:
  vec3 m_position;
  vec3 m_diffuse;
  vec3 m_specular;
  vec3 m_ambient;

  Light(): m_position(vec3(0.0f)), m_diffuse(vec3(1.0f)), m_specular(vec3(1.0f)), m_ambient(vec3(0.1f)) { }
  Light(vec3 _p, vec3 _d, vec3 _s, vec3 _a): m_position(_p), m_diffuse(_d), m_specular(_s), m_ambient(_a) { }
};

#endif

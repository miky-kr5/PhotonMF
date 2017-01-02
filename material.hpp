#pragma once
#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include <glm/vec3.hpp>

using glm::vec3;

class Material {
public:
  vec3 m_diffuse;
  vec3 m_specular;
  vec3 m_ambient;
  float m_rho;
  float m_shininess;
  float m_ref_index;

  Material(): m_diffuse(vec3(1.0f)), m_specular(vec3(1.0f)), m_ambient(vec3(1.0f)), m_rho(0.0f), m_shininess(89.0f), m_ref_index(1.0f) { }

  Material(const Material & m) {
    m_diffuse = m.m_diffuse;
    m_specular = m.m_specular;
    m_ambient = m.m_ambient;
    m_rho = m.m_rho;
    m_shininess = m.m_shininess;
    m_ref_index = m.m_ref_index;
  }
};

#endif

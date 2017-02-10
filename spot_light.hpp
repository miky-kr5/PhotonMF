#pragma once
#ifndef SPOT_LIGHT_HPP
#define SPOT_LIGHT_HPP

#include <glm/glm.hpp>

#include "point_light.hpp"

using glm::vec3;
using glm::normalize;

class SpotLight: public PointLight {
public:
  float m_spot_cutoff;
  float m_spot_exponent;
  vec3 m_spot_dir;

  SpotLight(): PointLight(), m_spot_cutoff(45.0f), m_spot_exponent(0.0f), m_spot_dir(vec3(0.0f, -1.0f, 0.0f)) { }

  SpotLight(vec3 _p,
	    vec3 _d,
	    vec3 _s,
	    float _c,
	    float _l,
	    float _q,
	    float _sco,
	    float _se,
	    vec3 _sd):
    PointLight(_p, _d, _s, _c, _l, _q),
    m_spot_cutoff(_sco),
    m_spot_exponent(_se),
    m_spot_dir(normalize(_sd))
  { }

  virtual vec3 diffuse(vec3 normal, Ray & r, vec3 i_pos, Material & m) const;
  virtual vec3 specular(vec3 normal, Ray & r, vec3 i_pos, Material & m) const;
};

#endif

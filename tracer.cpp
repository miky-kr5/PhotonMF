#include <iostream>
#include <limits>
#include <cstdlib>
#include <cmath>

#include <omp.h>

#include "tracer.hpp"

#define PI 3.14159265358979f
#define SHININESS 89.0f

using namespace std;

using std::numeric_limits;
using glm::normalize;
using glm::radians;
using glm::dot;
using glm::reflect;
using glm::clamp;

static const vec3 BCKG_COLOR = vec3(0.16f, 0.66f, 0.72f);

static inline float max(float a, float b) {
  return a >= b ? a : b;
}

static inline float random01() {
  return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}

Tracer::Tracer(): m_h(480), m_w(640), m_fov(90.0f), m_a_ratio(640.0f / 480.0f) {}

Tracer::Tracer(int h, int w, float fov): m_h(h), m_w(w), m_fov(fov) {
  m_a_ratio = static_cast<float>(w) / static_cast<float>(h);
}

vec2 Tracer::sample_pixel(int i, int j) const {
  float pxNDC;
  float pyNDC;
  float pxS;
  float pyS;
  pyNDC = (static_cast<float>(i) + random01()) / m_h;
  pyS = (1.0f - (2.0f * pyNDC)) * tan(radians(m_fov) / 2);
  pxNDC = (static_cast<float>(j) + random01()) / m_w;
  pxS = (2.0f * pxNDC) - 1.0f;
  pxS *= m_a_ratio * tan(radians(m_fov) / 2);

  return vec2(pxS, pyS);
}

vec3 Tracer::trace_ray(Ray & r, vector<Figure *> & vf, vector<Light *> & vl, unsigned int rec_level) const {
  size_t f_index;
  float t, _t, n_dot_l;
  Figure * _f;
  vec3 n, color, i_pos, ref;
  Ray sr;
  bool vis;

  t = numeric_limits<float>::max();
  _f = NULL;
  
  for (size_t f = 0; f < vf.size(); f++) {
    if (vf[f]->intersect(r, _t) && _t < t) {
      t = _t;
      _f = vf[f];
      f_index = f;
    }
  }

  if (_f != NULL) {
    i_pos = r.m_origin + (t * r.m_direction);
    n = _f->normal_at_int(r, t);

    for (size_t l = 0; l < vl.size(); l++) {
      vis = true;
      sr = Ray(vl[l]->m_position, i_pos);

      for (size_t f = 0; f < vf.size(); f++) {
	if (f != f_index && vf[f]->intersect(sr, _t)) {
	  vis = false;
	  break;
	}
      }

      n_dot_l = max(dot(n, vl[l]->m_position), 0.0);
      color += (vis ? 1.0f : 0.0f) * (_f->color / PI) * vl[l]->m_diffuse * n_dot_l;

      ref = reflect(vl[l]->m_position, n);
      color += (vis ? 1.0f : 0.0f) * vl[l]->m_specular * pow(max(dot(ref, r.m_direction), 0.0f), SHININESS);
    }

    return clamp(color, 0.0f, 1.0f);
  } else
    return vec3(BCKG_COLOR);
}

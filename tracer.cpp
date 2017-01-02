#include <iostream>
#include <limits>
#include <cstdlib>

#include "tracer.hpp"

#define MAX_RECURSION 9
#define BIAS 0.000001f

using namespace std;

using std::numeric_limits;
using glm::normalize;
using glm::radians;
using glm::dot;
using glm::reflect;
using glm::refract;
using glm::clamp;
using glm::tan;
using glm::sqrt;

static const vec3 BCKG_COLOR = vec3(0.16f, 0.66f, 0.72f);

static inline float random01() {
  return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}

static float fresnel(const vec3 & i, const vec3 & n, const float ir1, const float ir2) {
  float cos_t1 = dot(i, n);
  float cos_t2 = dot(normalize(refract(i, n, ir1 / ir2)), n);
  float sin_t2 = (ir1 / ir2) * sqrt(1.0f - (cos_t2 * cos_t2));

  if (sin_t2 >= 1.0f)
    return 1.0f;

  float fr_par = ((ir2 * cos_t1) - (ir1 * cos_t2)) / ((ir2 * cos_t1) + (ir1 * cos_t2));
  float fr_per = ((ir1 * cos_t2) - (ir2 * cos_t1)) / ((ir1 * cos_t2) + (ir2 * cos_t1));

  return ((fr_par * fr_par) + (fr_per * fr_per)) / 2.0f;
}

vec2 Tracer::sample_pixel(int i, int j) const {
  float pxNDC;
  float pyNDC;
  float pxS;
  float pyS;
  pyNDC = (static_cast<float>(i) + random01()) / m_h;
  pyS = (1.0f - (2.0f * pyNDC)) * tan(radians(m_fov / 2));
  pxNDC = (static_cast<float>(j) + random01()) / m_w;
  pxS = (2.0f * pxNDC) - 1.0f;
  pxS *= m_a_ratio * tan(radians(m_fov / 2));

  return vec2(pxS, pyS);
}

vec3 Tracer::trace_ray(Ray & r, vector<Figure *> & v_figures, vector<Light *> & v_lights, unsigned int rec_level) const {
  float t, _t;
  Figure * _f;
  vec3 n, color, i_pos, ref;
  Ray mv_r, sr, rr;
  bool vis;
  float kr;

  t = numeric_limits<float>::max();
  _f = NULL;

  for (size_t f = 0; f < v_figures.size(); f++) {
    if (v_figures[f]->intersect(r, _t) && _t < t) {
      t = _t;
      _f = v_figures[f];
    }
  }

  if (_f != NULL) {
    i_pos = r.m_origin + (t * r.m_direction);
    n = _f->normal_at_int(r, t);

    for (size_t l = 0; l < v_lights.size(); l++) {
      vis = true;
      sr = Ray(v_lights[l]->direction(i_pos), i_pos + n * BIAS);

      for (size_t f = 0; f < v_figures.size(); f++) {
	if (v_figures[f]->intersect(sr, _t) && _t < v_lights[l]->distance(i_pos)) {
	  vis = false;
	  break;
	}
      }

      color += (vis ? 1.0f : 0.0f) * v_lights[l]->shade(n, r, t, _f->m_mat);
    }

    if (_f->m_mat.m_refract)
      kr = fresnel(r.m_direction, n, r.m_ref_index, _f->m_mat.m_ref_index);
    else
      kr = _f->m_mat.m_rho;

    if (kr > 0.0f && rec_level < MAX_RECURSION) {
      rr = Ray(normalize(reflect(r.m_direction, n)), i_pos + n * BIAS);
      color += _f->m_mat.m_rho * kr * trace_ray(rr, v_figures, v_lights, rec_level + 1);

    } else if (rec_level >= MAX_RECURSION)
      return vec3(BCKG_COLOR);

    if (_f->m_mat.m_refract && kr < 1.0f && rec_level < MAX_RECURSION) {
      rr = Ray(normalize(refract(r.m_direction, n, r.m_ref_index / _f->m_mat.m_ref_index)), i_pos - n * BIAS, _f->m_mat.m_ref_index);
      color += (1.0f - _f->m_mat.m_rho) * (1.0f - kr) * trace_ray(rr, v_figures, v_lights, rec_level + 1);

    } else if (rec_level >= MAX_RECURSION)
      return vec3(BCKG_COLOR);

    return clamp(color, 0.0f, 1.0f);

  } else
    return vec3(BCKG_COLOR);
}

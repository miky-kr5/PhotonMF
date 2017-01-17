#include <limits>

#include <glm/gtc/constants.hpp>

#include "whitted_tracer.hpp"

using std::numeric_limits;
using namespace glm;

WhittedTracer::~WhittedTracer() { }

vec3 WhittedTracer::trace_ray(Ray & r, vector<Figure *> & v_figures, vector<Light *> & v_lights, Environment * e, unsigned int rec_level) const {
  float t, _t;
  Figure * _f;
  vec3 n, color, i_pos, ref, dir_diff_color, dir_spec_color;
  Ray mv_r, sr, rr;
  bool vis;
  float kr;

  t = numeric_limits<float>::max();
  _f = NULL;

  // Find the closest intersecting surface.
  for (size_t f = 0; f < v_figures.size(); f++) {
    if (v_figures[f]->intersect(r, _t) && _t < t) {
      t = _t;
      _f = v_figures[f];
    }
  }

  // If this ray intersects something:
  if (_f != NULL) {
    // Take the intersection point and the normal of the surface at that point.
    i_pos = r.m_origin + (t * r.m_direction);
    n = _f->normal_at_int(r, t);
    
    // Check if the material is not reflective/refractive.
    if (!_f->m_mat->m_refract) {
      // Calculate the direct lighting.
      for (size_t l = 0; l < v_lights.size(); l++) {
	// For every light source
	vis = true;

	// Cast a shadow ray to determine visibility.
	sr = Ray(v_lights[l]->direction(i_pos), i_pos + n * BIAS);
	for (size_t f = 0; f < v_figures.size(); f++) {
	  if (v_figures[f]->intersect(sr, _t) && _t < v_lights[l]->distance(i_pos)) {
	    vis = false;
	    break;
	  }
	}

	// Evaluate the shading model accounting for visibility.
	dir_diff_color += vis ? v_lights[l]->diffuse(n, r, i_pos, *_f->m_mat) : vec3(0.0f);
	dir_spec_color += vis ? v_lights[l]->specular(n, r, i_pos, *_f->m_mat) : vec3(0.0f);
      }
      
      color += (dir_diff_color * (_f->m_mat->m_diffuse / pi<float>())) + (_f->m_mat->m_specular * dir_spec_color);

      // Determine the specular reflection color.
      if (_f->m_mat->m_rho > 0.0f && rec_level < m_max_depth) {
	rr = Ray(normalize(reflect(r.m_direction, n)), i_pos + n * BIAS);
	color += _f->m_mat->m_rho * trace_ray(rr, v_figures, v_lights, e, rec_level + 1);
      } else if (_f->m_mat->m_rho > 0.0f && rec_level >= m_max_depth)
	  return vec3(0.0f);

    } else {
      // If the material has transmission enabled, calculate the Fresnel term.
      kr = fresnel(r.m_direction, n, r.m_ref_index, _f->m_mat->m_ref_index);

      // Determine the specular reflection color.
      if (kr > 0.0f && rec_level < m_max_depth) {
	rr = Ray(normalize(reflect(r.m_direction, n)), i_pos + n * BIAS);
	color += kr * trace_ray(rr, v_figures, v_lights, e, rec_level + 1);
      } else if (rec_level >= m_max_depth)
	return vec3(0.0f);

      // Determine the transmission color.
      if (_f->m_mat->m_refract && kr < 1.0f && rec_level < m_max_depth) {
	rr = Ray(normalize(refract(r.m_direction, n, r.m_ref_index / _f->m_mat->m_ref_index)), i_pos - n * BIAS, _f->m_mat->m_ref_index);
	color += (1.0f - kr) * trace_ray(rr, v_figures, v_lights, e, rec_level + 1);
      } else if (rec_level >= m_max_depth)
	  return vec3(0.0f);

    }

    // Return final color.
    return _f->m_mat->m_emission + color;

  } else {
    if (e != NULL)
      return e->get_color(r);
    else
      return vec3(0.0f);
  }
}

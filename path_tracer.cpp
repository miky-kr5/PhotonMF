#include <limits>

#include <glm/gtc/constants.hpp>

#include "path_tracer.hpp"
#include "sampling.hpp"
#include "area_light.hpp"

using std::numeric_limits;
using namespace glm;

PathTracer::~PathTracer() { }

vec3 PathTracer::trace_ray(Ray & r, Scene * s, unsigned int rec_level) const {
  float t, _t;
  Figure * _f;
  vec3 n, color, i_pos, ref, sample, dir_diff_color, dir_spec_color, ind_color, amb_color;
  Ray mv_r, sr, rr;
  bool vis, is_area_light = false;
  float kr, r1, r2;
  AreaLight * al;

  t = numeric_limits<float>::max();
  _f = NULL;

  // Find the closest intersecting surface.
  for (size_t f = 0; f < s->m_figures.size(); f++) {
    if (s->m_figures[f]->intersect(r, _t) && _t < t) {
      t = _t;
      _f = s->m_figures[f];
    }
  }

  // If this ray intersects something:
  if (_f != NULL) {
    // Take the intersection point and the normal of the surface at that point.
    i_pos = r.m_origin + (t * r.m_direction);
    n = _f->normal_at_int(r, t);

    is_area_light = false;
    // Check if the object is an area light;
    for (vector<Light *>::iterator it = s->m_lights.begin(); it != s->m_lights.end(); it++) {
      if ((*it)->light_type() == Light::AREA && static_cast<AreaLight *>(*it)->m_figure == _f)
	is_area_light = true;
    }

    // If the object is an area light, return it's emission value.
    if (is_area_light) {
      return _f->m_mat->m_emission;

    // Check if the material is not reflective/refractive.
    } else if (!_f->m_mat->m_refract) {
      // Calculate the direct lighting.
      for (size_t l = 0; l < s->m_lights.size(); l++) {
	// For every light source
	vis = true;

	if (s->m_lights[l]->light_type() == Light::INFINITESIMAL) {
	  // Cast a shadow ray to determine visibility.
	  sr = Ray(s->m_lights[l]->direction(i_pos), i_pos + (n * BIAS));

	  for (size_t f = 0; f < s->m_figures.size(); f++) {
	    if (s->m_figures[f]->intersect(sr, _t) && _t < s->m_lights[l]->distance(i_pos)) {
	      vis = false;
	      break;
	    }
	  }

	// Evaluate the shading model accounting for visibility.
	dir_diff_color += vis ? s->m_lights[l]->diffuse(n, r, i_pos, *_f->m_mat) : vec3(0.0f);
	dir_spec_color += vis ? s->m_lights[l]->specular(n, r, i_pos, *_f->m_mat) : vec3(0.0f);

	} else if (s->m_lights[l]->light_type() == Light::AREA) {
	  // Cast a shadow ray towards a sample point on the surface of the light source.
	  al = static_cast<AreaLight *>(s->m_lights[l]);
	  al->sample_at_surface();
	  sr = Ray(al->direction(i_pos), i_pos + (n * BIAS));

	  for (size_t f = 0; f < s->m_figures.size(); f++) {
	    // Avoid self-intersection with the light source.
	    if (al->m_figure != s->m_figures[f]) {
	      if (s->m_figures[f]->intersect(sr, _t) && _t < al->distance(i_pos)) {
		vis = false;
		break;
	      }
	    }
	  }

	  // Evaluate the shading model accounting for visibility.
	  dir_diff_color += vis ? s->m_lights[l]->diffuse(n, r, i_pos, *_f->m_mat) : vec3(0.0f);
	  dir_spec_color += vis ? s->m_lights[l]->specular(n, r, i_pos, *_f->m_mat) : vec3(0.0f);
	}
      }

      // Calculate indirect lighting contribution.
      if (rec_level < m_max_depth) {
	r1 = random01();
	r2 = random01();
	sample = sample_hemisphere(r1, r2);
	rotate_sample(sample, n);
	rr = Ray(normalize(sample), i_pos + (sample * BIAS));
	ind_color += r1 * trace_ray(rr, s, rec_level + 1) / PDF;
      }

      // Calculate environment light contribution
      vis = true;

      r1 = random01();
      r2 = random01();
      sample = sample_hemisphere(r1, r2);
      rotate_sample(sample, n);
      rr = Ray(normalize(sample), i_pos + (sample * BIAS));

      // Cast a shadow ray to determine visibility.
      for (size_t f = 0; f < s->m_figures.size(); f++) {
	if (s->m_figures[f]->intersect(rr, _t)) {
	  vis = false;
	  break;
	}
      }

      amb_color = vis ? s->m_env->get_color(rr) * max(dot(n, rr.m_direction), 0.0f) / PDF : vec3(0.0f);

      // Add lighting.
      color += ((dir_diff_color + ind_color + amb_color) * (_f->m_mat->m_diffuse / pi<float>())) + (_f->m_mat->m_specular * dir_spec_color);

      // Determine the specular reflection color.
      if (_f->m_mat->m_rho > 0.0f && rec_level < m_max_depth) {
	rr = Ray(normalize(reflect(r.m_direction, n)), i_pos + n * BIAS);
	color += _f->m_mat->m_rho * trace_ray(rr, s, rec_level + 1);
      } else if (_f->m_mat->m_rho > 0.0f && rec_level >= m_max_depth)
	  return vec3(0.0f);

    } else {
      // If the material has transmission enabled, calculate the Fresnel term.
      kr = fresnel(r.m_direction, n, r.m_ref_index, _f->m_mat->m_ref_index);

      // Determine the specular reflection color.
      if (kr > 0.0f && rec_level < m_max_depth) {
	rr = Ray(normalize(reflect(r.m_direction, n)), i_pos + n * BIAS);
	color += kr * trace_ray(rr, s, rec_level + 1);
      } else if (rec_level >= m_max_depth)
	return vec3(0.0f);

      // Determine the transmission color.
      if (_f->m_mat->m_refract && kr < 1.0f && rec_level < m_max_depth) {
	rr = Ray(normalize(refract(r.m_direction, n, r.m_ref_index / _f->m_mat->m_ref_index)), i_pos - n * BIAS, _f->m_mat->m_ref_index);
	color += (1.0f - kr) * trace_ray(rr, s, rec_level + 1);
      } else if (rec_level >= m_max_depth)
	  return vec3(0.0f);

    }

    // Return final color.
    return _f->m_mat->m_emission + color;

  } else
    return s->m_env->get_color(r);
}

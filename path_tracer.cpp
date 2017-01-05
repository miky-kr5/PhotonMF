#include <limits>

#include <glm/gtc/constants.hpp>

#include "path_tracer.hpp"

using std::numeric_limits;
using namespace glm;

PathTracer::~PathTracer() { }

vec3 PathTracer::trace_ray(Ray & r, vector<Figure *> & v_figures, vector<Light *> & v_lights, unsigned int rec_level) const {
  float t, _t;
  Figure * _f;
  vec3 n, color, i_pos, ref, sample, dir_diff_color, dir_spec_color, ind_color;
  Ray mv_r, sr, rr;
  bool vis;
  float kr, r1, r2;

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
    if( !_f->m_mat.m_refract && _f->m_mat.m_rho == 0.0f) {
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
	dir_diff_color += (vis ? 1.0f : 0.0f) * v_lights[l]->diffuse(n, r, t, _f->m_mat);
	dir_spec_color += (vis ? 1.0f : 0.0f) * v_lights[l]->specular(n, r, t, _f->m_mat);
      }

      // If enabled, calculate indirect lighting contribution.
      if (indirect_l && rec_level < MAX_RECURSION) {
	r1 = random01();
	r2 = random01();
	sample = sample_hemisphere(r1, r2);
	rotate_sample(sample, n);
	rr = Ray(normalize(sample), i_pos + (sample * BIAS));
	ind_color += r1 * trace_ray(rr, v_figures, v_lights, rec_level + 1) / (1.0f / (2.0f * pi<float>()));
      }

      color += ((dir_diff_color + ind_color) * (_f->m_mat.m_diffuse / pi<float>())) + dir_spec_color;

    } else {
      // If the material has reflection/transmission enabled.
      // Calculate the Fresnel term if the surface is refracting.
      if (_f->m_mat.m_refract)
	kr = fresnel(r.m_direction, n, r.m_ref_index, _f->m_mat.m_ref_index);
      else
	kr = _f->m_mat.m_rho;

      // Determinte the specular reflection color.
      if (kr > 0.0f && rec_level < MAX_RECURSION) {
	rr = Ray(normalize(reflect(r.m_direction, n)), i_pos + n * BIAS);
	color += _f->m_mat.m_rho * kr * trace_ray(rr, v_figures, v_lights, rec_level + 1);
      } else if (rec_level >= MAX_RECURSION)
	return vec3(0.0f);

      // Determine the transmission color.
      if (_f->m_mat.m_refract && kr < 1.0f && rec_level < MAX_RECURSION) {
	rr = Ray(normalize(refract(r.m_direction, n, r.m_ref_index / _f->m_mat.m_ref_index)), i_pos - n * BIAS, _f->m_mat.m_ref_index);
	color += (1.0f - _f->m_mat.m_rho) * (1.0f - kr) * trace_ray(rr, v_figures, v_lights, rec_level + 1);
      } else if (rec_level >= MAX_RECURSION)
	return vec3(0.0f);

    }

    // Return final color.
    return clamp(color, 0.0f, 1.0f);

  } else
    return vec3(BCKG_COLOR);
}

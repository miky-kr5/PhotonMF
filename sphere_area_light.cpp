#include "sphere_area_light.hpp"

vec3 SphereAreaLight::sample_at_surface() {
  Sphere * s = static_cast<Sphere *>(m_figure);
  m_last_sample = m_figure->sample_at_surface();
  m_n_at_last_sample = normalize(vec3((m_last_sample - s->m_center) / s->m_radius));
  return m_last_sample;
}

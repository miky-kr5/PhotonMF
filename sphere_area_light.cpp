#include "sphere_area_light.hpp"

void SphereAreaLight::sample_at_surface(vec3 point) {
  Sphere * s = static_cast<Sphere *>(m_figure);
  m_last_sample = m_figure->sample_at_surface();
  m_n_at_last_sample = normalize(vec3((m_last_sample - s->m_center) / s->m_radius));
}

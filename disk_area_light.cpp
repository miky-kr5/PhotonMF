#include "disk_area_light.hpp"

vec3 DiskAreaLight::sample_at_surface() {
  Disk * d = static_cast<Disk *>(m_figure);
  m_last_sample = m_figure->sample_at_surface();
  m_n_at_last_sample = d->m_normal;
  return m_last_sample;
}

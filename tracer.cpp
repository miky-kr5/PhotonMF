#include <cstdlib>

#include <glm/gtc/constants.hpp>

#include "tracer.hpp"

using namespace glm;

const vec3 BCKG_COLOR = vec3(1.0f);

float Tracer::random01() const {
  return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}

float Tracer::fresnel(const vec3 & i, const vec3 & n, const float ir1, const float ir2) const {
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

/* Helper functions pretty much taken from scratchapixel.com */
void Tracer::create_coords_system(const vec3 &n, vec3 &nt, vec3 &nb) const {
  if (abs(n.x) > abs(n.y))
    nt = normalize(vec3(n.z, 0.0f, -n.x));
  else
    nt = normalize(vec3(0.0f, -n.z, n.y));
  nb = normalize(cross(n, nt));
}

vec3 Tracer::sample_hemisphere(const float r1, const float r2) const {
  float sin_t = sqrt(1.0f - (r1 * r1));
  float phi = 2 * pi<float>() * r2;
  float x = sin_t * cos(phi);
  float z = sin_t * sin(phi);
  return vec3(x, r1, z);
}

void Tracer::rotate_sample(vec3 & sample, const vec3 & n) const {
  vec3 nt, nb;
  mat3 rot_m;

  create_coords_system(n, nt, nb);
  sample = vec3(sample.x * nb.x + sample.y * n.x + sample.z * nt.x,
		sample.x * nb.y + sample.y * n.y + sample.z * nt.y,
		sample.x * nb.z + sample.y * n.z + sample.z * nt.z);
}

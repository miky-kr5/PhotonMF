#include "tracer.hpp"

using glm::dot;
using glm::normalize;
using glm::refract;

const float BIAS = 0.000001f;

const vec3 BCKG_COLOR = vec3(0.1f);

float Tracer::fresnel(const vec3 & i, const vec3 & n, const float ir1, const float ir2) const {
  float cos_t1 = dot(i, n);
  float cos_t2 = dot(normalize(refract(i, n, ir1 / ir2)), n);
  float sin_t2 = (ir1 / ir2) * glm::sqrt(1.0f - (cos_t2 * cos_t2));

  if (sin_t2 >= 1.0f)
    return 1.0f;

  float fr_par = ((ir2 * cos_t1) - (ir1 * cos_t2)) / ((ir2 * cos_t1) + (ir1 * cos_t2));
  float fr_per = ((ir1 * cos_t2) - (ir2 * cos_t1)) / ((ir1 * cos_t2) + (ir2 * cos_t1));

  return ((fr_par * fr_par) + (fr_per * fr_per)) / 2.0f;
}

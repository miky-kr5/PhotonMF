#include <cstdlib>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "sampling.hpp"

using glm::mat3;
using glm::abs;
using glm::normalize;
using glm::cross;
using glm::radians;
using glm::pi;

const float PDF = (1.0f / (2.0f * pi<float>()));

float random01() {
  return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}

vec2 sample_pixel(int i, int j, float w, float h, float a_ratio, float fov) {
  float pxNDC;
  float pyNDC;
  float pxS;
  float pyS;
  pyNDC = (static_cast<float>(i) + random01()) / h;
  pyS = (1.0f - (2.0f * pyNDC)) * glm::tan(radians(fov / 2.0f));
  pxNDC = (static_cast<float>(j) + random01()) / w;
  pxS = (2.0f * pxNDC) - 1.0f;
  pxS *= a_ratio * glm::tan(radians(fov / 2.0f));

  return vec2(pxS, pyS);
}

/* Sampling functions pretty much taken from scratchapixel.com */
void create_coords_system(const vec3 &n, vec3 &nt, vec3 &nb) {
  if (abs(n.x) > abs(n.y))
    nt = normalize(vec3(n.z, 0.0f, -n.x));
  else
    nt = normalize(vec3(0.0f, -n.z, n.y));
  nb = normalize(cross(n, nt));
}

vec3 sample_hemisphere(const float r1, float r2) {
  float sin_t = glm::sqrt(1.0f - (r1 * r1));
  float phi = 2 * pi<float>() * r2;
  float x = sin_t * glm::cos(phi);
  float z = sin_t * glm::sin(phi);
  return vec3(x, r1, z);
}

void rotate_sample(vec3 & sample, vec3 & n) {
  vec3 nt, nb;
  mat3 rot_m;

  create_coords_system(n, nt, nb);
  sample = vec3(sample.x * nb.x + sample.y * n.x + sample.z * nt.x,
		sample.x * nb.y + sample.y * n.y + sample.z * nt.y,
		sample.x * nb.z + sample.y * n.z + sample.z * nt.z);
}

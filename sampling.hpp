#pragma once
#ifndef SAMPLING_HPP
#define SAMPLING_HPP

#include <glm/vec3.hpp>

using glm::vec3;

extern const float PDF;

extern float random01();
extern void create_coords_system(const vec3 &n, vec3 &nt, vec3 &nb);
extern vec3 sample_hemisphere(const float r1, float r2);
extern void rotate_sample(vec3 & sample, vec3 & n);

#endif

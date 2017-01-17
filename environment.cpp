#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "environment.hpp"

using glm::vec2;
using glm::acos;
using glm::pi;

vec3 Environment::get_color(Ray & r) {
  float _r;
  vec2 tex_coord;
  BYTE * bits;
  FIRGBF * pixel;
  unsigned int pitch;

  if (m_texture == NULL)
    return m_bckg_color;
  else {
    if (!m_probe) {
      tex_coord = vec2((1.0f + atan2(r.m_direction.x, -r.m_direction.z) / pi<float>()) / 2.0f, acos(r.m_direction.y) / pi<float>());
      tex_coord = vec2(tex_coord.x, 1.0f - tex_coord.y);
    } else {
      _r = (1.0f / pi<float>()) * acos(r.m_direction.z) / glm::sqrt((r.m_direction.x * r.m_direction.x) + (r.m_direction.y * r.m_direction.y));
      tex_coord = vec2(r.m_direction.x * _r, r.m_direction.y * _r);
      tex_coord += vec2(1.0f, 1.0f);
      tex_coord /= 2.0f;
    }

    tex_coord *= vec2(FreeImage_GetWidth(m_texture) - 1, FreeImage_GetHeight(m_texture) - 1);
    pitch = FreeImage_GetPitch(m_texture);
    bits = ((BYTE *)FreeImage_GetBits(m_texture)) + (static_cast<unsigned int>(tex_coord.y) * pitch);
    pixel = (FIRGBF *)bits;
    return vec3(pixel[static_cast<unsigned int>(tex_coord.x)].red, pixel[static_cast<unsigned int>(tex_coord.x)].green, pixel[static_cast<unsigned int>(tex_coord.x)].blue);
  }
}

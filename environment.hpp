#pragma once
#ifndef ENVIRONMENT_HPP
#define ENVIRONMENT_HPP

#include <FreeImage.h>
#include <glm/vec3.hpp>

#include "ray.hpp"

using glm::vec3;

class Environment {
public:
  Environment(const char * tex_file = NULL, bool light_probe = false, vec3 bckg = vec3(1.0f)): m_bckg_color(bckg), m_probe(light_probe) {
    FREE_IMAGE_FORMAT fif;

    if (tex_file != NULL) {
      fif = FreeImage_GetFIFFromFilename(tex_file);
      m_texture = FreeImage_Load(fif, tex_file, 0);
    } else
      m_texture = NULL;
  }

  ~Environment() {
    if (m_texture != NULL)
      FreeImage_Unload(m_texture);
  }

  vec3 get_color(Ray & r);

private:
  vec3 m_bckg_color;
  FIBITMAP * m_texture;
  bool m_probe;
};

#endif

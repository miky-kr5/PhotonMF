#pragma once
#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "ray.hpp"

using glm::mat4;
using glm::vec2;
using glm::vec3;
using glm::normalize;
using glm::lookAt;
using glm::inverse;

class Camera {
public:
  vec3 m_eye;
  vec3 m_look;
  vec3 m_up;

  Camera(vec3 _e = vec3(0.0f), vec3 _l = vec3(0.0f, 0.0f, -1.0f), vec3 _u = vec3(0.0f, 1.0f, 0.0f)):
    m_eye(_e),
    m_look(_l),
    m_up(normalize(_u)),
    m_inv_view_matrix(inverse(lookAt(_e, _l, normalize(_u))))
  { }

  void reset();
  void translate(vec3 t);
  void pitch(float angle);
  void yaw(float angle);
  void roll(float angle);
  void view_to_world(Ray & r) const;

private:
  mat4 m_inv_view_matrix;
};

#endif

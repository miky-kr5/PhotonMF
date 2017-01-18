#include <glm/gtx/rotate_vector.hpp>

#include "camera.hpp"

using glm::vec4;
using glm::rotate;
using glm::radians;
using glm::cross;

void Camera::reset() {
  m_inv_view_matrix = inverse(lookAt(m_eye, m_look, m_up));
}

void Camera::translate(vec3 t) {
  mat4 t_matrix = glm::translate(mat4(1.0f), t);
  vec4 new_eye = t_matrix * vec4(m_eye, 1.0f);
  vec4 new_look = t_matrix * vec4(m_look, 1.0f);
  m_eye = vec3(new_eye.x, new_eye.y, new_eye.z);
  m_look = vec3(new_look.x, new_look.y, new_look.z);
  reset();
}

void Camera::pitch(float angle) {
  vec3 view_dir = normalize(m_look - m_eye);
  vec3 left = cross(view_dir, m_up);
  view_dir = rotate(view_dir, radians(angle), left);
  m_up = normalize(rotate(m_up, radians(angle), left));
  m_look = m_eye + view_dir;
  reset();
}

void Camera::yaw(float angle) {
  vec3 view_dir = rotate(normalize(m_look - m_eye), radians(angle), m_up);
  m_look = m_eye + view_dir;
  reset();
}

void Camera::roll(float angle) {
  m_up = normalize(rotate(m_up, radians(angle), normalize(m_look - m_eye)));
  reset();
}

void Camera::view_to_world(Ray & r) const {
  vec4 dir = m_inv_view_matrix * vec4(r.m_direction, 0.0f);
  vec4 orig = m_inv_view_matrix * vec4(r.m_origin, 1.0f);

  r.m_direction = vec3(dir.x, dir.y, dir.z);
  r.m_origin = vec3(orig.x, orig.y, orig.z);
}

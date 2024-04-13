#include "camera.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <format>
#include "defines.hpp"

std::shared_ptr<Camera> Camera::create(f32 aspect_ratio, f32 fov, f32 near, f32 far) {
    return std::make_shared<Camera>(aspect_ratio, fov, near, far);
}

Camera::Camera(f32 aspect_ratio, f32 fov, f32 near, f32 far)
    : m_aspect_ratio(aspect_ratio), m_fov(fov), m_near(near), m_far(far) {

    m_position = glm::vec3(0.0f, 0.0f, 0.0f);
    m_front = glm::vec3(0.0f, 0.0f, -1.0f);
    m_up = glm::vec3(0.0f, 1.0f, 0.0f);
    m_right = glm::vec3(1.0f, 0.0f, 0.0f);
    m_world_up = glm::vec3(0.0f, 1.0f, 0.0f);

    m_yaw = -90.0f;
    m_pitch = 0.0f;

    m_speed = 2.5f;
    m_sensitivity = 15.0f;

    update_vectors();
}

Camera::~Camera() {}

void Camera::move(Direction direction, f32 delta_time, f32 speed) {
    f32 velocity = speed * m_speed * delta_time;

    switch (direction) {
    case FORWARD:
        m_position += m_front * velocity;
        break;
    case BACKWARD:
        m_position -= m_front * velocity;
        break;
    case LEFT:
        m_position -= m_right * velocity;
        break;
    case RIGHT:
        m_position += m_right * velocity;
        break;
    case UP:
        m_position += m_world_up * velocity;
        break;
    case DOWN:
        m_position -= m_world_up * velocity;
        break;
    }
}

void Camera::rotate(f32 x_offset, f32 y_offset, f32 delta_time) {
    x_offset *= m_sensitivity * delta_time;
    y_offset *= m_sensitivity * delta_time;

    m_yaw += x_offset;
    m_pitch += y_offset;

    if (m_pitch > 89.0f) {
        m_pitch = 89.0f;
    }
    if (m_pitch < -89.0f) {
        m_pitch = -89.0f;
    }

    update_vectors();
}

void Camera::set_position(const glm::vec3& position) {
    m_position = position;
}

glm::vec3 Camera::get_position() const {
    return m_position;
}

void Camera::update_vectors() {
    glm::vec3 front = glm::vec3(0.0f);
    front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    front.y = sin(glm::radians(m_pitch));
    front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));

    m_front = glm::normalize(front);
    m_right = glm::normalize(glm::cross(m_front, m_world_up));
    m_up = glm::normalize(glm::cross(m_right, m_front));
}

glm::mat4 Camera::get_view_matrix() const {
    return glm::lookAt(m_position, m_position + m_front, m_up);
}

glm::mat4 Camera::get_projection_matrix() const {
    return glm::perspective(glm::radians(m_fov), m_aspect_ratio, m_near, m_far);
}
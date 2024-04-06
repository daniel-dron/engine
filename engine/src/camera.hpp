#pragma once

#include <memory> 
#include <glm/glm/glm.hpp>
#include <defines.hpp>

class KAPI Camera {
public:
    enum Direction { NONE, FORWARD, BACKWARD, LEFT, RIGHT, UP, DOWN };

    static std::shared_ptr<Camera> create(f32 aspect_ratio, f32 fov, f32 near, f32 far);
    Camera(f32 aspect_ratio, f32 fov, f32 near, f32 far);
    ~Camera();

    void move(Direction direction, f32 delta_time, f32 speed = 1.0f);
    void rotate(f32 x_offset, f32 y_offset, f32 delta_time);

    void set_position(const glm::vec3& position);
    glm::vec3 get_position() const;

    glm::mat4 get_view_matrix() const;
    glm::mat4 get_projection_matrix() const;

private:
    glm::vec3 m_position;
    glm::vec3 m_front;
    glm::vec3 m_up;
    glm::vec3 m_right;
    glm::vec3 m_world_up;

    f32 m_yaw;
    f32 m_pitch;

    f32 m_aspect_ratio;
    f32 m_fov;
    f32 m_near;
    f32 m_far;

    f32 m_speed;
    f32 m_sensitivity;

    void update_vectors();
};
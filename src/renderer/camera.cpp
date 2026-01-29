#include "../renderer/camera.h"

// 构造函数 (向量版)
Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : front(glm::vec3(0.0f, 0.0f, -1.0f)),
      movement_speed(SPEED),
      mouse_sensitivity(SENSITIVITY),
      zoom(ZOOM),
      near_plane(0.1f),  // 默认近平面
      far_plane(100.0f)  // 默认远平面
{
    this->position = position;
    this->world_up = up;
    this->yaw = yaw;
    this->pitch = pitch;
    update_camera_vectors();
}

// 构造函数 (标量版)
Camera::Camera(float pos_x, float pos_y, float pos_z, float up_x, float up_y, float up_z, float yaw, float pitch)
    : front(glm::vec3(0.0f, 0.0f, -1.0f)),
      movement_speed(SPEED),
      mouse_sensitivity(SENSITIVITY),
      zoom(ZOOM),
      near_plane(0.1f),
      far_plane(100.0f)
{
    this->position = glm::vec3(pos_x, pos_y, pos_z);
    this->world_up = glm::vec3(up_x, up_y, up_z);
    this->yaw = yaw;
    this->pitch = pitch;
    update_camera_vectors();
}

// 获取 View 矩阵
glm::mat4 Camera::get_view_matrix() const
{
    // lookAt(eye, center, up)
    return glm::lookAt(position, position + front, up);
}

// 获取 Projection 矩阵 (新增功能)
glm::mat4 Camera::get_projection_matrix(float width, float height) const
{
    // 防止除以0错误
    if (height == 0) height = 1;

    // glm::perspective(fov_in_radians, aspect_ratio, near, far)
    return glm::perspective(glm::radians(zoom), width / height, near_plane, far_plane);
}

// 处理键盘移动
void Camera::process_keyboard(camera_movement direction, float delta_time)
{
    float velocity = movement_speed * delta_time;

    if (direction == camera_movement::FORWARD)
        position += front * velocity;
    if (direction == camera_movement::BACKWARD)
        position -= front * velocity;
    if (direction == camera_movement::LEFT)
        position -= right * velocity;
    if (direction == camera_movement::RIGHT)
        position += right * velocity;

    // 类似“飞行模式”，如果是FPS模式（不能飞），通常会忽略Up/Down并把y轴锁定
    if (direction == camera_movement::UPWARD)
        position += world_up * velocity;
    if (direction == camera_movement::DOWNWARD)
        position -= world_up * velocity;
}

// 处理鼠标视角
void Camera::process_mouse_movement(float xoffset, float yoffset, bool constrain_pitch)
{
    xoffset *= mouse_sensitivity;
    yoffset *= mouse_sensitivity;

    yaw   += xoffset;
    pitch += yoffset;

    // 限制俯仰角，防止万向节死锁或视角颠倒
    if (constrain_pitch)
    {
        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;
    }

    // 角度改变后，必须重新计算方向向量
    update_camera_vectors();
}

// 处理滚轮缩放
void Camera::process_mouse_scroll(float yoffset)
{
    zoom -= (float)yoffset;
    if (zoom < 1.0f)
        zoom = 1.0f;
    if (zoom > 45.0f)
        zoom = 45.0f;
}

// 更新内部向量
void Camera::update_camera_vectors()
{
    // 计算新的 Front 向量
    glm::vec3 new_front;
    new_front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    new_front.y = sin(glm::radians(pitch));
    new_front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(new_front);

    // 重新计算 Right 和 Up
    // Normalize 也是必须的，因为叉乘结果长度可能不为1
    right = glm::normalize(glm::cross(front, world_up));
    up    = glm::normalize(glm::cross(right, front));
}
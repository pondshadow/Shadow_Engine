#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>        // glm::lookAt, glm::perspective, glm::radians 等变换相关函数

// 定义摄像机移动方向的枚举
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UPWARD,
    DOWNWARD
};

// 默认摄像机参数（常量）
const float YAW         = -90.0f;   // 初始偏航角（度）。-90 的原因：在右手坐标系中，默认朝 -Z 方向看
const float PITCH       =  0.0f;    // 初始俯仰角（度）
const float SPEED       =  2.5f;    // 默认移动速度（单位：单位/秒，和 deltaTime 相乘）
const float SENSITIVITY =  0.05f;    // 默认鼠标灵敏度
const float ZOOM        =  45.0f;   // 默认 FOV（视野）

// 一个抽象的摄像机类：处理输入并计算欧拉角、方向向量和视图矩阵，供 OpenGL 使用
class Camera
{
public:
    // --- 摄像机属性---
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    // 欧拉角（角度制）
    float Yaw;
    float Pitch;

    // 摄像机选项
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    // ---------- 构造函数（向量形式） ----------
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
           float yaw = YAW,
           float pitch = PITCH)
        : Front(glm::vec3(0.0f, 0.0f, -1.0f)),    // 默认 Front 指向 -Z（OpenGL 习惯）
          MovementSpeed(SPEED),
          MouseSensitivity(SENSITIVITY),
          Zoom(ZOOM)
    {
        Position = position;  // 摄像机位置
        WorldUp = up;         // 世界 up（通常不变）
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors(); // 根据初始欧拉角计算 Front/Right/Up
    }

    // ---------- 构造函数（标量形式） ----------
    Camera(float posX, float posY, float posZ,
           float upX, float upY, float upZ,
           float yaw, float pitch)
        : Front(glm::vec3(0.0f, 0.0f, -1.0f)),
          MovementSpeed(SPEED),
          MouseSensitivity(SENSITIVITY),
          Zoom(ZOOM)
    {
        Position = glm::vec3(posX, posY, posZ);
        WorldUp = glm::vec3(upX, upY, upZ);
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    // 返回视图矩阵（使用 glm::lookAt）
    // lookAt(eye, center, up) ：eye = 摄像机位置， center = 目标位置（Position + Front）， up = 摄像机的上向量
    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(Position, Position + Front, Up);
    }

    // 处理键盘输入 (或任何“匀速”/离散的方向输入)，direction 使用上面的枚举，deltaTime 保证与帧率无关
    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {
        float velocity = MovementSpeed * deltaTime; // 单位：单位/秒 * 秒 = 单位（位移）
        if (direction == FORWARD)
            Position += Front * velocity;   // 沿 Front 前进
        if (direction == BACKWARD)
            Position -= Front * velocity;   // 沿 Front 后退
        if (direction == LEFT)
            Position -= Right * velocity;   // 左移（沿 Right 的负方向）
        if (direction == RIGHT)
            Position += Right * velocity;   // 右移

        if (direction == UPWARD)
            Position += WorldUp * velocity;
        if (direction == DOWNWARD)
            Position -= WorldUp * velocity;
        // 注意：这里没有对 Position 做额外约束（例如地面高度或碰撞），需要用户自己实现
    }

    // 处理鼠标位置偏移（xoffset, yoffset 是像素或相对偏移）
    // constrainPitch 用于是否限制俯仰角，避免“翻转”现象（即上下看 90° 导致方向反转）
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
    {
        xoffset *= MouseSensitivity; // 按灵敏度缩放偏移
        yoffset *= MouseSensitivity;

        Yaw   += xoffset; // 偏航（左右移动鼠标改变 Yaw）
        Pitch += yoffset; // 俯仰（上下移动鼠标改变 Pitch）

        // 限制 Pitch 的范围，避免当 Pitch 接近 ±90° 时出现翻转或数值不稳定（万向节锁问题）
        if (constrainPitch)
        {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }

        // 更新方向向量（Front, Right, Up）
        updateCameraVectors();
    }

    // 处理鼠标滚轮滚动（通常用于缩放视野，即改变 FOV）
    void ProcessMouseScroll(float yoffset)
    {
        Zoom -= (float)yoffset;  // 往前滚动通常是负值或正值，取决于回调约定；这里按减法处理
        if (Zoom < 1.0f)         // 限制最小 FOV，避免过度放大
            Zoom = 1.0f;
        if (Zoom > 45.0f)        // 限制最大 FOV（这里用 45°，与默认 ZOOM 保持一致）
            Zoom = 45.0f;
    }

private:
    // 根据当前的欧拉角（Yaw, Pitch）计算 Front、Right、Up 三个单位向量（在世界空间中）
    void updateCameraVectors()
    {
        // 根据欧拉角计算新的前向向量（非归一化）
        // 注意：glm::radians 将角度变成弧度，C++ 三角函数接受弧度。
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front); // 归一化，确保速度计算正确（单位向量）

        // 重新计算 Right 与 Up（使用叉乘）
        // Right = normalize(cross(Front, WorldUp))
        // 注意叉乘的顺序：cross(a, b) 给出一个垂直于 a 和 b 的向量，遵循右手法则
        // 这里用 Front, WorldUp 的顺序是为了确保 Right 指向相机的“右边”。
        Right = glm::normalize(glm::cross(Front, WorldUp));

        // Up = normalize(cross(Right, Front))
        // 这样得到的 Up 与 WorldUp 一致方向但已纠正以保持与 Front 正交（数值稳定）
        Up = glm::normalize(glm::cross(Right, Front));
    }
};
#endif

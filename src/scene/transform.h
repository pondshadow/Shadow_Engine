#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

class Transform
{
public:
    // 位置、旋转、缩放
    glm::vec3 position;
    glm::vec3 rotation; // 存储欧拉角（角度制，Degrees），分别对应 Pitch, Yaw, Roll
    glm::vec3 scale;

    // 构造函数
    Transform();

    // 核心功能：获取模型矩阵
    // 矩阵乘法顺序通常是：Translate * Rotate * Scale (T * R * S)
    glm::mat4 get_model_matrix() const;

    // 辅助函数：重置状态
    void reset();
};
#pragma once
#include <glm/glm.hpp> // 需要 GLM 类型

// 定向光参数
struct DirLightParams {
    bool enable = true;
    glm::vec3 direction = glm::vec3(-0.2f, -1.0f, -0.3f); // 使用 glm::vec3 方便 ImGui 绑定
    glm::vec3 color     = glm::vec3(1.0f, 1.0f, 1.0f);
};

// 点光源参数
struct PointLightParams {
    bool enable = true;
    glm::vec3 color     = glm::vec3(1.0f, 1.0f, 1.0f);
    float constant = 1.0f;
    float linear   = 0.09f;
    float quadratic = 0.032f;
};

// 聚光灯参数
struct SpotLightParams {
    bool enable = true;
    glm::vec3 color     = glm::vec3(1.0f, 1.0f, 1.0f);
    float cut_off = 12.5f;
    float outer_cut_off = 17.5f;
    float constant = 1.0f;
    float linear   = 0.09f;
    float quadratic = 0.032f;
};
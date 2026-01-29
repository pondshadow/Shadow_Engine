#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include "shader.h" // 引用你之前的 Shader 类

// 定义顶点的标准格式
// 这种结构体在内存中是紧凑排列的：PX,PY,PZ, NX,NY,NZ, U,V
// 这与 OpenGL 的缓冲布局完美对应
struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

// 用于 Mesh 内部记录纹理信息的轻量级结构
struct TextureInfo {
    unsigned int id;   // OpenGL 纹理 ID
    std::string type;  // 类型: "texture_diffuse" 或 "texture_specular"
    std::string path;  // (可选) 用于防止重复加载，Assimp 模型加载时会用到
};

class Mesh {
public:
    // 网格数据
    std::vector<Vertex>       vertices;
    std::vector<unsigned int> indices;
    std::vector<TextureInfo>  textures;

    // 构造函数
    // 灵活支持有索引(模型)和无索引(手写顶点)的情况
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<TextureInfo> textures);

    // 绘制函数
    void Draw(Shader& shader);

private:
    // 渲染数据对象
    unsigned int VAO, VBO, EBO;

    // 初始化缓冲区对象
    void setupMesh();
};
#include "mesh.h"

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<TextureInfo> textures)
{
    this->vertices = vertices;
    this->indices = indices;
    this->textures = textures;

    // 创建 Mesh 后立即建立缓冲区
    setupMesh();
}

void Mesh::setupMesh()
{
    // 生成缓冲对象 ID
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    // 如果有索引数据，才生成 EBO
    if (!indices.empty()) {
        glGenBuffers(1, &EBO);
    }

    glBindVertexArray(VAO);

    // 绑定并填充 VBO
    // 这里的关键是 &vertices[0]，直接获取 vector 内部数组的指针
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    // 绑定并填充 EBO (如果存在)
    if (!indices.empty()) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
    }

    // 设置顶点属性指针
    // 这里的 offsetof(Struct, Member) 是 C++ 的宏，能自动计算成员变量在结构体内的字节偏移量
    // 极其方便，不用手动算 float 大小了

    // 位置 Position (Location = 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    // 法线 Normal (Location = 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

    // 纹理坐标 TexCoords (Location = 2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

    // 解绑 VAO 防止意外修改
    glBindVertexArray(0);
}

void Mesh::Draw(Shader& shader)
{
    // 绑定纹理
    // 这里的逻辑是为了应对 Shader 中可能有多个漫反射/镜面光贴图的情况
    // 命名约定：material.texture_diffuse1, material.texture_specular1, ...
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;

    for (unsigned int i = 0; i < textures.size(); i++)
    {
        // 激活对应的纹理单元
        glActiveTexture(GL_TEXTURE0 + i);

        std::string number;
        std::string name = textures[i].type;

        if (name == "texture_diffuse")
            number = std::to_string(diffuseNr++);
        else if (name == "texture_specular")
            number = std::to_string(specularNr++);

        // 设置 Shader 中的采样器 Uniform
        // 最终拼接出类似 "material.texture_diffuse1" 的字符串
        shader.setInt(("material." + name + number).c_str(), i);

        // 绑定纹理 ID
        glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }

    // 绘制网格
    glBindVertexArray(VAO);

    if (!indices.empty()) {
        // 如果有索引，使用 glDrawElements (通常用于 Assimp 加载的模型)
        glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
    } else {
        // 如果没有索引，使用 glDrawArrays (通常用于你的手写顶点)
        glDrawArrays(GL_TRIANGLES, 0, static_cast<unsigned int>(vertices.size()));
    }

    glBindVertexArray(0);

    // 恢复默认激活纹理单元，是个好习惯
    glActiveTexture(GL_TEXTURE0);
}
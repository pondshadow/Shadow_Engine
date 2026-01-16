#ifndef MESH_H
#define MESH_H

#include <glad/glad.h> // 包含所有 OpenGL 类型声明

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.h"

#include <string>
#include <vector>
using namespace std;

#define MAX_BONE_INFLUENCE 4

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;
    glm::vec3 Bitangent; 
    // 骨骼索引总是从 0 开始，所以我们可以使用 0 作为默认值
    int m_BoneIDs[MAX_BONE_INFLUENCE]; 
    float m_BoneWeights[MAX_BONE_INFLUENCE];
};

struct Texture {
    unsigned int id;
    std::string type;
    std::string path;
};

class Mesh {
public:
    // 网格数据
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;
    unsigned int VAO;

    Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures)
    {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;

        // 现在我们有了所有需要的数据，设置顶点缓冲及其属性指针。
        setupMesh();
    }
    
    void Draw(Shader& shader)
    {
        // 绑定合适的纹理
        unsigned int diffuseNr  = 1;
        unsigned int specularNr = 1;
        unsigned int normalNr   = 1;
        unsigned int heightNr   = 1;
        for(unsigned int i = 0; i < textures.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i); // 在绑定之前激活合适的纹理单元
            // 获取纹理编号（diffuse_textureN 中的 N）
            string number;
            string name = textures[i].type;
            if(name == "texture_diffuse")
                number = std::to_string(diffuseNr++);
            else if(name == "texture_specular")
                number = std::to_string(specularNr++); // 将 unsigned int 转换为流
            else if(name == "texture_normal")
                number = std::to_string(normalNr++); // 将 unsigned int 转换为流
            else if(name == "texture_height")
                number = std::to_string(heightNr++); // 将 unsigned int 转换为流

            // 现在将采样器设置到正确的纹理单元
            glUniform1i(glGetUniformLocation(shader.ID, (name + number).c_str()), i);
            // 最后绑定纹理
            glBindTexture(GL_TEXTURE_2D, textures[i].id);

        }
        // 绘制网格
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        
        glActiveTexture(GL_TEXTURE0);
    }
private:
    // 渲染数据
    unsigned int VBO, EBO;

    // 初始化所有缓冲对象/数组
    void setupMesh()
    {
        // 创建缓冲/数组
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        // 将数据加载到顶点缓冲
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        // 结构体的一个优点是其内存布局对于所有项都是连续的。
        // 结果是我们可以简单地传递一个指向结构体的指针，它完美地转换为 glm::vec3/2 数组，
        // 进而转换为 float 数组。
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);  

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        // 设置顶点属性指针
        // 顶点位置
        glEnableVertexAttribArray(0);	
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        // 顶点法线
        glEnableVertexAttribArray(1);	
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
        // 顶点纹理坐标
        glEnableVertexAttribArray(2);	
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
        // 顶点切线
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
        // 顶点副切线
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));
        // 骨骼 ID
        glEnableVertexAttribArray(5);
        glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, m_BoneIDs));
        // 骨骼权重
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_BoneWeights));
        glBindVertexArray(0);
    }
};

#endif

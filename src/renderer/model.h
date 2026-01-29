#pragma once

#include <vector>
#include <string>
#include <iostream>

// 引入 Assimp 库头文件
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// 引入你自己的 Mesh 和 Shader 类
#include "mesh.h"
#include "shader.h"

// Model 类：负责加载外部 3D 模型文件（如 .obj, .fbx）
// 它包含一个 Mesh 对象的数组，因为一个复杂的模型通常由多个子网格组成
class Model
{
public:
    // 存储模型包含的所有网格
    std::vector<Mesh> meshes;

    // 纹理缓存：防止同一个纹理被重复加载
    // 这里的 TextureInfo 是我们在 mesh.h 中定义的结构体
    std::vector<TextureInfo> textures_loaded;

    // 模型文件所在的目录路径（用于加载相对路径的纹理）
    std::string directory;

    // 构造函数：传入文件路径即可加载
    // gamma 参数用于伽马校正，目前我们暂时默认为 false
    Model(std::string const &path, bool gamma = false);

    // 绘制函数：遍历所有网格并调用它们的 Draw
    void Draw(Shader &shader);

private:
    // --- 内部处理函数 ---

    // 加载模型的入口函数
    void loadModel(std::string const &path);

    // 递归处理 Assimp 的节点树
    // Assimp 将模型加载为节点树结构，我们需要递归遍历每个节点来获取 Mesh
    void processNode(aiNode *node, const aiScene *scene);

    // 将 Assimp 的 aiMesh 数据转换为我们需要 Mesh 类数据
    Mesh processMesh(aiMesh *mesh, const aiScene *scene);

    // 加载材质纹理
    // 检查材质中是否有纹理，如果有则加载，并处理缓存逻辑
    std::vector<TextureInfo> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName);
};
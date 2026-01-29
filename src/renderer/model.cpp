#include "model.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <stb_image.h> // 需要直接使用 stb_image 加载纹理数据

// 一个辅助函数：直接从文件加载纹理并返回 OpenGL ID
// 这与你 core/texture.cpp 的逻辑类似，但这里作为内部工具函数使用
unsigned int TextureFromFile(const char *path, const std::string &directory)
{
    std::string filename = std::string(path);
    filename = directory + '/' + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    // 加载纹理数据
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // 设置纹理环绕和过滤方式
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

// 构造函数实现
Model::Model(std::string const &path, bool gamma)
{
    loadModel(path);
}

// 绘制函数实现
void Model::Draw(Shader &shader)
{
    // 简单地遍历所有子网格并绘制
    for(unsigned int i = 0; i < meshes.size(); i++)
        meshes[i].Draw(shader);
}

// 加载模型主逻辑
void Model::loadModel(std::string const &path)
{
    // 使用 Assimp 导入器读取文件
    Assimp::Importer importer;
    // aiProcess_Triangulate: 如果模型有四边形面，自动转换成三角形
    // aiProcess_FlipUVs: 翻转 Y 轴 UV（OpenGL 需要）
    // aiProcess_GenSmoothNormals: 如果模型没有法线，自动生成平滑法线
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

    // 检查错误
    // 如果 scene 为空，或者标志位不完整，或者根节点为空，说明加载失败
    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
        return;
    }

    // 以此路径为基准，提取目录路径（用于之后加载同目录下的纹理文件）
    directory = path.substr(0, path.find_last_of('/'));

    // 开始递归处理根节点
    processNode(scene->mRootNode, scene);
}

// 递归处理节点
void Model::processNode(aiNode *node, const aiScene *scene)
{
    // 处理当前节点下的所有网格
    for(unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        // 节点中只存储了网格的索引，真正的数据在 scene->mMeshes 中
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }

    // 递归处理子节点
    for(unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }
}

// 将 Assimp 网格数据转换为我们的 Mesh 数据
Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene)
{
    // 准备数据容器
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<TextureInfo> textures;

    // 处理顶点数据
    for(unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        glm::vec3 vector;

        // 位置
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.Position = vector;

        // 法线
        if (mesh->HasNormals())
        {
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.Normal = vector;
        }

        // 纹理坐标
        // Assimp 允许一个顶点最多有 8 套纹理坐标，我们只关心第一套 (0)
        if(mesh->mTextureCoords[0])
        {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.TexCoords = vec;

            // 注意：如果你的 Vertex 结构体还没有 Tangent 字段，请保持下面注释
            // 如果你想做法线贴图，需要在 mesh.h 的 Vertex 中添加 Tangent 和 Bitangent
            /*
            vector.x = mesh->mTangents[i].x;
            vector.y = mesh->mTangents[i].y;
            vector.z = mesh->mTangents[i].z;
            vertex.Tangent = vector;

            vector.x = mesh->mBitangents[i].x;
            vector.y = mesh->mBitangents[i].y;
            vector.z = mesh->mBitangents[i].z;
            vertex.Bitangent = vector;
            */
        }
        else
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);

        vertices.push_back(vertex);
    }

    // 处理索引 (EBO 数据)
    for(unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        // 提取面的所有索引
        for(unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    // 处理材质 (纹理)
    if(mesh->mMaterialIndex >= 0)
    {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        // 漫反射贴图 -> texture_diffuse
        std::vector<TextureInfo> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

        // 镜面光贴图 -> texture_specular
        std::vector<TextureInfo> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

        // 法线贴图 (通常 Assimp 中是 HEIGHT 类型，或者 NORMALS 类型，具体看模型格式)
        std::vector<TextureInfo> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
    }

    // 返回构建好的 Mesh 对象
    return Mesh(vertices, indices, textures);
}

// 加载材质纹理
std::vector<TextureInfo> Model::loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName)
{
    std::vector<TextureInfo> textures;

    // 遍历该类型的所有纹理
    for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);

        // 检查是否已经加载过该纹理（优化）
        bool skip = false;
        for(unsigned int j = 0; j < textures_loaded.size(); j++)
        {
            if(std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
            {
                textures.push_back(textures_loaded[j]);
                skip = true;
                break;
            }
        }

        // 如果没加载过，则加载
        if(!skip)
        {
            TextureInfo texture;
            // 调用辅助函数加载图片文件
            texture.id = TextureFromFile(str.C_Str(), this->directory);
            texture.type = typeName;
            texture.path = str.C_Str(); // 保存路径用于下次对比

            textures.push_back(texture);
            textures_loaded.push_back(texture);  // 加入缓存
        }
    }
    return textures;
}
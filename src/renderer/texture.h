#pragma once

#include <glad/glad.h> // 需要 OpenGL 函数来管理纹理 ID
#include <string>

class Texture
{
public:
    unsigned int ID;      // OpenGL 分配的纹理 ID
    int width, height;    // 纹理的像素宽高
    int nrChannels;       // 颜色通道数 (RGB/RGBA)

    // 构造函数：传入文件路径，自动加载图片并生成纹理
    Texture(const char* path);

    // 析构函数：对象销毁时自动释放显存
    ~Texture();

    // 激活并绑定纹理到指定的纹理单元 (Slot)
    // slot = 0 对应 GL_TEXTURE0, slot = 1 对应 GL_TEXTURE1...
    void bind(unsigned int slot = 0) const;

    // 解绑当前纹理
    void unbind() const;
};
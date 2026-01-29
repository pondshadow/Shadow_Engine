#include "../renderer/texture.h"

#include <iostream>

// 这是一个预处理器宏，告诉 stb_image.h 在这里“实现”它的函数代码
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

Texture::Texture(const char* path)
{
    // 生成纹理 ID
    glGenTextures(1, &ID);

    // 加载图片数据
    unsigned char *data = nullptr;

    // 翻转 Y 轴：OpenGL 的纹理坐标原点在左下角，而大多数图片格式原点在左上角
    // 这是一个全局设置，放在这里确保加载前生效
    stbi_set_flip_vertically_on_load(true);

    // 使用 stbi_load 加载图片
    // &width, &height, &nrChannels 会被填充为图片的实际信息
    data = stbi_load(path, &width, &height, &nrChannels, 0);

    if (data)
    {
        GLenum format;
        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;

        // 绑定当前纹理 ID，后续的操作都会作用于它
        glBindTexture(GL_TEXTURE_2D, ID);

        // 将图片数据上传到 GPU
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

        // 自动生成多级渐远纹理 (Mipmap)
        glGenerateMipmap(GL_TEXTURE_2D);

        // 设置纹理环绕方式 (Wrap)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        // 设置纹理过滤方式 (Filter)
        // 缩小使用 Mipmap 线性插值，放大使用线性插值
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // 上传完成后，释放 CPU 端的图片内存
        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }
}

Texture::~Texture()
{
    // 当 Texture 对象销毁时，告诉 OpenGL 删除这个纹理 ID
    glDeleteTextures(1, &ID);
}

void Texture::bind(unsigned int slot) const
{
    // 激活对应的纹理单元 (例如 GL_TEXTURE0 + 1 = GL_TEXTURE1)
    glActiveTexture(GL_TEXTURE0 + slot);
    // 绑定纹理
    glBindTexture(GL_TEXTURE_2D, ID);
}

void Texture::unbind() const
{
    glBindTexture(GL_TEXTURE_2D, 0);
}
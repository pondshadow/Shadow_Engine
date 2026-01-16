#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>  // 包含OpenGL核心函数声明
#include <string>       // 字符串处理
#include <fstream>      // 文件输入流
#include <sstream>      // 字符串流
#include <iostream>     // 标准输入输出
#include <glm/glm.hpp>

class Shader
{
public:
    unsigned int ID;  // 着色器程序的OpenGL ID
    
    Shader(const char* vertexPath, const char* fragmentPath)
    {
        // 存储读取到的着色器代码（最终要交给OpenGL的内容）
        std::string vertexCode;
        std::string fragmentCode;
        // 文件输入流（用来“打开”和“读取”文件的工具）
        std::ifstream vShaderFile;
        std::ifstream fShaderFile;
        // 让文件流在出错时能“抛出异常”，方便我们捕获错误
        vShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        fShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        try {
            // 1. 打开文件：用文件流“关联”到磁盘上的文件
            vShaderFile.open(vertexPath);  // 打开顶点着色器文件（路径由参数传入）
            fShaderFile.open(fragmentPath);  // 打开片段着色器文件

            // 2. 创建字符串流：临时存储文件内容的“缓冲区”
            std::stringstream vShaderStream, fShaderStream;

            // 3. 读取文件内容到字符串流：把文件里的所有内容“灌”进字符串流
            vShaderStream << vShaderFile.rdbuf();  // rdbuf()返回文件的“读取缓冲区”，直接传给流
            fShaderStream << fShaderFile.rdbuf();

            // 4. 关闭文件：读取完成后，释放文件资源（避免占用）
            vShaderFile.close();
            fShaderFile.close();

            // 5. 转换为字符串：从字符串流中提取内容，存到我们定义的string变量里
            vertexCode   = vShaderStream.str();  // 顶点着色器代码→vertexCode
            fragmentCode = fShaderStream.str();  // 片段着色器代码→fragmentCode
        }
        catch (std::ifstream::failure& e) {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
        }
        const char* vShaderCode = vertexCode.c_str();
        const char * fShaderCode = fragmentCode.c_str();
        // 顶点着色器编译
        unsigned int vertex;
        vertex = glCreateShader(GL_VERTEX_SHADER);  // 创建顶点着色器对象
        glShaderSource(vertex, 1, &vShaderCode, NULL);  // 绑定着色器源码
        glCompileShader(vertex);  // 编译着色器
        checkCompileErrors(vertex, "VERTEX");  // 检查编译错误

        // 片段着色器编译（流程同上）
        unsigned int fragment;
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        checkCompileErrors(fragment, "FRAGMENT");

        ID = glCreateProgram();  // 创建着色器程序对象
        glAttachShader(ID, vertex);  // 附加顶点着色器
        glAttachShader(ID, fragment);  // 附加片段着色器
        glLinkProgram(ID);  // 链接程序
        checkCompileErrors(ID, "PROGRAM");  // 检查链接错误

        glDeleteShader(vertex);   // 删除顶点着色器（已链接到程序，不再需要）
        glDeleteShader(fragment); // 删除片段着色器
    }
    void use() { 
        glUseProgram(ID);  // 告诉OpenGL使用当前着色器程序
    }
        
    void setBool(const std::string &name, bool value) const
    {         
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); 
    }
    // ------------------------------------------------------------------------
    void setInt(const std::string &name, int value) const
    { 
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value); 
    }
    // ------------------------------------------------------------------------
    void setFloat(const std::string &name, float value) const
    { 
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value); 
    }
    // ------------------------------------------------------------------------
    void setVec2(const std::string &name, const glm::vec2 &value) const
    { 
        glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); 
    }
    void setVec2(const std::string &name, float x, float y) const
    { 
        glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y); 
    }
    // ------------------------------------------------------------------------
    void setVec3(const std::string &name, const glm::vec3 &value) const
    { 
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); 
    }
    void setVec3(const std::string &name, float x, float y, float z) const
    { 
        glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z); 
    }
    // ------------------------------------------------------------------------
    void setVec4(const std::string &name, const glm::vec4 &value) const
    { 
        glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); 
    }
    void setVec4(const std::string &name, float x, float y, float z, float w) const
    { 
        glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w); 
    }
    // ------------------------------------------------------------------------
    void setMat2(const std::string &name, const glm::mat2 &mat) const
    {
        glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    // ------------------------------------------------------------------------
    void setMat3(const std::string &name, const glm::mat3 &mat) const
    {
        glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    // ------------------------------------------------------------------------
    void setMat4(const std::string &name, const glm::mat4 &mat) const
    {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
private:
    // utility function for checking shader compilation/linking errors.
    // ------------------------------------------------------------------------
    void checkCompileErrors(unsigned int shader, std::string type)
    {
        int success;
        char infoLog[1024];
        if (type != "PROGRAM")
        {
            // 检查着色器编译错误
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
        else
        {
            // 检查程序链接错误
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
    }
};
#endif
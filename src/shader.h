/**
 * @file shader.h
 * @brief Simple OpenGl shader class that handles loading, compiling, and linking vertex and fragment shaders as well as hot reloading.
 * 
 * LOG:
 * 
 * Allows hot reloading but since binaries are compiled at runtime, it break the shaders somewhat.
 * Bugs in this case are: 
 * - bone structure and animation is removed 
 * - model is distorted (prolly a symptom of the above)
 * Not a priority to sort this out cuz no impact on gameplay or engine
 * 
 * @warning
 * personal note
 * ------------------------
 * DO NOT MODIFY
 * ------------------------
 * koi errors yaha nahi hai
 */

#ifndef SHADER_H

#define SHADER_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

class Shader {

public:
    unsigned int ID;

    std::string vertexPath;
    std::string fragmentPath;

    std::filesystem::file_time_type vertexModTimeRaw;
    std::filesystem::file_time_type fragmentModTimeRaw;

    Shader(const char *vPath, const char *fPath) {

        vertexPath = vPath;
        fragmentPath = fPath;

        updateModificationTimes();
        compileShaders();
    };

    void compileShaders() {

        std::string vertexCode;
        std::string fragmentCode;

        std::ifstream vShaderFile;
        std::ifstream fShaderFile;

        vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        try {
            vShaderFile.open(vertexPath);
            fShaderFile.open(fragmentPath);
            std::stringstream vShaderStream, fShaderStream;

            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();

            vShaderFile.close();
            fShaderFile.close();
            vertexCode = vShaderStream.str();
            fragmentCode = fShaderStream.str();
        } catch (std::ifstream::failure &) {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
        }
        const char *vShaderCode = vertexCode.c_str();
        const char *fShaderCode = fragmentCode.c_str();
        unsigned int vertex, fragment;
        vertex = glCreateShader(GL_VERTEX_SHADER);

        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        checkCompileErrors(vertex, "VERTEX");
        fragment = glCreateShader(GL_FRAGMENT_SHADER);

        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        checkCompileErrors(fragment, "FRAGMENT");

        ID = glCreateProgram();

        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);

        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");
        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }

    void updateModificationTimes() {
        try {
            vertexModTimeRaw = std::filesystem::last_write_time(vertexPath);
            fragmentModTimeRaw = std::filesystem::last_write_time(fragmentPath);
        } catch (const std::exception &e) {

            std::cout << "ERROR::SHADER::FAILED_TO_GET_MOD_TIME: " << e.what() << std::endl;
        }
    }

    bool hasShaderChanged() {
        try {
            auto vNewModTime = std::filesystem::last_write_time(vertexPath);
            auto fNewModTime = std::filesystem::last_write_time(fragmentPath);
            return (vNewModTime != vertexModTimeRaw || fNewModTime != fragmentModTimeRaw);
        } catch (const std::exception &e) {
            std::cout << "ERROR::SHADER::FAILED_TO_CHECK_MOD_TIME: " << e.what() << std::endl;
            return false;
        }
    }

    void hotReload() {
        if (hasShaderChanged()) {
            std::cout << "SHADER::HOT_RELOAD: Reloading shaders: " << vertexPath << " and " << fragmentPath << std::endl;
            unsigned int oldID = ID;

            compileShaders();
            updateModificationTimes();

            glDeleteProgram(oldID);
            std::cout << "SHADER::HOT_RELOAD: Shaders reloaded successfully!" << std::endl;
        }
    }

    void use() const {
        glUseProgram(ID);
    }
    void setBool(const std::string &name, bool value) const {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
    }
    void setInt(const std::string &name, int value) const {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }
    void setFloat(const std::string &name, float value) const {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }
    void setVec2(const std::string &name, const glm::vec2 &value) const {
        glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
    }
    void setVec2(const std::string &name, float x, float y) const {
        glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
    }
    void setVec3(const std::string &name, const glm::vec3 &value) const {
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
    }
    void setVec3(const std::string &name, float x, float y, float z) const {
        glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
    }
    void setVec4(const std::string &name, const glm::vec4 &value) const {
        glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
    }
    void setVec4(const std::string &name, float x, float y, float z, float w) const {
        glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
    }
    void setMat2(const std::string &name, const glm::mat2 &mat) const {
        glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    void setMat3(const std::string &name, const glm::mat3 &mat) const {
        glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    void setMat4(const std::string &name, const glm::mat4 &mat) const {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

private:
    void checkCompileErrors(unsigned int shader, std::string type) {
        int success;
        char infoLog[1024];

        if (type != "PROGRAM") {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

            if (!success) {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n"
                          << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        } else {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);

            if (!success) {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n"
                          << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
    }
};

#endif
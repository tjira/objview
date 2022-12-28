#pragma once

#include "../lib/GL/gl.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string>
#include <stdexcept>
#include <vector>

class Shader {
public:
    Shader(const std::string& vertex, const std::string& fragment);
    ~Shader() { glDeleteProgram(id); };
    void use() const { glUseProgram(id); };
    template <typename T> void set(const std::string& name, T value) const;

private:
    void errorCheck(unsigned int shader, const std::string& title) const;
    unsigned int id;
};

template <typename T>
void Shader::set(const std::string& name, T value) const {
    if constexpr (std::is_same<T, int>()) glUniform1i(glGetUniformLocation(id, name.c_str()), value);
    if constexpr (std::is_same<T, float>()) glUniform1f(glGetUniformLocation(id, name.c_str()), value);
    if constexpr (std::is_same<T, glm::vec3>()) glUniform3f(glGetUniformLocation(id, name.c_str()), value[0], value[1], value[2]);
    if constexpr (std::is_same<T, glm::vec4>()) glUniform4f(glGetUniformLocation(id, name.c_str()), value[0], value[1], value[2], value[3]);
    if constexpr (std::is_same<T, glm::mat4>()) glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &value[0][0]);
}

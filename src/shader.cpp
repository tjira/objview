#include "../include/shader.h"

Shader::Shader(const std::string& vertex, const std::string& fragment) : id(glCreateProgram()) {
    const char *fsCode = fragment.c_str(), *vsCode = vertex.c_str();
    unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER);
    unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vsCode, nullptr);
    glShaderSource(fs, 1, &fsCode, nullptr);
    glCompileShader(vs), glCompileShader(fs);
    errorCheck(vs, "vertex"), errorCheck(fs, "fragment");
    glAttachShader(id, vs), glAttachShader(id, fs);
    glLinkProgram(id), glValidateProgram(id);
    glDetachShader(id, vs), glDetachShader(id, fs);
    glDeleteShader(vs), glDeleteShader(fs), use();
}

void Shader::errorCheck(unsigned int shader, const std::string& title) const {
    int success; glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if(int length = 0; success == 0) {
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        std::vector<char> log(length);
        glGetShaderInfoLog(shader, length, &length, &log[0]);
        throw std::runtime_error(title + ": " + std::string(log.begin(), log.end()));
    }
}

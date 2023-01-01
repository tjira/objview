#pragma once


#include "buffer.h"
#include "shader.h"
#include <filesystem>
#include <functional>
#include <iostream>

class Mesh3D {
public:
    Mesh3D() {}; Mesh3D(std::vector<Vertex3D> data, const std::string& name = "mesh") : translate(1.0f), rotate(1.0f), scale(1.0f), name(name), buffer(data) {};
    static Mesh3D Load(const std::string& filename);
    void render(const Shader& shader, const glm::mat4& transform = glm::mat4(1.0f)) const;

private:
    glm::mat4 translate, rotate, scale;
    std::string name;
    Buffer buffer;
};

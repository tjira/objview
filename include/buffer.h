#pragma once

#include "../lib/GL/gl.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>

struct Vertex3D {
    glm::vec3 position, normal, color;
};

class Buffer {
public:
    Buffer() { generate(); }; Buffer(const std::vector<Vertex3D>& data) : data(data) { generate(); };
    ~Buffer() { glDeleteVertexArrays(1, &vao), glDeleteBuffers(1, &vbo); };
    Buffer& operator=(const Buffer& buffer);
    void bind() const { glBindVertexArray(vao); };
    std::vector<Vertex3D> getData() const { return data; }
    size_t getSize() const { return data.size(); };

private:
    void generate();
    unsigned int vao, vbo;
    std::vector<Vertex3D> data;
};

#define TINYOBJLOADER_IMPLEMENTATION

#include "../include/mesh3D.h"
#include "../lib/tinyobjloader/tiny_obj_loader.h"

Mesh3D Mesh3D::Load(const std::string& filename) {
    
    // Check if the file exists
    if (!std::filesystem::exists(filename)) {
        std::cerr << std::string("File '") + filename + std::string("' does not exist.") << std::endl; return Mesh3D();
    }

    // Prepare the data structures
    std::vector<Vertex3D> data;
    glm::vec3 center(0.0f);

    // Read the mesh
    tinyobj::ObjReader reader;
    if (!reader.ParseFromFile(filename)) {
        if (!reader.Error().empty()) {
            throw std::runtime_error(reader.Error());
        }
    }

    // Extract references stored in the mesh
    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();

    // Format the data to my own structure
    for (size_t i = 0; i < shapes.size(); i++) {
        for (size_t j = 0, offset = 0; j < shapes[i].mesh.num_face_vertices.size(); j++) {
            for (size_t v = 0; v < shapes[i].mesh.num_face_vertices[j]; v++) {
                Vertex3D vertex; tinyobj::index_t k = shapes[i].mesh.indices[offset + v];
                vertex.position.x = attrib.vertices[3 * k.vertex_index + 0];
                vertex.position.y = attrib.vertices[3 * k.vertex_index + 1];
                vertex.position.z = attrib.vertices[3 * k.vertex_index + 2];
                if (k.normal_index >= 0) {
                    vertex.normal.x = attrib.normals[3 * k.normal_index + 0];
                    vertex.normal.y = attrib.normals[3 * k.normal_index + 1];
                    vertex.normal.z = attrib.normals[3 * k.normal_index + 2];
                }
                center = center + vertex.position;
                vertex.color = {1, 1, 1};
                data.push_back(vertex);
            }
            offset += shapes[i].mesh.num_face_vertices[j];
        }
    }

    // Center the mesh at the origin
    for (Vertex3D& vertex : data) {
        vertex.position = vertex.position - center / (float)data.size();
    }

    // Return the object
    return Mesh3D(data, "test");
}

void Mesh3D::render(const Shader& shader, const glm::mat4& transform) const {
    shader.use(), shader.set<glm::mat4>("u_model", transform * model);
    buffer.bind(), glDrawArrays(GL_TRIANGLES, 0, (int)buffer.getSize());
}

#include "include/mesh3D.h"
#include "lib/argparse/argparse.hpp"
#include "lib/imgui/imgui_impl_opengl3.h"
#include "lib/imgui/imgui_impl_glfw.h"
#include "lib/imgui/ImGuiFileDialog.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#define WIDTH 1024
#define HEIGHT 576

std::string vertex = R"(
#version 420 core
layout(location = 0) in vec3 i_position;
layout(location = 1) in vec3 i_normal;
layout(location = 2) in vec3 i_color;
uniform mat4 u_model, u_view, u_proj;
out vec3 fragment, normal, color;
out mat3 transform;
void main() {
    normal = normalize(mat3(transpose(inverse(u_model))) * i_normal);
    fragment = vec3(u_model * vec4(i_position, 1)), color = i_color;
    gl_Position = u_proj * u_view * vec4(fragment, 1);
    transform = inverse(mat3(u_view));
})";

std::string fragment = R"(
#version 420 core
struct Light { vec3 position; float ambient, diffuse, specular, shininess; };
uniform Light u_light; uniform vec3 u_camera;
in vec3 fragment, normal, color;
in mat3 transform;
out vec4 o_color;
void main() {
    vec3 lightPos = transform * u_light.position;

    vec3 reflection = reflect(-normalize(lightPos), normal);
    vec3 direction = normalize(u_camera - fragment);
    vec3 specular = vec3(pow(max(dot(direction, reflection), 0), u_light.shininess));
    vec3 diffuse = vec3(max(dot(normal, normalize(lightPos)), 0));
    o_color = vec4((vec3(u_light.ambient) + u_light.diffuse * diffuse + u_light.specular * specular), 1) * vec4(color, 1);
})";

struct GLFWPointer {
    int width, height, samples = 16, major = 4, minor = 2;
    std::string title = "OpenGL Window";
    glm::vec2 mouse{};
    struct Camera {
        glm::mat4 view, proj;
    } camera{};
    struct Light {
        float ambient = 0.4f, diffuse = 0.2f, specular = 0.4f, shininess = 4.0f;
        glm::vec3 position = { 1.0f, 1.0f, 1.0f };
    } light{};
};

class Gui {
public:
    Gui(GLFWwindow* window); ~Gui();
    void render(Mesh3D& mesh);

private:
    GLFWwindow* window;
};

Gui::Gui(GLFWwindow* window) : window(window) {
    ImGui::CreateContext();
    ImGui_ImplOpenGL3_Init("#version 420");
    ImGui_ImplGlfw_InitForOpenGL(this->window, true);
    ImGui::GetIO().IniFilename = nullptr;
}

Gui::~Gui() {
    ImGui_ImplGlfw_Shutdown();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui::DestroyContext();
}

void Gui::render(Mesh3D& mesh) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos({ 0, 0 });
    ImGui::Begin("info", nullptr,
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoFocusOnAppearing
    );
    ImGui::Text("%.1f", ImGui::GetIO().Framerate);
    ImGui::End();

    if (ImGuiFileDialog::Instance()->Display("Import 3D Model", ImGuiWindowFlags_NoCollapse, { 512, 288 })) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            mesh = Mesh3D::Load(ImGuiFileDialog::Instance()->GetFilePathName());
        }
        ImGuiFileDialog::Instance()->Close();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void keyCallback(GLFWwindow* window, int key, int, int action, int mods) {
    if (action == GLFW_PRESS) {
        if (mods == GLFW_MOD_CONTROL && action == GLFW_PRESS) {
            if (key == GLFW_KEY_O) {
                std::string files = "Model Files{.obj,.stl},All Files{.*}";
                ImGuiFileDialog::Instance()->OpenDialog("Import 3D Model", "Import 3D Model", files.c_str(), ".");
            } else if (key == GLFW_KEY_Q) {
                glfwSetWindowShouldClose(window, GLFW_TRUE);
            }
        }
    }
}

void positionCallback(GLFWwindow* window, double x, double y) {
    GLFWPointer* pointer = (GLFWPointer*)glfwGetWindowUserPointer(window);
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !ImGui::GetIO().WantCaptureMouse) {
        glm::vec3 yaxis = glm::inverse(glm::mat3(pointer->camera.view)) * glm::vec3(1, 0, 0);
        glm::vec3 xaxis = glm::inverse(glm::mat3(pointer->camera.view)) * glm::vec3(0, 1, 0);
        pointer->camera.view = glm::rotate(pointer->camera.view, 0.01f * ((float)y - pointer->mouse.y), yaxis);
        pointer->camera.view = glm::rotate(pointer->camera.view, 0.01f * ((float)x - pointer->mouse.x), xaxis);
    }
    pointer->mouse = { x, y };
}

void resizeCallback(GLFWwindow* window, int width, int height) {
    if (GLFWPointer* pointer = (GLFWPointer*)glfwGetWindowUserPointer(window); width > 0 && height > 0) {
        pointer->camera.proj = glm::perspective(glm::radians(45.0f), (float)width / height, 0.01f, 1000.0f);
        pointer->width = width, pointer->height = height; glViewport(0, 0, width, height);
    }
}

void scrollCallback(GLFWwindow* window, double, double dy) {
    ((GLFWPointer*)glfwGetWindowUserPointer(window))->camera.view *= glm::mat4(glm::mat3(1.0f + 0.08f * (float)dy));
}

void set(const Shader& shader, const GLFWPointer::Camera& camera, const GLFWPointer::Light& light) {
    shader.set<glm::vec3>("u_camera", -glm::inverse(glm::mat3(camera.view)) * glm::vec3(camera.view[3]));
    shader.set<glm::mat4>("u_view", camera.view);
    shader.set<glm::mat4>("u_proj", camera.proj);
    shader.set<glm::vec3>("u_light.position", light.position);
    shader.set<float>("u_light.shininess", light.shininess);
    shader.set<float>("u_light.specular", light.specular);
    shader.set<float>("u_light.ambient", light.ambient);
    shader.set<float>("u_light.diffuse", light.diffuse);
}

int main(int argc, char** argv) {

    // Initialize argument parser
    argparse::ArgumentParser program("ObjView", "1.0", argparse::default_arguments::none);

    // Add argumens to argument parser
    program.add_argument("-f").help("File with model data to display.").default_value(std::string("model.obj"));
    program.add_argument("-h").help("Display this help message and exit.").default_value(false).implicit_value(true);

    // Parse the arguments
    try {
        program.parse_args(argc, argv);
    } catch (const std::runtime_error &error) {
        std::cerr << error.what() << std::endl << std::endl << program; return EXIT_FAILURE;
    }

    // Print the help message if the -h flag was passed
    if (program.get<bool>("-h")) {
        std::cout << program.help().str(); return EXIT_SUCCESS;
    }

    // Create GLFW variable struct and a window pointer
    GLFWPointer pointer{
        .width = WIDTH, .height = HEIGHT,
        .title = "Model View",
    };
    GLFWwindow* window;

    // Initialize GLFW and throw error if failed
    if(!glfwInit()) {
        throw std::runtime_error("Error during GLFW initialization.");
    }

    // Pass OpenGL version and other hints
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, pointer.major);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, pointer.minor);
    glfwWindowHint(GLFW_SAMPLES, pointer.samples);

    // Create the window
    if (window = glfwCreateWindow(pointer.width, pointer.height, pointer.title.c_str(), nullptr, nullptr); !window) {
        throw std::runtime_error("Error during window creation.");
    }

    // Initialize GLAD
    if (glfwMakeContextCurrent(window); !gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        throw std::runtime_error("Error during GLAD initialization.");
    }

    // Enable some options
    glEnable(GL_DEPTH_TEST), glEnable(GL_CULL_FACE);
    glfwSetWindowUserPointer(window, &pointer);
    glfwSwapInterval(1);

    // Set event callbacks
    glfwSetCursorPosCallback(window, positionCallback);
    glfwSetWindowSizeCallback(window, resizeCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetKeyCallback(window, keyCallback);

    // Initialize camera matrices
    pointer.camera.proj = glm::perspective(glm::radians(45.0f), (float)pointer.width / pointer.height, 0.01f, 1000.0f);
    pointer.camera.view = glm::lookAt({ 0.0f, 0.0f, 5.0f }, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    {
        // Create mesh, shader and GUI
        Mesh3D mesh = Mesh3D::Load(program.get<std::string>("-f"));
        Shader shader(vertex, fragment);
        Gui gui(window);
        
        // Enter the render loop
        while (!glfwWindowShouldClose(window)) {
            
            // Clear the color and depth buffer
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Set shade variables
            set(shader, pointer.camera, pointer.light);

            // Render mesh and GUI
            mesh.render(shader);
            gui.render(mesh);
            
            // Swap buffers and poll events
            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }

    // Clean up GLFW
    glfwTerminate();
}

#include "mesh3D.h"
#include "lib/imgui/imgui_impl_opengl3.h"
#include "lib/imgui/imgui_impl_glfw.h"
#include "lib/imgui/ImGuiFileDialog.h"

class Gui {
public:
    Gui(GLFWwindow* window); ~Gui();
    void render(Mesh3D& mesh);

private:
    GLFWwindow* window;
};

#include "window.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "ImGui/imgui_impl_glfw.h"
#include "ImGui/imgui_impl_opengl3.h"


BEGIN_CONTEXT_NAMESPACE

WinHdl Window::create(WindowProperties const& props)
{
    Window* win = new Window{};
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE,        GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_DOUBLEBUFFER,          1);
    glfwWindowHint(GLFW_DEPTH_BITS,            4);
    glfwWindowHint(GLFW_STENCIL_BITS,          8);
    debugnobr(glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);)


    win->glfw = glfwCreateWindow(props.x, props.y, props.title, NULL, NULL);
    win->m_arr = { props.x, props.y };
    ifcrashdo(win->glfw == nullptr, {
        markstr("Window::create(...) Failed to Create a GLFW window Instance.\n");
        /* replace with active-app instance termination from another include */
        glfwTerminate();
    });

    
    glfwMakeContextCurrent(win->glfw);
    glfwSwapInterval(0);
    ifcrashdo(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress), {
        markstr("Window::create(...) Failed to Update The OpenGL Context\n");

        /* replace with active-app instance termination from another include */
        glfwDestroyWindow(win->glfw);
        glfwTerminate();
    });


    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(win->glfw, true);
    ifcrashdo(!ImGui_ImplOpenGL3_Init("#version 460"), {
        markstr("Window::create(...) Failed to Initialize ImGui.\n")
    });
    return win;
}


END_CONTEXT_NAMESPACE
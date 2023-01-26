#pragma once
#include "shader.hpp"
#include "vertexArray.hpp"
#include "texture.hpp"
#include "GLFW/glfw3.h"




struct defaultCallbacks 
{
    using generic_error    = GLFWerrorfun;
    using framebuffer_size = GLFWframebuffersizefun;
    using input_keys       = GLFWkeyfun;

    generic_error*    errorEvent;
    framebuffer_size* windowSizeEvent;
    input_keys*       keyEvent;
};


struct Window
{
public:
    /* Note: The ImGui code will eventually be moved into a renderer Class/struct, when deemed necessary */
    GLFWwindow* handle;

    void create(i32 width, i32 height, defaultCallbacks const& link);
    void destroy();


    bool shouldClose()        const { return glfwWindowShouldClose(handle); }
    void procUpcomingEvents() const;
    void procOngoingEvents()  const;
};


struct GLObjects 
{
    ShaderProgram visual;
    ShaderProgram compute;

    std::vector<Buffer> buffers;
    VertexArray    vao;
    Texture2D      quad;

    GLObjects() = default;


    void initialize();
    void free();


    void bind();


    __force_inline void bindComputeShader() const { compute.bind(); return; }
    __force_inline void bindDefaultShader() const { visual.bind();  return; }
};




struct Context 
{
public:
    Window    glfw;
	GLObjects gl;


    Context()  = default;
    ~Context() = default;


    void create(
        i32 windowWidth, 
        i32 windowHeight,
        std::array<f32, 4> const& windowColor
    );
    void destroy();


    void processUpcomingEvents();
    void processOngoingEvents();
    void render();
    

    void queryGLinfo();


    static void glfw_error_callback(
        int 		error, 
        const char* description
    );

    static void glfw_framebuffer_size_callback(
        GLFWwindow* handle, 
        i32 width, 
        i32 height
    );

    static void glfw_key_callback(
        GLFWwindow* handle,
        int key, 
        int scancode, 
        int action, 
        int mods
    );

    static void gl_debug_message_callback(
        GLenum 		source, 
        GLenum 		type, 
        GLuint 		id, 
        GLenum 		severity, 
        GLsizei		length, 
        char const* message, 
        void const* user_param
    );


};

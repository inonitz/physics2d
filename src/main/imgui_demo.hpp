#pragma once
#include <glad/glad.h>
#include <ImGui/imgui.h>
#include "context.hpp"




namespace ImGui {
    void ShowDemoWindow(bool* p_open);
}



inline int imgui_demo_main()
{
    auto* ctx = getGlobalContext();
    defaultCallbacks eventFuncs = {
        glfw_error_callback,
        glfw_framebuffer_size_callback,
        glfw_key_callback,
        glfw_cursor_position_callback,
        glfw_mouse_button_callback
    };
    bool running = true;
    

    ctx->glfw.create(1280, 720, eventFuncs);
	debug( /* Enable Advanced OpenGL Debugging if enabled. */
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(gl_debug_message_callback, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
		// glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
	);
    while(running) 
    {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ctx->glfw.procUpcomingEvents();
        
        
        ImGui::ShowDemoWindow();
        
        
        running = !ctx->glfw.shouldClose() && !isKeyPressed(KeyCode::ESCAPE);
        ctx->glfw.procOngoingEvents();
    }
    ctx->glfw.close();
    return 0;
}
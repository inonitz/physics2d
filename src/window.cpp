#include <glad/glad.h>
#include "window.hpp"
#include "ImGui/imgui_impl_glfw.h"
#include "ImGui/imgui_impl_opengl3.h"




void window::create(i32 width, i32 height, defaultCallbacks const& glfwOverride)
{
	glfwSetErrorCallback(*glfwOverride.errorEvent);
	ifcrashdo(!glfwInit(), 
	{ 
		printf("GLFW Initialization Failed!\n"); 
	});
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_DOUBLEBUFFER , 1);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);
	debugnobr(
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	)


	handle = glfwCreateWindow(width, height, "[ImGui-1.85, GLFW, OpenGL-4.6, MinGW-64] mglw-strip App Window", NULL, NULL);
	ifcrashdo(!handle, { 
		printf("GLFW Window Creation Failed!\n");
		glfwTerminate(); 
	});
	glfwMakeContextCurrent(handle);
	glfwSwapInterval(0); // Disable vsync
	
	glfwSetFramebufferSizeCallback(handle, *glfwOverride.windowSizeEvent);
	glfwSetKeyCallback(handle, *glfwOverride.keyEvent);


	ifcrashdo( !gladLoadGLLoader((GLADloadproc)glfwGetProcAddress), 
	{ 
		printf("Couldn't initialize GLAD\n");

		glfwDestroyWindow(handle);
		glfwTerminate();
	});
    

	IMGUI_CHECKVERSION();
    ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io; 
	// io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
	
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(handle, true);
	ifcrash(!ImGui_ImplOpenGL3_Init("#version 460"));
	
	return;
}


void window::destroy()
{
    glfwSetWindowShouldClose(handle, true);
	ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(handle);
    glfwTerminate();
}


void window::procUpcomingEvents() const 
{
	/* begin frame */
	glfwPollEvents();
	
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	return;
}


void window::procOngoingEvents() const 
{
	/* end frame */
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	glfwSwapBuffers(handle);
	return;
}
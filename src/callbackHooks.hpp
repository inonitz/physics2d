#pragma once
#include "base.hpp"
#include <GLFW/glfw3.h>
#include <array>




/* should be probably placed in a different place from all the input-related functions */
struct defaultCallbacks 
{
    using generic_error    = GLFWerrorfun;
    using framebuffer_size = GLFWframebuffersizefun;
    using input_keys       = GLFWkeyfun;

    generic_error*    errorEvent;
    framebuffer_size* windowSizeEvent;
    input_keys*       keyEvent;
};




enum class KeyCode : u8 {
	W,
	A,
	S,
	D,
	Q,
	E,
	R,
	F,
	C,
	X,
	Z,
	T,
	G,
	V,
	ESCAPE,
	NUM0,
	NUM1,
	NUM2,
	NUM3,
	NUM4,
	NUM5,
	NUM6,
	NUM7,
	NUM8,
	NUM9,
	KEY_MAX
};


constexpr u16 keyCodeToGlfwKey(KeyCode kc)
{
	constexpr std::array<u16, (u8)KeyCode::KEY_MAX> glfwKeys = { GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D, GLFW_KEY_Q, GLFW_KEY_E, GLFW_KEY_R, GLFW_KEY_F, GLFW_KEY_C, GLFW_KEY_X, GLFW_KEY_Z, GLFW_KEY_T, GLFW_KEY_G, GLFW_KEY_V, GLFW_KEY_0, GLFW_KEY_1, GLFW_KEY_2, GLFW_KEY_3, GLFW_KEY_4, GLFW_KEY_5, GLFW_KEY_6, GLFW_KEY_7, GLFW_KEY_8, GLFW_KEY_9 };	
	return glfwKeys[static_cast<u8>(kc)];
}


constexpr KeyCode glfwKeyToKeyCode(u16 glfw)
{
	constexpr std::array<u16, (u8)KeyCode::KEY_MAX> glfwKeys  = { GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D, GLFW_KEY_Q, GLFW_KEY_E, GLFW_KEY_R, GLFW_KEY_F, GLFW_KEY_C, GLFW_KEY_X, GLFW_KEY_Z, GLFW_KEY_T, GLFW_KEY_G, GLFW_KEY_V, GLFW_KEY_0, GLFW_KEY_1, GLFW_KEY_2, GLFW_KEY_3, GLFW_KEY_4, GLFW_KEY_5, GLFW_KEY_6, GLFW_KEY_7, GLFW_KEY_8, GLFW_KEY_9 };	
	constexpr std::array<const char*, 2> 			chooseMsg = { "", "glfwKeyToKeyCode() ==> couldn't find glfw-KeyCode of value %u\n" };
	u8 i = 0;
	
	while(glfw != glfwKeys[i] && i < (u8)KeyCode::KEY_MAX) { ++i; }
	printf(chooseMsg[i == (u8)KeyCode::KEY_MAX], glfw);


	return (KeyCode)i;
}




void glfw_error_callback(int error, const char* description);
void glfw_framebuffer_size_callback(__unused GLFWwindow* handle, i32 w, i32 h);
void glfw_key_callback(GLFWwindow* handle, int key, int scancode, int action, __unused int mods);
void gl_debug_message_callback(
	GLenum 		source, 
	GLenum 		type, 
	GLuint 		id, 
	GLenum 		severity, 
	GLsizei		length, 
	char const* message, 
	void const* user_param
);


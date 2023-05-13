#pragma once
#include "util/base.hpp"
#include <GLFW/glfw3.h>
#include <array>




/* should be probably placed in a different place from all the input-related functions */
struct defaultCallbacks 
{
    using generic_error    = GLFWerrorfun;
    using framebuffer_size = GLFWframebuffersizefun;
    using input_keys       = GLFWkeyfun;
	using window_focused   = GLFWwindowfocusfun;
	using mouse_position   = GLFWcursorposfun;
	using mouse_input      = GLFWmousebuttonfun;

    generic_error    errorEvent;
    framebuffer_size windowSizeEvent;
    input_keys       keyEvent;
	// window_focused   activeWinEvent;
	mouse_position   mousePosEvent;
	mouse_input      mouseButtonEvent;
};




enum class KeyCode : u8 {
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
	B,
	H,
	Y,
	U,
	J,
	N,
	M,
	K,
	I,
	O,
	L,
	P,
	KEY_MAX
};


enum class MouseButton : u8 {
	LEFT,
	RIGHT,
	MIDDLE,
	MAX
};


enum class InputState : u8 {
	DEFAULT = 0, /* mapping is (1 << GLFW_KEY_STATE) */
	RELEASE = 1,
	PRESS   = 2, 
	REPEAT  = 4,
	MAX     = 8
};




void glfw_error_callback(int error, const char* description);
void glfw_framebuffer_size_callback(__unused GLFWwindow* handle, i32 w, i32 h);
void glfw_key_callback(GLFWwindow* handle, int key, int scancode, int action, __unused int mods);
// void glfw_window_focus_callback(GLFWwindow* window, int focused);
void glfw_cursor_position_callback(GLFWwindow *window, double xpos, double ypos);
void glfw_mouse_button_callback(GLFWwindow *window, int button, int action, int mods);


void gl_debug_message_callback(
	GLenum 		source, 
	GLenum 		type,
	GLuint 		id, 
	GLenum 		severity, 
	GLsizei		length, 
	char const* message, 
	void const* user_param
);


void update_key_callback_states();
void update_mouse_callback_states();


InputState getKeyState(KeyCode key);
InputState getMouseButtonState(MouseButton key);
template<typename T> std::array<T, 2> getCurrentFrameCursorPos();
template<typename T> std::array<T, 2> getPreviousFrameCursorPos();
template<typename T> std::array<T, 2> getCursorDelta();



#define isKeyPressed(key)  boolean(getKeyState(key) == InputState::PRESS  )
#define isKeyReleased(key) boolean(getKeyState(key) == InputState::RELEASE)
#define isButtonPressed(mButton)  boolean(getMouseButtonState(mButton) == InputState::PRESS  )
#define isButtonReleased(mButton) boolean(getMouseButtonState(mButton) == InputState::RELEASE)
#include "callback.hpp"
#include <GLFW/glfw3.h>




EVENT_NAMESPACE_BEGIN

void glfw_error_callback(
    int error, 
    const char* description
);
void glfw_framebuffer_size_callback(
    __unused GLFWwindow* handle, 
    i32 w, 
    i32 h
);
void glfw_key_callback(
    GLFWwindow* handle, 
    int key, 
    int scancode, 
    int action, 
    __unused int mods
);
void glfw_window_focus_callback(
    GLFWwindow* window, 
    int focused
);
void glfw_cursor_position_callback(
    GLFWwindow *window, 
    double xpos, 
    double ypos
);
void glfw_mouse_button_callback(
    GLFWwindow *window, 
    int button, 
    int action, 
    int mods
);
#ifdef _DEBUG
void gl_debug_message_callback(
	u32 	    source, 
	u32 	    type,
	u32 	    id, 
	u32 	    severity, 
	i32	        length, 
	char const* message, 
	void const* user_param
);
#endif

EVENT_NAMESPACE_END




INPUT_NAMESPACE_BEGIN

static constexpr std::array<u16, (u8)KeyCode::KEY_MAX> global_glfwKeys = { 
	GLFW_KEY_ESCAPE,
	GLFW_KEY_0,
	GLFW_KEY_1,
	GLFW_KEY_2,
	GLFW_KEY_3,
	GLFW_KEY_4,
	GLFW_KEY_5,
	GLFW_KEY_6,
	GLFW_KEY_7,
	GLFW_KEY_8,
	GLFW_KEY_9,
	GLFW_KEY_W,
	GLFW_KEY_A,
	GLFW_KEY_S,
	GLFW_KEY_D,
	GLFW_KEY_Q,
	GLFW_KEY_E,
	GLFW_KEY_R,
	GLFW_KEY_F,
	GLFW_KEY_C,
	GLFW_KEY_X,
	GLFW_KEY_Z,
	GLFW_KEY_T,
	GLFW_KEY_G,
	GLFW_KEY_V,
	GLFW_KEY_B,
	GLFW_KEY_H,
	GLFW_KEY_Y,
	GLFW_KEY_U,
	GLFW_KEY_J,
	GLFW_KEY_N,
	GLFW_KEY_M,
	GLFW_KEY_K,
	GLFW_KEY_I,
	GLFW_KEY_O,
	GLFW_KEY_L,
	GLFW_KEY_P
};	


static constexpr std::array<u16, (u8)MouseButton::MAX> global_glfwMouseButtons = { 
	GLFW_MOUSE_BUTTON_LEFT,
	GLFW_MOUSE_BUTTON_RIGHT, 
	GLFW_MOUSE_BUTTON_MIDDLE 
};



constexpr u16 toGLFWKeyCode(KeyCode kc)
{
	return global_glfwKeys[static_cast<u8>(kc)];
}
constexpr KeyCode toKeyCode(u16 glfw)
{
	u8 i = 0;
	while( i < (u8)KeyCode::KEY_MAX && glfw != global_glfwKeys[i]) { ++i; }
	debugnobr(if(unlikely(i == (u8)KeyCode::KEY_MAX)) {
		markfmt("glfwKeyToKeyCode() ==> couldn't find glfw-KeyCode of value %u\n", glfw);
	});
	return (KeyCode)i;
}


constexpr u16 toGLFWMouseButton(MouseButton button)
{
	return global_glfwMouseButtons[static_cast<u8>(button)];
}
constexpr MouseButton toMouseButton(u16 glfw)
{
	u8 i = 0;
	while( i < (u8)MouseButton::MAX && glfw != global_glfwMouseButtons[i]) { ++i; }
	debugnobr(if(unlikely(i == (u8)MouseButton::MAX)) {
		debug_messagefmt("glfwMouseButtonToButton() ==> couldn't find glfw-button of value %u\n", glfw);
	});
	return (MouseButton)i;
}

INPUT_NAMESPACE_END






// template<> void overrideEventCallback<GLFWerrorfun>(GLFWerrorfun overridefunc)
// {
//     defaultCallback.errorEvent = overridefunc;
//     glfwSetErrorCallback(defaultCallback.errorEvent);
//     return;
// }
// template<> void overrideEventCallback<GLFWframebuffersizefun>(GLFWframebuffersizefun overridefunc)
// {
//     defaultCallback.windowSizeEvent = overridefunc;
//     glfwSetFramebufferSizeCallback(handle, defaultCallback.windowSizeEvent);
//     return;
// }
// template<> void overrideEventCallback<GLFWkeyfun>(GLFWkeyfun overridefunc)
// {
//     defaultCallback.keyEvent = overridefunc;
//     return;
// }
// template<> void overrideEventCallback<GLFWwindowfocusfun>(GLFWwindowfocusfun overridefunc)
// {
//     defaultCallback.activeWinEvent = overridefunc;
//     return;
// }
// template<> void overrideEventCallback<GLFWcursorposfun>(GLFWcursorposfun overridefunc)
// {
//     defaultCallback.mousePosEvent = overridefunc;
//     return;
// }
// template<> void overrideEventCallback<GLFWmousebuttonfun>(GLFWmousebuttonfun overridefunc)
// {
//     defaultCallback.mouseButtonEvent = overridefunc;
//     return;
// }
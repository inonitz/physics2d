#include <glad/glad.h>
#include "callbackHooks.hpp"
#include <utility>
#include "context.hpp"




constexpr u16 toGLFWKeyCode(KeyCode kc)
{
	constexpr std::array<u16, (u8)KeyCode::KEY_MAX> glfwKeys = { GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D, GLFW_KEY_Q, GLFW_KEY_E, GLFW_KEY_R, GLFW_KEY_F, GLFW_KEY_C, GLFW_KEY_X, GLFW_KEY_Z, GLFW_KEY_T, GLFW_KEY_G, GLFW_KEY_V, GLFW_KEY_0, GLFW_KEY_1, GLFW_KEY_2, GLFW_KEY_3, GLFW_KEY_4, GLFW_KEY_5, GLFW_KEY_6, GLFW_KEY_7, GLFW_KEY_8, GLFW_KEY_9 };	
	return glfwKeys[static_cast<u8>(kc)];
}


constexpr KeyCode toKeyCode(u16 glfw)
{
	constexpr std::array<u16, (u8)KeyCode::KEY_MAX> glfwKeys  = { GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D, GLFW_KEY_Q, GLFW_KEY_E, GLFW_KEY_R, GLFW_KEY_F, GLFW_KEY_C, GLFW_KEY_X, GLFW_KEY_Z, GLFW_KEY_T, GLFW_KEY_G, GLFW_KEY_V, GLFW_KEY_0, GLFW_KEY_1, GLFW_KEY_2, GLFW_KEY_3, GLFW_KEY_4, GLFW_KEY_5, GLFW_KEY_6, GLFW_KEY_7, GLFW_KEY_8, GLFW_KEY_9 };	
	u8 i = 0;
	
	while(i < (u8)KeyCode::KEY_MAX && glfw != glfwKeys[i]) { ++i; }
	debugnobr(if(unlikely(i == (u8)KeyCode::KEY_MAX)) {
		markfmt("glfwKeyToKeyCode() ==> couldn't find glfw-KeyCode of value %u\n", glfw);
	});


	return (KeyCode)i;
}


constexpr u16 toGLFWMouseButton(MouseButton button)
{
	constexpr std::array<u16, (u8)MouseButton::MAX> glfwMouseButtons = { GLFW_MOUSE_BUTTON_LEFT, GLFW_MOUSE_BUTTON_RIGHT, GLFW_MOUSE_BUTTON_MIDDLE };
	return glfwMouseButtons[static_cast<u8>(button)];
}


constexpr MouseButton toMouseButton(u16 glfw)
{
	constexpr std::array<u16, (u8)MouseButton::MAX> glfwMouseButtons = { GLFW_MOUSE_BUTTON_LEFT, GLFW_MOUSE_BUTTON_RIGHT, GLFW_MOUSE_BUTTON_MIDDLE };
	u8 i = 0;

	while( i < (u8)MouseButton::MAX && glfw != glfwMouseButtons[i]) { ++i; }
	debugnobr(if(unlikely(i == (u8)KeyCode::KEY_MAX)) {
		debug_message("glfwMouseButtonToButton() ==> couldn't find glfw-button of value %u\n", glfw);
	});


	return (MouseButton)i;
}




struct
{
	u8 keys[(u8)KeyCode::KEY_MAX + 1]; /* KeyCode enum types are also used to index into the array */
	/* 
		bit 0: key was Release. 
		bit 1: key was Pressed. 
		when release => reset press 
		<==>
		when press   => reset release
	*/

	__force_inline void setState(u8 keyIndex, u8 state) {
		keys[keyIndex] = state;
	}
	__force_inline InputState getState(u8 keyIndex) const { 
		return static_cast<InputState>(keys[keyIndex]); 
	}


} KeyListener;


struct {
	/* Storing both previous & current frame position allows for mouse delta-calculations (mouse movement) */
	std::array<f64, 2> previousFramePos;
	std::array<f64, 2> currentFramePos;
	u8  buttons[static_cast<u8>(MouseButton::MAX) + 1];


	__force_inline void setState(u8 buttonIndex, u8 state) {
		buttons[buttonIndex] = state;
	}
	__force_inline InputState getState(u8 buttonIndex) const {
		return static_cast<InputState>(buttons[buttonIndex]);
	}


} MouseListener;




InputState getKeyState(KeyCode key) { return KeyListener.getState((u8)key); }
void update_key_callback_states()
{
	memset(KeyListener.keys, 0x00, (u8)KeyCode::KEY_MAX + 1); /* reset the array completely. */
	return;
}


InputState getMouseButtonState(MouseButton key) { return MouseListener.getState((u8)key); }
void update_mouse_callback_states()
{
	memset(MouseListener.buttons, 0x00, (u8)MouseButton::MAX + 1);
	return;
}
std::array<f64, 2> const& getCurrentFrameCursorPos()  { return MouseListener.currentFramePos;  }
std::array<f64, 2> const& getPreviousFrameCursorPos() { return MouseListener.previousFramePos; }
std::array<f64, 2> 		  getCursorDelta() 			  { 
	return {
		MouseListener.currentFramePos[0] - MouseListener.previousFramePos[0],
		-(MouseListener.currentFramePos[1] - MouseListener.previousFramePos[1]) /* Y axis is flipped on GLFW (X_axis = right, Y_axis = down)*/
	};
}




void glfw_error_callback(int error, const char* description)
{
	printf("GLFW Error %d: %s\n", error, description);
}


void glfw_framebuffer_size_callback(__unused GLFWwindow* handle, i32 w, i32 h)
{
	glViewport(0, 0, w, h);
	globalContext* ctx = reinterpret_cast<globalContext*>( glfwGetWindowUserPointer(handle) );
	
	ctx->glfw.minimized = (w == 0) || (h == 0);
	ctx->glfw.wh = { w, h };
	return;
}


void glfw_key_callback(__unused GLFWwindow* handle, int key, int __unused scancode, int action, __unused int mods)
{
	u8 keyCodeIndex = (u8)toKeyCode(key);
	u8 before		= KeyListener.keys[keyCodeIndex];
	static std::array<const char*, (u8)InputState::MAX + 1> actionStr = {
		"RELEASED",
		"PRESSED ",
		"REPEAT  ",
		""
	};

	actionStr[3] = actionStr[static_cast<u8>(action)];
	KeyListener.setState(keyCodeIndex, (action + 1) * (action != GLFW_REPEAT) );

	
	debug_message("[key_callback][kci=%02u][Before=%u]  { GLFW ==> [%s] Key %s }  [After=%u]\n", 
		keyCodeIndex, 
		before, 
		glfwGetKeyName(key, scancode), 
		actionStr[3], 
		(u8)KeyListener.getState(keyCodeIndex)
	);


	auto* ctx = getGlobalContext();
	ctx->camera.updateFromKeyboard(ctx->glfw.time_dt());
	return;
}


// void glfw_window_focus_callback(GLFWwindow* window, int focused)
// {
// 	windowIsCurrentlyFocused = 
// }


void glfw_cursor_position_callback(__unused GLFWwindow* window, double xpos, double ypos)
{
	MouseListener.previousFramePos = MouseListener.currentFramePos;
	MouseListener.currentFramePos = { xpos, ypos };
	
	getGlobalContext()->camera.updateFromMouse();
	return;
}


void glfw_mouse_button_callback(__unused GLFWwindow* window, int button, int action, __unused int mods)
{
	u8 buttonIndex = (u8)toMouseButton(button); /* might return MoueButton::MAX */
	u8 before      = MouseListener.buttons[buttonIndex];
	static std::array<const char*, (u8)InputState::MAX + 1> actionStr = {
		"RELEASED",
		"PRESSED ",
		"REPEAT  ",
		""
	};
	static std::array<const char*, (u8)MouseButton::MAX + 2> ButtonNames = {
		"MOUSE_BUTTON_LEFT  ",
		"MOUSE_BUTTON_RIGHT ",
		"MOUSE_BUTTON_MIDDLE",
		"MOUSE_BUTTON_UNKOWN",
		""
	};

	actionStr[3]   = actionStr[static_cast<u8>(action)];
	ButtonNames[4] = ButtonNames[static_cast<u8>(buttonIndex)];
	MouseListener.setState(buttonIndex, (action + 1) * (action != GLFW_REPEAT) );


	debug_message("[mouse_button_callback][bi=%02u][Before=%u]  { GLFW ==> [%s] Mouse Button %s }  [After=%u]\n", 
		buttonIndex,
		before,
		ButtonNames[4],
		actionStr[3], 
		(u8)MouseListener.getState(buttonIndex)
	);
	return;
}




#ifdef _DEBUG
void gl_debug_message_callback(
	GLenum 		  		source, 
	GLenum 		  		type, 
	GLuint 		  		id, 
	GLenum 		  		severity, 
	notused GLsizei     length, 
	GLchar const*       message, 
	notused void const* user_param
) {
	const std::pair<u32, const char*> srcStr[6] = {
		{ GL_DEBUG_SOURCE_API,             "API" 			 },
		{ GL_DEBUG_SOURCE_WINDOW_SYSTEM,   "WINDOW SYSTEM"   },
		{ GL_DEBUG_SOURCE_SHADER_COMPILER, "SHADER COMPILER" },
		{ GL_DEBUG_SOURCE_THIRD_PARTY,	   "THIRD PARTY" 	 },
		{ GL_DEBUG_SOURCE_APPLICATION,	   "APPLICATION" 	 },
		{ GL_DEBUG_SOURCE_OTHER, 		   "OTHER" 			 }
	};
	const std::pair<u32, const char*> typeStr[7] = {
		{ GL_DEBUG_TYPE_ERROR, 			     "ERROR"               },
		{ GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR, "DEPRECATED_BEHAVIOR" },
		{ GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR,  "UNDEFINED_BEHAVIOR"  },
		{ GL_DEBUG_TYPE_PORTABILITY,		 "PORTABILITY" 	       },
		{ GL_DEBUG_TYPE_PERFORMANCE,		 "PERFORMANCE" 		   },
		{ GL_DEBUG_TYPE_MARKER,			   	 "MARKER" 			   },
		{ GL_DEBUG_TYPE_OTHER,			     "OTHER" 			   }
	};
	const std::pair<u32, const char*> severityStr[6] = {
		{ GL_DEBUG_SEVERITY_NOTIFICATION, "NOTIFICATION" },
		{ GL_DEBUG_SEVERITY_LOW, 		  "LOW"		     },
		{ GL_DEBUG_SEVERITY_MEDIUM, 	  "MEDIUM"	     },
		{ GL_DEBUG_SEVERITY_HIGH, 		  "HIGH"	     }
	};
	const char* src_str      = srcStr[0].second;
	const char* type_str     = typeStr[0].second;
	const char* severity_str = severityStr[0].second;
	u32 		idx 		 = 0;
	

	while(srcStr[idx].first != source) { ++idx; }
	src_str = srcStr[idx].second;
	idx = 0;

	while(typeStr[idx].first != type)  { ++idx; }
	type_str = typeStr[idx].second;
	idx = 0;

	while(severityStr[idx].first != severity)  { ++idx; }
	severity_str = severityStr[idx].second;
	idx = 0;
	
	
	printf("OPENGL >> %s::%s::%s %u: %s\n", src_str, type_str, severity_str, id, message);
	return;
}
#else
void gl_debug_message_callback(
	__unused GLenum 	   source, 
	__unused GLenum 	   type, 
	__unused GLuint 	   id, 
	__unused GLenum 	   severity, 
	__unused GLsizei  	   length, 
	__unused GLchar const* message, 
	__unused void const*   user_param
) {
	return;
}
#endif

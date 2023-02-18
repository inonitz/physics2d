#include <array>
#include <glad/glad.h>
#include "callbackHooks.hpp"
#include <utility>
#include "context.hpp"




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
	while(i < (u8)KeyCode::KEY_MAX && glfw != global_glfwKeys[i]) { ++i; }
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
	debugnobr(if(unlikely(i == (u8)KeyCode::KEY_MAX)) {
		debug_messagefmt("glfwMouseButtonToButton() ==> couldn't find glfw-button of value %u\n", glfw);
	});


	return (MouseButton)i;
}




struct
{
	u8 keys[(u8)KeyCode::KEY_MAX + 1]; /* KeyCode enum types are also used to index into the array */


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

template<typename T> std::array<T, 2> getCurrentFrameCursorPos()  { return { static_cast<T>(MouseListener.currentFramePos[0]), static_cast<T>(MouseListener.currentFramePos[1]) }; }
template<typename T> std::array<T, 2> getPreviousFrameCursorPos() { return { static_cast<T>(MouseListener.previousFramePos[0]), static_cast<T>(MouseListener.previousFramePos[1]) };    }
template<typename T> std::array<T, 2> getCursorDelta() 			  { 
	return {
		(T)(MouseListener.currentFramePos[0]  - MouseListener.previousFramePos[0]),
		(T)(MouseListener.previousFramePos[1] - MouseListener.currentFramePos[1] ) /* Y axis is flipped on GLFW (X_axis = right, Y_axis = down)*/
	};
}
template std::array<f32, 2> getCurrentFrameCursorPos<f32>();
template std::array<f64, 2> getCurrentFrameCursorPos<f64>();
template std::array<u32, 2> getCurrentFrameCursorPos<u32>();
template std::array<f32, 2> getPreviousFrameCursorPos<f32>();
template std::array<f64, 2> getPreviousFrameCursorPos<f64>();
template std::array<u32, 2> getPreviousFrameCursorPos<u32>();
template std::array<f32, 2> getCursorDelta<f32>();
template std::array<f64, 2> getCursorDelta<f64>();
template std::array<u32, 2> getCursorDelta<u32>();




void glfw_error_callback(int error, const char* description)
{
	printf("GLFW Error %d: %s\n", error, description);
}


void glfw_framebuffer_size_callback(__unused GLFWwindow* handle, i32 w, i32 h)
{
	glViewport(0, 0, w, h);
	globalContext* ctx = reinterpret_cast<globalContext*>( glfwGetWindowUserPointer(handle) );
	
	debug_messagefmt("framebuffer callback ==> { %d , %d }\n", w, h);
	ctx->glfw.windowMinimized = (w == 0) || (h == 0); 
	ctx->glfw.wh = { w, h };
	ctx->persp.__.aspectRatio = w / (f32)h;
	return;
}


void glfw_key_callback(__unused GLFWwindow* handle, int key, int __unused scancode, int action, __unused int mods)
{
	u8 keyCodeIndex = (u8)toKeyCode(key);
	debugnobr(u8 before = KeyListener.keys[keyCodeIndex]);
	static std::array<const char*, (u8)InputState::MAX + 1> actionStr = {
		"RELEASED",
		"PRESSED ",
		"REPEAT  ",
		""
	};
	
	actionStr[3] = actionStr[static_cast<u8>(action)];
	KeyListener.setState(keyCodeIndex, (1 << action) );

	
	debug_messagefmt("[key_callback][kci=%02u][Before=%u]  [%s] Key %s  [After=%u]\n", 
		keyCodeIndex, 
		before, 
		glfwGetKeyName(key, scancode), 
		actionStr[3], 
		(u8)KeyListener.getState(keyCodeIndex)
	);



	// auto* ctx = getGlobalContext();
	// ctx->camera.updateFromKeyboard(ctx->glfw.time_dt());
	return;
}


void glfw_cursor_position_callback(__unused GLFWwindow* window, double xpos, double ypos)
{
	MouseListener.previousFramePos = MouseListener.currentFramePos;
	MouseListener.currentFramePos = { xpos, ypos };
	
	// getGlobalContext()->camera.updateFromMouse();
	return;
}


void glfw_mouse_button_callback(__unused GLFWwindow* window, int button, int action, __unused int mods)
{
	u8 buttonIndex = (u8)toMouseButton(button); /* might return MoueButton::MAX */
	debugnobr(u8 before = MouseListener.buttons[buttonIndex]);
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
	MouseListener.setState(buttonIndex, (1 << action) );


	auto p = getGlobalContext();
	if(getMouseButtonState(MouseButton::RIGHT) == InputState::PRESS) 
		p->glfw.lockCursor();

	if(getMouseButtonState(MouseButton::LEFT) == InputState::PRESS) 
		p->glfw.unlockCursor();
	
	
	debug_messagefmt("[mouse_button_callback][bi=%02u][Before=%u]  [%s] Mouse Button %s  [After=%u]\n", 
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

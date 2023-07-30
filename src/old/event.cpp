#include "event.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>


EVENT_NAMESPACE_BEGIN

static callbackTable defaultCallbacks;


void glfw_error_callback(int error, const char* description)
{
	printf("GLFW Error %d: %s\n", error, description);
}


void glfw_framebuffer_size_callback(__unused GLFWwindow* handle, i32 w, i32 h)
{
	glViewport(0, 0, w, h);
	globalContext* ctx = __rcast( globalContext*, glfwGetWindowUserPointer(handle) );
	
	debug_messagefmt("framebuffer callback ==> { %d , %d } [Old was %ux%u]\n", w, h, ctx->glfw.dims[0], ctx->glfw.dims[1]);
	ctx->glfw.windowMinimized = (w == 0) || (h == 0); 
	ctx->glfw.windowSizeChange = !ctx->glfw.windowMinimized && (  (ctx->glfw.dims[0] != w) || (ctx->glfw.dims[1] != h)  );
	ctx->glfw.dims = { w, h };
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
	return;
}


void glfw_cursor_position_callback(__unused GLFWwindow* window, double xpos, double ypos)
{
	MouseListener.previousFramePos = MouseListener.currentFramePos;
	MouseListener.currentFramePos = { xpos, ypos };
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

EVENT_NAMESPACE_END
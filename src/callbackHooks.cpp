#include <glad/glad.h>
#include "callbackHooks.hpp"
#include <utility>



struct
{
	u8 keys[(u8)KeyCode::KEY_MAX + 1]; /* KeyCode enum types are also used to index into the array */
	/* 
		bit 0: key was Pressed. 
		bit 1: key was released. 
		when release => reset press 
		<==>
		when press   => reset release
	*/

	__force_inline void setPressedState (u8 keyIndex) { /* keys[keyIndex] = (keys[keyIndex] & ~0b11) | 0b01; */ keys[keyIndex] = 0b01; }
	__force_inline void setReleasedState(u8 keyIndex) { /* keys[keyIndex] = (keys[keyIndex] & ~0b11) | 0b10; */ keys[keyIndex] = 0b10; }
	__force_inline void clearState      (u8 keyIndex) { keys[keyIndex] = 0; }

} KeyListener;




void glfw_error_callback(int error, const char* description)
{
	printf("GLFW Error %d: %s\n", error, description);
}


void glfw_framebuffer_size_callback(__unused GLFWwindow* handle, i32 w, i32 h)
{
	glViewport(0, 0, w, h);
}


void glfw_key_callback(__unused GLFWwindow* handle, int key, int scancode, int action, __unused int mods)
{
	mark();
	u8 keyCodeIndex = (action > 1) * 2 + action * (action < 2 && action >= 0);
	static std::array<const char*, 4> actionStr = {
		"PRESSED",
		"RELEASED",
		"UNKOWN_ACTION"
	};
	actionStr[3] = actionStr[keyCodeIndex];


	keyCodeIndex = (u8)glfwKeyToKeyCode(key);
	switch(action)
	{
		markfmt("[GLFW Registered Key[%s] for [%s] Key\n", actionStr[3], glfwGetKeyName(key, scancode));

		case GLFW_PRESS: 
		KeyListener.setPressedState(keyCodeIndex); 
		break;
		case GLFW_RELEASE: 
		KeyListener.setReleasedState(keyCodeIndex); 
		break;
		case GLFW_REPEAT:
		break;
		default:
		KeyListener.clearState(keyCodeIndex);
		break;
	}
	
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
void Context::gl_debug_message_callback(
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

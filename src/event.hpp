#include "util/base.hpp"


typedef struct GLFWwindow GLFWwindow;
#define EVENT_NAMESPACE_BEGIN namespace Event {
#define EVENT_NAMESPACE_END } 


EVENT_NAMESPACE_BEGIN

typedef void (* GLFWerrorfun)(
	int 		error_code, 
	const char* description
);
typedef void (* GLFWframebuffersizefun)(
	GLFWwindow* window, 
	int 		width, 
	int 		height
);
typedef void (* GLFWkeyfun)(
	GLFWwindow* window, 
	int 		key, 
	int 		scancode, 
	int 		action, 
	int 		mods
);
typedef void (* GLFWwindowfocusfun)(
	GLFWwindow* window, 
	int 		focused
);
typedef void (* GLFWcursorposfun)(
	GLFWwindow* window, 
	double 		xpos, 
	double 		ypos
);
typedef void (* GLFWmousebuttonfun)(
	GLFWwindow* window, 
	int 		button, 
	int 		action, 
	int 		mods
);
typedef void (* OpenGLdbgmsgfun)(
	unsigned int source, 
	unsigned int type,
	unsigned int id, 
	unsigned int severity, 
	signed   int length, 
	char const*  message, 
	void const*  user_param
);


struct callbackTable 
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
	window_focused   activeWinEvent;
	mouse_position   mousePosEvent;
	mouse_input      mouseButtonEvent;
#ifdef _DEBUG
    OpenGLdbgmsgfun  openglDebugEvent = nullptr; /* if nullptr context doesn't have opengl context */
#endif
};


callbackTable const& getDefaultEventTable();

EVENT_NAMESPACE_END
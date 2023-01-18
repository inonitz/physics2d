#include "basic.hpp"
#include "../base.hpp"
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "ImGui/imgui_impl_glfw.h"
#include "ImGui/imgui_impl_opengl3.h"
#include <GL/gl.h>
#include <string.h>
#include <bitset>




static void glfw_error_callback(
	int 		error, 
	const char* description
);


static void glfw_framebuffer_size_callback(
	GLFWwindow* handle, 
	i32 width, 
	i32 height
);


static void glfw_key_callback(
	GLFWwindow* handle,
	int key, 
	int scancode, 
	int action, 
	int mods
);



static void gl_debug_message_callback(
	GLenum 		  source, 
	GLenum 		  type, 
	GLuint 		  id, 
	GLenum 		  severity, 
	GLsizei		  length, 
	GLchar const* message, 
	void const*   user_param
);






GLFWwindow* createContext(i32 WindowWidth, i32 WindowHeight)
{
	glfwSetErrorCallback(glfw_error_callback);
	ifcrashdo(!glfwInit(), { printf("GLFW Initialization Failed!\n"); });


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


	GLFWwindow* window = glfwCreateWindow(WindowWidth, WindowHeight, "ImGui GLFW+OpenGL-4.5 App Window", NULL, NULL);
	ifcrashdo(!window, { 
		printf("GLFW Window Creation Failed!\n");
		glfwTerminate(); 
	});


	glfwMakeContextCurrent(window);
	glfwSwapInterval(0); // Disable vsync
	glfwSetFramebufferSizeCallback(window, glfw_framebuffer_size_callback);
	glfwSetKeyCallback(window, glfw_key_callback);


	ifcrashdo( !gladLoadGLLoader((GLADloadproc)glfwGetProcAddress), 
	{ 
		printf("Couldn't initialize GLAD\n");

		glfwDestroyWindow(window);
		glfwTerminate();
	});
    

	IMGUI_CHECKVERSION();
    ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGuiIO& io = ImGui::GetIO(); io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
	(void)io;


	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ifcrash(!ImGui_ImplOpenGL3_Init("#version 460"));


	debug( /* Enable Advanced Debugging if enabled. */
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(gl_debug_message_callback, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	);
	return window;
}


void destroyContext(GLFWwindow* handle)
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(handle);
    glfwTerminate();
}


void queryOpenGLInfo()
{
	i32 extensionCount, maj = 0, min = 0, isProfile = 0, glCtxt = 0;
	u64 choose = 0;
	std::bitset<8 * sizeof(u32)> contextFlagsBits;


	const char*    verCore   	 = "Core";
    const char*    verCompat 	 = "Compatibility";
	const char*    chosenProfile = nullptr;
	const GLubyte* vendor  		 = glGetString(GL_VENDOR);
	const GLubyte* renderer 	 = glGetString(GL_RENDERER);

	
    glGetIntegerv(GL_NUM_EXTENSIONS, 	   &extensionCount);
    glGetIntegerv(GL_MAJOR_VERSION,        &maj);
    glGetIntegerv(GL_MINOR_VERSION,        &min);
    glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &isProfile);
    glGetIntegerv(GL_CONTEXT_FLAGS,        &glCtxt);

    contextFlagsBits = glCtxt;
	choose = boolean(isProfile & GL_CONTEXT_CORE_PROFILE_BIT);
	choose = choose * (u64)verCore + !choose * (u64)verCompat;
    chosenProfile = (const char*)choose;


	printf("OpenGL Version %u.%u [%s]\nVendor: %s\nRenderer: %s\nContext Bits: %s\n\n", maj, min, chosenProfile, vendor, renderer, contextFlagsBits.to_string().c_str());
    printf("Listing Currently Available Extensions: \n");
    for(i32 i = 0; i < extensionCount; ++i) {        
        printf("Extension [%s]\n", glGetStringi(GL_EXTENSIONS, i));
    }


	printf("\n\npretty-print-list: \n[");
    for(i32 i = 0; i < extensionCount; ++i) {
        printf("%s, ", glGetStringi(GL_EXTENSIONS, i));
    }
	printf("\b\b]\n");
}




int basic()
{
	std::array<float, 4> clear  = { 0.45f, 0.55f, 0.60f, 1.00f };
	std::array<u32, 2>   dims   = { 1920, 1080  };
	GLFWwindow* 		 handle = createContext(dims[0], dims[1]);


	// game loop
	glClearColor(clear[0], clear[1], clear[2], clear[3]);
	while(!glfwWindowShouldClose(handle))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        glfwSwapBuffers(handle);
        glfwPollEvents();		
	}
    // game loop end


    glfwSetWindowShouldClose(handle, true);
	destroyContext(handle);
    return 0;
}




static void glfw_key_callback(GLFWwindow* handle, int key, int scancode, int action, __unused int mods)
{
	switch(action) 
	{
		case GLFW_PRESS:
		if(unlikely(key == GLFW_KEY_ESCAPE)) 
			glfwSetWindowShouldClose(handle, true);
		printf("[GLFW Registered KeyPress for [%s] Key\n", glfwGetKeyName(key, scancode));
		break;


	}
	return;
}


void glfw_framebuffer_size_callback(__unused GLFWwindow* handle, i32 w, i32 h)
{
	glViewport(0, 0, w, h);
}


static void glfw_error_callback(int error, const char* description)
{
	printf("GLFW Error %d: %s\n", error, description);
}


#ifdef _DEBUG
static void gl_debug_message_callback(
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
#endif
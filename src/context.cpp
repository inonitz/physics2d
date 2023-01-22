#include "glad/glad.h"
#include "context.hpp"
#include "ImGui/imgui_impl_glfw.h"
#include "ImGui/imgui_impl_opengl3.h"
#include <bitset>
#include <memory>




void Window::create(i32 width, i32 height, defaultCallbacks const& glfwOverride)
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
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
	
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(handle, true);
	ifcrash(!ImGui_ImplOpenGL3_Init("#version 460"));
	
	return;
}


void Window::destroy()
{
    glfwSetWindowShouldClose(handle, true);
	ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(handle);
    glfwTerminate();
}


void Window::procUpcomingEvents() const 
{
	/* begin frame */
	glfwPollEvents();
	
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	return;
}


void Window::procOngoingEvents() const 
{
	/* end frame */
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	glfwSwapBuffers(handle);
	return;
}




void GLObjects::initialize()
{
	constexpr std::array<const char*, 4> paths = {
		"/c/CTools/Projects/mglw-strip/assets/shaders/compute_basic/shader.vert",
		"/c/CTools/Projects/mglw-strip/assets/shaders/compute_basic/shader.frag",
		"/c/CTools/Projects/mglw-strip/assets/shaders/compute_basic/shader.comp",
		"/c/CTools/Projects/mglw-strip/assets/sample_img.jpg"
	};
	const manvec<float> vertices = 
	{
	    // positions               // texture coords
	     1.0f,  1.0f, 0.0f, 1.0f,  1.0f, 1.0f,  // top right
	     1.0f, -1.0f, 0.0f, 1.0f,  1.0f, 0.0f,  // bottom right
	    -1.0f, -1.0f, 0.0f, 1.0f,  0.0f, 0.0f,  // bottom left
	    -1.0f,  1.0f, 0.0f, 1.0f,  0.0f, 1.0f   // top left 
	};
	const manvec<u32> indices = 
	{
		0, 1, 3, /* first  triangle */
		1, 2, 3  /* second triangle */
	};


	buffers[0].create(
		BUFFER_TYPE_ARRAY, 		 
		BUFFER_USAGE_BIT_DYNAMIC, 
		VERTEX_DATATYPE_F32, 
		4,
		(void* const)vertices.begin()
	);
	buffers[1].create(
		BUFFER_TYPE_ELEMENT_ARRAY, 		 
		BUFFER_USAGE_BIT_DYNAMIC, 
		VERTEX_DATATYPE_F32, 
		6,
		(void* const)indices.begin()
	);

	vao.create();
	vao.addBuffer(
		0,
		buffers[0],
		{
			VertexArray::VertexDescriptor{
				4,
				VERTEX_DATATYPE_F32 | (0b10000000 ^ 0b10000000),
				0,
				0	
			},
			VertexArray::VertexDescriptor{
				2,
				VERTEX_DATATYPE_F32 | (0b10000000 ^ 0b10000000),
				sizeof(f32) * 4,
				1
			},
		}
	).addBuffer(0, buffers[1], {});

	
	Texture2D::fromFile(quad, paths[3], {
		{
			{ GL_TEXTURE_WRAP_S, 	 GL_REPEAT 				 },
			{ GL_TEXTURE_WRAP_T, 	 GL_REPEAT 				 },
			{ GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR },
			{ GL_TEXTURE_MAG_FILTER, GL_LINEAR 				 }
		},
		DEFAULT32,
		DEFAULT32
	});


	visual.create();  
	visual.addShader(paths[0], SHADER_TYPE_VERTEX)
		.addShader(paths[1], SHADER_TYPE_FRAGMENT);
	
	compute.create(); 
	// compute.addShader(paths[2], SHADER_TYPE_COMPUTE);


	visual.finalize();
	// compute.finalize();
	return;
}


void GLObjects::free()
{
	ShaderProgram::unbindCurrentProgram();
	
	quad.unbindFromTextureUnit();
	vao.unbind();
	
	
	for(auto& glBuffer : buffers) { glBuffer.destroy(); }
	vao.destroy();
	quad.destroy();


	visual.destroy();
	compute.destroy();
	return;
}


void GLObjects::bind()
{
	const u32 texUnit = 1;
	
	bindDefaultShader();
	quad.bindToTextureUnit(texUnit);
	quad.setActiveTexture(texUnit);
	vao.bind();

	visual.setUniform1u("texData", 1);
}



void Context::queryGLinfo()
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


void Context::glfw_error_callback(int error, const char* description)
{
	printf("GLFW Error %d: %s\n", error, description);
}


void Context::glfw_framebuffer_size_callback(__unused GLFWwindow* handle, i32 w, i32 h)
{
	glViewport(0, 0, w, h);
}


void Context::glfw_key_callback(GLFWwindow* handle, int key, int scancode, int action, __unused int mods)
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


#ifdef _DEBUG
void Context::gl_debug_message_callback(
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




void Context::create(i32 windowWidth, i32 windowHeight, std::array<f32, 4> const& windowColor)
{
	/* Initialize Window */
	defaultCallbacks eventFuncs = {};
	eventFuncs.errorEvent      = (defaultCallbacks::generic_error*)&glfw_error_callback;
	eventFuncs.windowSizeEvent = (defaultCallbacks::framebuffer_size*)&glfw_framebuffer_size_callback;
	eventFuncs.keyEvent 	   = (defaultCallbacks::input_keys*)&glfw_key_callback;
	glfw.create(windowWidth, windowHeight, eventFuncs);


	/* Initialize OpenGL stuff */
	debug( /* Enable Advanced Debugging if enabled. */
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(gl_debug_message_callback, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	);
	glClearColor(windowColor[0], windowColor[1], windowColor[2], windowColor[3]);
	mark();
	gl.initialize();
	mark();

	return;
}


void Context::destroy()
{
	/* Destroy Window (GLFW + ImGui) */
	glfw.destroy();


	/* Free OpenGL-related Resources */
	gl.free();


	return;
}


void Context::processUpcomingEvents()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glfw.procUpcomingEvents();
	return;
}


void Context::processOngoingEvents()
{
	glfw.procOngoingEvents();
	return;
}


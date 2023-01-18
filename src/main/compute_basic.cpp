// #include "compute_basic.hpp"
// #include "../imgui185/imgui.h"
// #include "../Context/context.hpp"
// #include "../Context/render.hpp"
// #include "../Context/input.hpp"
// #include "../Context/window.hpp"


// #include "../Context/state.hpp"
// #include <string>
// #include "GLFW/glfw3.h"




// void glfw_cursor_position_callback(GLFWwindow *window, double xpos, double ypos)
// {
// 	State* win = (State*)glfwGetWindowUserPointer(window);
// 	win->cursor = { (float)xpos, (float)ypos };
// 	return;
// }


// void framebuffer_size_callback(GLFWwindow* window, int width, int height)
// {
// 	State* win = (State*)glfwGetWindowUserPointer(window);
// 	win->dims = { width, height };

// 	glViewport(0, 0, width, height);
// 	return;
// }


// void glfw_error_callback(int error, const char* description)
// {
// 	printf("GLFW Error %d: %s\n", error, description);
// }





// #define MONITOR_MAX_WIDTH  2560
// #define MONITOR_MAX_HEIGHT 1440


// int compute_basic() 
// {
// 	Context::overrideDefaultCallbacks(
// 		reinterpret_cast<void*>(framebuffer_size_callback),
// 		nullptr,
// 		reinterpret_cast<void*>(glfw_cursor_position_callback)
// 	);
// 	Context::init(false, MONITOR_MAX_WIDTH, MONITOR_MAX_HEIGHT);
	
	
// 	const manvec<float> vertices = 
// 	{
// 	    // positions               // texture coords
// 	     1.0f,  1.0f, 0.0f, 1.0f,  1.0f, 1.0f,  // top right
// 	     1.0f, -1.0f, 0.0f, 1.0f,  1.0f, 0.0f,  // bottom right
// 	    -1.0f, -1.0f, 0.0f, 1.0f,  0.0f, 0.0f,  // bottom left
// 	    -1.0f,  1.0f, 0.0f, 1.0f,  0.0f, 1.0f   // top left 
// 	};
// 	const manvec<u32> indices = 
// 	{
// 		0, 1, 3, /* first  triangle */
// 		1, 2, 3  /* second triangle */
// 	};
// 	std::array<i32, 2> dims    = Context::Window::dimensions();
// 	std::array<i32, 3> wgpDims = { 0, 0, 1 };
// 	i32 	    	   tempVal = 0;
// 	std::string 	   str[3]  = {
// 		"/home/inon/mglw/assets/shaders/compute_basic/shader.vert",
// 		"/home/inon/mglw/assets/shaders/compute_basic/shader.frag",
// 		"/home/inon/mglw/assets/shaders/compute_basic/shader.comp"
// 	};


// 	Shader compute, program;
// 	VertexBuffer vbo;
// 	IndexBuffer  ibo;
// 	VertexArray  vao;
// 	Texture2D    tex;


// 	vbo.create(vertices.begin(), vertices.len(), VertexDataType::f32, GL_DYNAMIC_STORAGE_BIT);
// 	ibo.create(indices.begin(),  indices.len(),  VertexDataType::u32, GL_DYNAMIC_STORAGE_BIT);
// 	vao.create(vbo, ibo, 
// 	{
// 		{ 4, (u32)VertexDataType::f32, GL_FALSE, 0 },
// 		{ 2, (u32)VertexDataType::u32, GL_FALSE, 0 }
// 	});

// 	tex.create(
// 		{ GL_RGBA32F, GL_RGBA, GL_FLOAT },
// 		dims,
// 		{
// 			{ GL_TEXTURE_MIN_FILTER, GL_LINEAR          },
// 			{ GL_TEXTURE_MAG_FILTER, GL_LINEAR          },
// 			{ GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_BORDER },
// 			{ GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_BORDER }
// 		},
// 		nullptr
// 	);	

// 	compute.create({ loadShader(str[2], ShaderType::Compute ) });
// 	program.create({ loadShader(str[0], ShaderType::Vertex  ),
// 					 loadShader(str[1], ShaderType::Fragment) 
// 	});
// 	compute.compile();
// 	program.compile();

	
// 	compute.print();
// 	// vao.print();
	
	
// 	tex.bindToTexUnit(0);
// 	vao.bind();
// 	while(Context::active()) 
// 	{
// 		Renderer::beginBackendFrame();
		

// 		compute.setActive();
// 		{ 
// 			ImGui::BeginPopup("Program Parameters");
// 			ImGui::BeginGroup();

// 			ImGui::Text("%.5f ms/frame\n", 1000.0f / ImGui::GetIO().Framerate);
// 			ImGui::DragInt("Window Width" , &dims[0], 1.0f, 0, MONITOR_MAX_WIDTH );
// 			ImGui::DragInt("Window Height", &dims[1], 1.0f, 0, MONITOR_MAX_HEIGHT);

// 			ImGui::EndGroup();
// 			ImGui::DragInt3("Compute Workgroup Dimensions", wgpDims.begin(), 0.5f, 0, 2560);
// 			ImGui::EndPopup();
// 		}
// 		wgpDims = { dims[0], dims[1], 1 };
// 		compute.beginCompute(wgpDims);
// 		compute.barrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		

// 		program.setActive();
// 		program.setUniform("texData", UniformType::i32p, 1, &tempVal);
// 		tex.activeTexture(0);
// 		glDrawElements(GL_TRIANGLES, ibo.count(), ibo.gltype(), nullptr);


// 		Renderer::endBackendFrame();
// 	}


// 	/* Cleanup */
// 	vbo.destroy();
// 	ibo.destroy();
// 	vao.destroy();
// 	tex.destroy();
	
// 	compute.destroy();
// 	program.destroy();
// 	Context::terminate();
// 	return 0;
// }
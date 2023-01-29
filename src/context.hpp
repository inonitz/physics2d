#pragma once
#include "window.hpp"
#include "camera.hpp"
#include <array>
#include <vector>




static constexpr std::array<const char*, 4> paths = {
		"C:/CTools/Projects/mglw-strip/assets/shaders/compute_basic/shader.vert",
		"C:/CTools/Projects/mglw-strip/assets/shaders/compute_basic/shader.frag",
		"C:/CTools/Projects/mglw-strip/assets/shaders/compute_basic/shader.comp",
		"C:/CTools/Projects/mglw-strip/assets/sample_img.jpg"
};
static const std::vector<float> vertices = 
{
	// positions               // texture coords
		1.0f,  1.0f, 1.0f, 1.0f,  1.0f, 1.0f,  // top right
		1.0f, -1.0f, 1.0f, 1.0f,  1.0f, 0.0f,  // bottom right
	-1.0f, -1.0f, 1.0f, 1.0f,  0.0f, 0.0f,  // bottom left
	-1.0f,  1.0f, 1.0f, 1.0f,  0.0f, 1.0f   // top left 
};
static const std::vector<u32> indices = 
{
	0, 1, 3, /* first  triangle */
	1, 2, 3  /* second triangle */
};

struct Vertex {
	std::array<f32, 4> pos;
	std::array<f32, 2> tex;
};


struct globalContext {
	window    glfw;
	FPSCamera camera;

};


globalContext* getGlobalContext();



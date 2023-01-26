#pragma once
#include "window.hpp"
#include <array>
#include <vector>




static constexpr std::array<const char*, 4> paths = {
		"C:/Program Files/Programming Utillities/CProjects/mglw-strip/assets/shaders/compute_basic/shader.vert",
		"C:/Program Files/Programming Utillities/CProjects/mglw-strip/assets/shaders/compute_basic/shader.frag",
		"C:/Program Files/Programming Utillities/CProjects/mglw-strip/assets/shaders/compute_basic/shader.comp",
		"C:/Program Files/Programming Utillities/CProjects/mglw-strip/assets/sample_img.jpg"
};
static const std::vector<float> vertices = 
{
	// positions               // texture coords
		1.0f,  1.0f, 0.0f, 1.0f,  1.0f, 1.0f,  // top right
		1.0f, -1.0f, 0.0f, 1.0f,  1.0f, 0.0f,  // bottom right
	-1.0f, -1.0f, 0.0f, 1.0f,  0.0f, 0.0f,  // bottom left
	-1.0f,  1.0f, 0.0f, 1.0f,  0.0f, 1.0f   // top left 
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


static inline window glfw;
static inline u32 VAO, VBO, EBO, TEX;
static inline u32 windowWidth  = 1920;
static inline u32 windowHeight = 1080;
static inline std::array<float, 4> windowColor  		= { 0.45f, 0.55f, 0.60f, 1.00f };
static inline std::array<float, 4> texelColorMultiplier = { 1.0f,   0.0f, 0.5f,   1.0f };
static inline u32 shaders[2];
static inline u32 shaderProgram;
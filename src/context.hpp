#pragma once
#include "window.hpp"
// #include "vec.hpp"
#include "camera.hpp"
#include <array>
#include <vector>




static constexpr std::array<const char*, 4> assetPaths = {
		"C:/CTools/Projects/mglw-strip/assets/shaders/compute_basic/shader.vert",
		"C:/CTools/Projects/mglw-strip/assets/shaders/compute_basic/shader.frag",
		"C:/CTools/Projects/mglw-strip/assets/shaders/compute_basic/shader.comp",
		"C:/CTools/Projects/mglw-strip/assets/white_paper.jpg"
};
// static constexpr std::array<const char*, 4> assetPaths = {
// 		"C:/Program Files/Programming Utillities/CProjects/mglw-strip/assets/shaders/compute_basic/shader.vert",
// 		"C:/Program Files/Programming Utillities/CProjects/mglw-strip/assets/shaders/compute_basic/shader.frag",
// 		"C:/Program Files/Programming Utillities/CProjects/mglw-strip/assets/shaders/compute_basic/shader.comp",
// 		"C:/Program Files/Programming Utillities/CProjects/mglw-strip/assets/sample_img.jpg"
// };
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


struct Sphere
{
	math::vec4f center_radius; /* { Center(x, y, z), radius }*/
};


struct CameraTransform
{
	math::vec3f position;
	math::vec2f viewport;
	f32     focal_length;
	u8      reserved[4];
};


struct alignsz(8) SceneData
{
	CameraTransform transform;
	u32 	        max_length;
	u32 			curr_length;
	Sphere 			objects[1];
};


struct globalContext {
	window 			 glfw;
	Camera 			 cam;
	ProjectionMatrix persp;
	size_t 			 frameIndex;
};


globalContext* getGlobalContext();



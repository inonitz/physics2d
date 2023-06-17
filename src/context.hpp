#pragma once
#include "window.hpp"
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




/*
	{ Position(X, Y, Z), Radius(W) }
*/
using Sphere = math::vec4f;


/* 
	Depends on Material type:
	Lambertian: { albedo(x, y, z), reserved 	    }
	Metal:	    { albedo(x, y, z), fuzz     	    }
	dialectric: { reserved, 	   refraction_index }
*/
using Material = math::vec4f;


struct pack ObjectMaterialDescriptor
{
	#define MATERIAL_LAMBERTIAN  0b00
	#define MATERIAL_METAL 		 0b01
	#define MATERIAL_METAL_FUZZY 0b10
	#define MATERIAL_DIELECTRIC  0b11
	u8  materialType;
	u8  reserved;
	u16 materialIndex;
};


struct CameraTransform
{
	math::vec3f position;
	math::vec2f viewport;
	f32         focal_length;
	u32         reserved;
};


struct alignsz(8) SceneData
{
	CameraTransform transform;
	u32 			max_size; 
	u32 			curr_size; 
	u64 			reserved;
	Sphere 			objects[1];
};


struct globalContext {
	window 			 glfw;
	Camera 			 cam;
	ProjectionMatrix persp;
	size_t 			 frameIndex;
};


globalContext* getGlobalContext();



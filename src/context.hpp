#pragma once
#include "window.hpp"
#include "camera.hpp"
#include <array>




static constexpr std::array<const char*, 4> assetPaths = {
		"C:/CTools/Projects/mglw-strip/assets/shaders/compute_basic/shader.vert",
		"C:/CTools/Projects/mglw-strip/assets/shaders/compute_basic/shader.frag",
		"C:/CTools/Projects/mglw-strip/assets/shaders/compute_basic/shader.comp",
		"C:/CTools/Projects/mglw-strip/assets/white_paper.jpg"
};


struct globalContext {
	window 			 glfw;
	Camera 			 cam;
	ProjectionMatrix persp;
	size_t 			 frameIndex;
};


globalContext* getGlobalContext();



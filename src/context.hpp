#pragma once
#include "window.hpp"
#include "gl/shader2.hpp"
#include "gl/renderer.hpp"




struct globalContext
{
	using Renderer = Renderer::ObjectManager;
	Context   glfw;
	Program  shader;
	Renderer renderer;
	size_t   frameIndex;
};


void 		   createGlobalContext();
void 		   destroyGlobalContext();
globalContext* getGlobalContext();
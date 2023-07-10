#pragma once
#include "window.hpp"
#include "gl/shader2.hpp"




struct globalContext
{
	Window  glfw;
	Program shader;
	size_t  frameIndex;
};


globalContext* getGlobalContext();



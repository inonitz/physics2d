#pragma once
#include "base.hpp"
#include <GLFW/glfw3.h>
#include "callbackHooks.hpp"



struct window
{
public:
    /* Note: The ImGui code will eventually be moved into a renderer Class/struct, when deemed necessary */
    GLFWwindow* handle;

    void create(i32 width, i32 height, defaultCallbacks const& link);
    void destroy();


    bool shouldClose()        const { return glfwWindowShouldClose(handle); }
    void procUpcomingEvents() const;
    void procOngoingEvents()  const;
};
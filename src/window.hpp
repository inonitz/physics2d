#pragma once
#include "base.hpp"
#include <GLFW/glfw3.h>
#include "callbackHooks.hpp"



struct window
{
public:
    /* Note: The ImGui code will eventually be moved into a renderer Class/struct, when deemed necessary */
    GLFWwindow*        handle;
    std::array<f32, 2> deltaTime{0.0f, 0.0f};
    std::array<i32, 2> wh{(i32)DEFAULT32, (i32)DEFAULT32};
    bool               windowMinimized{false};



    void create(i32 width, i32 height, defaultCallbacks const& link);
    void destroy();

    template<typename T> T aspectRatio() const { return wh[0] / ( (T)wh[1] ) ;}
    f32  time_dt()      const { return deltaTime[1] - deltaTime[0];   }
    bool shouldClose()  const { return glfwWindowShouldClose(handle); }
    bool minimized()    const { return windowMinimized;               }

    __force_inline void close() const { glfwSetWindowShouldClose(handle, true); }
    void lockCursor() const;
    void unlockCursor() const;
    void procUpcomingEvents();
    void procOngoingEvents()  const;
};
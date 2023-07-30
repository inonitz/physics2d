#pragma once
#include "util/base.hpp"
#include "event.hpp"
#include "input.hpp"




struct Context
{
public:
    GLFWwindow *handle;
    std::array<f32, 2> deltaTime{0.0f, 0.0f};
    std::array<i32, 2> dims{(i32)DEFAULT32, (i32)DEFAULT32};
    bool windowMinimized{false};
    bool windowSizeChange{false};

    void create(i32 width, i32 height);
    void destroy();

    template <typename T> T aspectRatio() const
    {
        return dims[0] / ((T)dims[1]);
    }
    f32 time_dt() const
    {
        return deltaTime[1] - deltaTime[0];
    }
    bool minimized() const
    {
        return windowMinimized;
    }
    bool windowSizeChanged() const
    {
        return windowSizeChange;
    }
    void dimensions(u32 *out) const
    {
        out[0] = __scast(u32, dims[0]);
        out[1] = __scast(u32, dims[1]);
        return;
    }

    bool shouldClose() const;
    void close() const;
    void lockCursor() const;
    void unlockCursor() const;
    void setCursorMode(bool toLock) const;
    void processPreviousFrame();
    void processCurrentFrame();
};
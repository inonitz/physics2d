#pragma once
#include "util/base.hpp"
#include <array>


#define BEGIN_CONTEXT_NAMESPACE namespace Context { 
#define END_CONTEXT_NAMESPACE }
typedef struct GLFWwindow GLFWwindow;
 

BEGIN_CONTEXT_NAMESPACE

struct WindowProperties
{
    const char* title;
    u32 x, y;
};


class Window
{
public: 
    static Window* create(WindowProperties const& props);


    void destroy();
    u32  getWidth()  const;
    u32  getHeight() const;
    std::array<u32, 2> const& getDims() const { return m_arr; }

private:
    union {
        struct { u32 x, y; } m_dims;
        std::array<u32, 2>   m_arr;
    };
    GLFWwindow* glfw;
};

using WinHdl  = mut_type_handle<Window>;
using kWinHdl = imut_type_handle<Window>;

END_CONTEXT_NAMESPACE
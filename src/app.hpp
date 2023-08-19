#pragma once
#include "util/base.hpp"
#include "window.hpp"
#include "input.hpp"


class Application 
{
public: 
    void create();
    void destroy();
    static void attachWindow(Application* app, Context::WinHdl window);
    static void attachInputContext(Application* app);
private:
    Context::WinHdl win;
    
};


using AppHdl  = mut_type_handle<Application>;
using kAppHdl = imut_type_handle<Application>;


AppHdl getActiveApp();
bool   isInit();
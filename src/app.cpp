#include <GLFW/glfw3.h>
#include "app.hpp"
#include "event.hpp"


void Application::create()
{
    glfwSetErrorCallback(Event::getDefaultEventTable().errorEvent);
    ifcrashdo(!glfwInit(), { 
        printf("GLFW Initialization Failed!\n"); 
    });
}


void Application::destroy()
{
    
    return;
}


void Application::attachWindow(Application* app, Context::WinHdl window)
{
    app->win = window;
    /*    */
    return;
}


void Application::attachInputContext(Application* app)
{

}




// AppHdl getActiveApp() {
//     return activeApp;
// }


// bool isInit() {
//     return activeApp != nullptr;
// }
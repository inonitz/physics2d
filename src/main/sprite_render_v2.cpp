#include "sprite_render_v2.hpp"
#include "app.hpp"
#include "window.hpp"
#include "input.hpp"




int sprite_render_v2()
{
    AppHdl app = new Application{};
    Context::WinHdl  win;
    Input::InputUnit in;


    app->create();
    win = Context::Window::create({ "Testing Window Lmao ", 1280, 720 });
    in = Input::
    Application::attachWindow(app, win);
    Application::attachInputContext(app, )




}
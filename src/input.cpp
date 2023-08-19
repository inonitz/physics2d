#include <GLFW/glfw3.h>
#include "input.hpp"


INPUT_NAMESPACE_BEGIN

InputUnit* InputUnit::create() {
    InputUnit* hdl = new InputUnit{};
    hdl->mouse.resetState();
    hdl->keyboard.resetState();
    return hdl;
}

void InputUnit::onUpdate()
{
    mouse.resetState();
    keyboard.resetState();
    return;
}


INPUT_NAMESPACE_END
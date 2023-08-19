#pragma once
#include "util/base.hpp"
#include <array>


#define INPUT_NAMESPACE_BEGIN namespace Input {
#define INPUT_NAMESPACE_END }
#define ANONYMOUSE_NAMESPACE_BEGIN namespace {
#define ANONYMOUSE_NAMESPACE_END }


INPUT_NAMESPACE_BEGIN

enum class keyCode : u8 {
	ESCAPE,
	NUM0,
	NUM1,
	NUM2,
	NUM3,
	NUM4,
	NUM5,
	NUM6,
	NUM7,
	NUM8,
	NUM9,
	W,
	A,
	S,
	D,
	Q,
	E,
	R,
	F,
	C,
	X,
	Z,
	T,
	G,
	V,
	B,
	H,
	Y,
	U,
	J,
	N,
	M,
	K,
	I,
	O,
	L,
	P,
	KEY_MAX
};


enum class mouseButton : u8 {
	LEFT,
	RIGHT,
	MIDDLE,
	MAX
};


enum class inputState : u8 {
	DEFAULT = 0, /* mapping is (1 << GLFW_KEY_STATE) */
	RELEASE = 1,
	PRESS   = 2, 
	REPEAT  = 4,
	MAX     = 8
};


class InputUnit
{
public:
    static InputUnit* create();
    void onUpdate();


    inputState getKeyState        (keyCode key    );
    inputState getMouseButtonState(mouseButton key);
    template<typename T> 
    std::array<T, 2> getCurrentFrameCursorPos() {
        return mouse.getCurrentFrameCursorPos<T>();
    }
    template<typename T> 
    std::array<T, 2> getPreviousFrameCursorPos() {
        return mouse.getPreviousFrameCursorPos<T>();
    } 
    template<typename T> 
    std::array<T, 2> getCursorDelta() {
        return mouse.getCursorDelta<T>();
    }


private:
    typedef struct KeyboardStateArray
    {
        u8 keys[(u8)Input::keyCode::KEY_MAX + 1] = {0}; /* keyCode enum types are also used to index into the array */


        __force_inline void setState(u8 keyIndex, u8 state) {
            keys[keyIndex] = state;
        }
        __force_inline inputState getState(u8 keyIndex) const { 
            return __scast(inputState, keys[keyIndex]); 
        }
        __force_inline void resetState() {
            memset(&keys[0], 0x00, sizeof(keys));
            return;
        }
    } KeyListener;


    typedef struct MouseButtonStateArray 
    {
        std::array<f64, 2> previousFramePos;
        std::array<f64, 2> currentFramePos;
        u8 buttons[static_cast<u8>(mouseButton::MAX) + 1] = {0};


        __force_inline void setState(u8 buttonIndex, u8 state) {
            buttons[buttonIndex] = state;
            return;
        }
        __force_inline inputState getState(u8 buttonIndex) const {
            return __scast(inputState, buttons[buttonIndex]); 
        }
        __force_inline void resetState() {
            memset(&buttons[0], 0x00, sizeof(buttons));
            return;
        }


        template<typename T> 
        std::array<T, 2> getCurrentFrameCursorPos() const {
            return {
                __scast(T, currentFramePos[0]),
                __scast(T, currentFramePos[1])
            };
        }
        template<typename T> 
        std::array<T, 2> getPreviousFrameCursorPos() const {
            return {
                __scast(T, previousFramePos[0]),
                __scast(T, previousFramePos[1])
            };
        }
        template<typename T> 
        std::array<T, 2> getCursorDelta() const { 
        return { /* Y axis is flipped on GLFW (X_axis = right, Y_axis = down) */
            __scast(T, (currentFramePos[0]  - previousFramePos[0]) ),
            __scast(T, (previousFramePos[1] - currentFramePos[1] ) )
            };
        }


        template<> std::array<f32, 2> getCurrentFrameCursorPos<f32>() const;
        template<> std::array<f64, 2> getCurrentFrameCursorPos<f64>() const;
        template<> std::array<u32, 2> getCurrentFrameCursorPos<u32>() const;
        template<> std::array<f32, 2> getPreviousFrameCursorPos<f32>() const;
        template<> std::array<f64, 2> getPreviousFrameCursorPos<f64>() const;
        template<> std::array<u32, 2> getPreviousFrameCursorPos<u32>() const;
        template<> std::array<f32, 2> getCursorDelta<f32>() const;
        template<> std::array<f64, 2> getCursorDelta<f64>() const;
        template<> std::array<u32, 2> getCursorDelta<u32>() const;
    } MouseListener;


private:
    MouseListener mouse;
    KeyListener   keyboard;
};


/* needs to move to app.cpp */
#define isKeyPressed(key)  boolean(getKeyState(key) == inputState::PRESS  )
#define isKeyReleased(key) boolean(getKeyState(key) == inputState::RELEASE)
#define isButtonPressed(mButton)  boolean(getMouseButtonState(mButton) == inputState::PRESS  )
#define isButtonReleased(mButton) boolean(getMouseButtonState(mButton) == inputState::RELEASE)

INPUT_NAMESPACE_END
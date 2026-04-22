#pragma once

#include "ui/Screen.h"
#include "ui/NavInput.h"

class Router {
public:
    void push(Screen* screen);
    void pop();
    void navigateTo(Screen* screen);
    Screen* currentScreen() const;
    void handleInput(NavInput input);

private:
    static constexpr uint8_t MAX_STACK_SIZE = 8;
    Screen* _stack[MAX_STACK_SIZE] = {};
    uint8_t _stackSize = 0;
};

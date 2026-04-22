#include "ui/Router.h"

void Router::push(Screen* screen) {
    if (screen == nullptr) return;
    if (_stackSize >= MAX_STACK_SIZE) return;
    if (_stackSize > 0) _stack[_stackSize - 1]->onUnmount();
    _stack[_stackSize] = screen;
    _stackSize++;
    screen->onMount();
}

void Router::pop() {
    if (_stackSize <= 1) return;
    _stack[_stackSize - 1]->onUnmount();
    _stackSize--;
    _stack[_stackSize - 1]->onMount();
}

void Router::navigateTo(Screen* screen) {
    if (screen == nullptr) return;
    if (_stackSize > 0) _stack[_stackSize - 1]->onUnmount();
    _stack[0] = screen;
    _stackSize = 1;
    screen->onMount();
}

Screen* Router::currentScreen() const {
    if (_stackSize == 0) return nullptr;
    return _stack[_stackSize - 1];
}

void Router::handleInput(NavInput input) {
    Screen* current = currentScreen();
    if (current != nullptr) current->handleInput(input);
}

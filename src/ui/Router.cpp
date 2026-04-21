#include "ui/Router.h"

void Router::push(Screen* screen) {
    if (screen == nullptr) {
        return;
    }
    if (_stackSize >= MAX_STACK_SIZE) {
        return; // Pilha cheia — ignorar silenciosamente
    }

    // Desmonta a Screen atual (se houver)
    if (_stackSize > 0) {
        _stack[_stackSize - 1]->onUnmount();
    }

    // Empilha a nova Screen e monta
    _stack[_stackSize] = screen;
    _stackSize++;
    screen->onMount();
}

void Router::pop() {
    if (_stackSize <= 1) {
        return; // Pilha com 0 ou 1 Screen — ignorar silenciosamente
    }

    // Desmonta a Screen atual
    _stack[_stackSize - 1]->onUnmount();
    _stackSize--;

    // Monta a Screen que ficou no topo
    _stack[_stackSize - 1]->onMount();
}

void Router::navigateTo(Screen* screen) {
    if (screen == nullptr) {
        return;
    }

    // Desmonta a Screen atual (se houver)
    if (_stackSize > 0) {
        _stack[_stackSize - 1]->onUnmount();
    }

    // Substitui toda a pilha por uma única Screen
    _stack[0] = screen;
    _stackSize = 1;
    screen->onMount();
}

Screen* Router::currentScreen() const {
    if (_stackSize == 0) {
        return nullptr;
    }
    return _stack[_stackSize - 1];
}

void Router::handleInput(ButtonEvent event) {
    Screen* current = currentScreen();
    if (current != nullptr) {
        current->handleInput(event);
    }
}

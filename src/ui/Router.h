#pragma once

#include <stdint.h>
#include "ui/Screen.h"

/**
 * Router — gerencia navegação por pilha estática de Screens.
 *
 * Mantém uma pilha LIFO de ponteiros para Screen. Operações inválidas
 * (pop com 1 Screen, push com pilha cheia) são ignoradas silenciosamente.
 */
class Router {
public:
    void push(Screen* screen);
    void pop();
    void navigateTo(Screen* screen);
    Screen* currentScreen() const;
    void handleInput(ButtonEvent event);

private:
    static constexpr uint8_t MAX_STACK_SIZE = 8;
    Screen* _stack[MAX_STACK_SIZE] = {};
    uint8_t _stackSize = 0;
};

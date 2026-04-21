#pragma once

#include "ui/Screen.h"

/**
 * Template de estado reativo.
 *
 * Encapsula um valor tipado e mantém referência à Screen dona.
 * Quando set() é chamado com um valor diferente do atual, a Screen
 * é marcada como suja (dirty) para disparar redesenho no próximo
 * ciclo do RenderLoop.
 *
 * Suporta tipos primitivos (int, bool, float) e structs do usuário
 * que implementem operator!=.
 */
template <typename T>
class State {
public:
    State(Screen* owner, T initialValue)
        : _owner(owner), _value(initialValue) {}

    /// Retorna o valor atual (somente leitura).
    const T& get() const { return _value; }

    /// Atualiza o valor. Marca a Screen como dirty apenas se o valor mudou.
    void set(const T& newValue) {
        if (_value != newValue) {
            _value = newValue;
            if (_owner) _owner->markDirty();
        }
    }

private:
    Screen* _owner;
    T _value;
};

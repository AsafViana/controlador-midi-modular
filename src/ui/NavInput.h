#pragma once

/**
 * NavInput — identifica qual botão físico foi pressionado.
 * Cada botão tem papel fixo, independente do tipo de clique.
 */
enum class NavInput : uint8_t {
    NONE,
    UP,      // btnUp   — pressionar sobe na lista
    DOWN,    // btnDown — pressionar desce na lista
    SELECT   // btnSelect — pressionar confirma / entra
};

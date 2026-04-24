#pragma once

/**
 * NavInput — identifica qual botão físico foi pressionado.
 * Cada botão tem papel fixo, independente do tipo de clique.
 *
 * LONG_UP e LONG_DOWN são enviados quando o botão é mantido
 * pressionado, permitindo aceleração na edição de valores.
 */
enum class NavInput : uint8_t {
  NONE,
  UP,       // btnUp   — pressionar sobe na lista / incrementa
  DOWN,     // btnDown — pressionar desce na lista / decrementa
  SELECT,   // btnSelect — pressionar confirma / entra
  LONG_UP,  // btnUp mantido — incremento rápido (+5)
  LONG_DOWN // btnDown mantido — decremento rápido (-5)
};

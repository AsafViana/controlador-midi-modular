#pragma once

#include <cstdint>

/**
 * CCActivityInfo — Dados do último CC enviado.
 *
 * Usado para feedback visual na PerformanceScreen.
 * Contém todas as informações necessárias para o cliente
 * validar que o controle certo está enviando o valor certo.
 */
struct CCActivityInfo {
  const char *label;     // Nome do controle (ex: "Pot Volume", "Pot1")
  uint8_t cc;            // Número do CC enviado (0-127)
  uint8_t valor;         // Valor do CC (0-127)
  uint8_t canal;         // Canal MIDI (1-16)
  bool isRemoto;         // true se vem de módulo externo
  uint8_t moduleAddress; // Endereço I2C (só para remotos, ex: 0x20)
};

/// Tipo do callback chamado a cada envio de CC com informações completas.
using CCActivityCallback = void (*)(const CCActivityInfo &info);

#pragma once

#include <cstdint>
#include "hardware/HardwareMap.h"
#include "hardware/UnifiedControlList.h"

class MidiEngine;
class Storage;
class I2CScanner;

/**
 * ControlReader — Leitura automática de controles analógicos.
 *
 * Varre todos os controles do HardwareMap que são analógicos
 * (POTENCIOMETRO ou SENSOR), lê o GPIO, converte para 0-127,
 * e envia o CC configurado no Storage — tudo automático.
 *
 * Quando uma UnifiedControlList e I2CScanner são fornecidos,
 * também lê controles remotos de módulos I2C e aplica a mesma
 * lógica de zona morta e envio de CC.
 *
 * Respeita o flag de habilitado/desabilitado do Storage.
 * Inclui filtro de ruído (zona morta) para evitar flood.
 */
class ControlReader {
public:
    /// Zona morta: só envia CC se a diferença for maior que este valor.
    static constexpr uint8_t ZONA_MORTA = 1;

    /// Intervalo mínimo entre leituras (ms).
    static constexpr uint32_t INTERVALO_MS = 10;

    /// Máximo de controles remotos suportados.
    static constexpr uint8_t MAX_REMOTE_CONTROLS =
        UnifiedControlList::MAX_TOTAL_CONTROLS - HardwareMap::NUM_CONTROLES;

    /**
     * @param engine  Ponteiro para o MidiEngine (envio de CC)
     * @param storage Ponteiro para o Storage (CC map + habilitado)
     * @param ucl     Ponteiro para a UnifiedControlList (nullptr = modo standalone)
     * @param scanner Ponteiro para o I2CScanner (nullptr = modo standalone)
     */
    ControlReader(MidiEngine* engine, Storage* storage,
                  UnifiedControlList* ucl = nullptr,
                  I2CScanner* scanner = nullptr);

    /// Inicializa os pinos analógicos. Chamar no setup().
    void begin();

    /// Lê todos os controles e envia CC se necessário. Chamar no loop().
    void update();

private:
    MidiEngine* _engine;
    Storage* _storage;
    UnifiedControlList* _ucl;
    I2CScanner* _scanner;

    uint8_t _ultimoValor[HardwareMap::NUM_CONTROLES];
    uint8_t _ultimoValorRemoto[MAX_REMOTE_CONTROLS];
    uint32_t _ultimaLeitura = 0;

    /// Lê um controle analógico local e retorna valor 0-127.
    uint8_t lerControle(uint8_t indice);
};

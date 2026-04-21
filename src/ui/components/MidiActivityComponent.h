#pragma once

#include "ui/UIComponent.h"
#include <cstdint>

class Adafruit_SSD1306;

/**
 * Indicador visual de atividade MIDI.
 *
 * Exibe um pequeno quadrado que "pisca" brevemente quando
 * trigger() é chamado. Ideal para colocar no header de qualquer
 * tela como feedback visual de que mensagens MIDI estão sendo enviadas.
 *
 * Uso típico: o MidiEngine chama trigger() a cada envio,
 * e o OledApp renderiza o indicador como overlay sobre a tela ativa.
 */
class MidiActivityComponent : public UIComponent {
public:
    /**
     * @param x Posição horizontal (pixels)
     * @param y Posição vertical (pixels)
     * @param tamanho Lado do quadrado indicador (pixels, padrão 6)
     */
    MidiActivityComponent(int16_t x, int16_t y, uint8_t tamanho = 6);

    void render(Adafruit_SSD1306& display) override;

    /// Dispara o indicador — ele ficará visível por DURACAO_MS.
    void trigger();

    /// Retorna true se o indicador está ativo (piscando).
    bool isActive() const;

    /// Duração do "piscar" em milissegundos.
    static constexpr uint32_t DURACAO_MS = 100;

private:
    int16_t _x;
    int16_t _y;
    uint8_t _tamanho;
    uint32_t _triggerTime = 0;
    bool _active = false;
};

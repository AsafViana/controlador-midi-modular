#pragma once

#include "ui/Screen.h"
#include "ui/components/TextComponent.h"
#include "hardware/HardwareMap.h"

class Storage;
class OledApp;
class UnifiedControlList;

/**
 * Tela de endereçamento CC.
 *
 * Lista os controles do HardwareMap (e remotos, se disponíveis).
 * Para cada um mostra:
 *   - Label, CC atribuído, e status (ON/OFF)
 *   - Para remotos: prefixo "[XX]" com endereço I2C hex
 *
 * Navegação:
 *   - PRESSED:      Próximo controle
 *   - LONG_PRESS:   Controle anterior
 *   - SINGLE_CLICK: Entra no modo edição (cicla: CC → ON/OFF → sai)
 *   - DOUBLE_CLICK: Voltar
 *
 * No modo edição:
 *   Fase 1 (CC): UP/DOWN muda CC, SELECT confirma e vai para fase 2
 *   Fase 2 (ON/OFF): UP/DOWN alterna, SELECT confirma e sai
 *
 * Se UnifiedControlList for nullptr, comportamento idêntico ao original
 * (apenas controles locais do HardwareMap).
 */
class CCMapScreen : public Screen {
public:
    CCMapScreen(Storage* storage, UnifiedControlList* ucl = nullptr);

    void setApp(OledApp* app) { _app = app; }

    void handleInput(ButtonEvent event) override;
    void onMount() override;
    void render(Adafruit_SSD1306& display) override;

private:
    Storage* _storage;
    OledApp* _app;
    UnifiedControlList* _ucl;
    TextComponent _titulo;

    uint8_t _indice = 0;

    enum class ModoEdicao : uint8_t {
        NENHUM,     // Navegando na lista
        EDITAR_CC,  // Editando número CC
        EDITAR_ONOFF // Editando habilitado/desabilitado
    };
    ModoEdicao _modo = ModoEdicao::NENHUM;

    uint8_t _ccTemp = 0;
    bool _onOffTemp = true;

    char _lineBuf[4][24];

    /// Retorna o número total de controles (UCL se disponível, senão HardwareMap).
    uint8_t getTotalControles() const;

    /// Retorna o número de controles locais.
    uint8_t getNumLocais() const;

    /// Verifica se o índice corresponde a um controle remoto.
    bool isRemoto(uint8_t idx) const;

    /// Formata o label para exibição. Para remotos, inclui prefixo "[XX]".
    /// Escreve no buffer fornecido e retorna ponteiro para ele.
    const char* formatLabel(uint8_t idx, char* buf, uint8_t bufSize) const;

    /// Retorna o CC atual para o controle no índice (local ou remoto).
    uint8_t getCC(uint8_t idx) const;

    /// Define o CC para o controle no índice (local ou remoto).
    void setCC(uint8_t idx, uint8_t cc);

    /// Retorna se o controle está habilitado (local ou remoto).
    bool isHabilitado(uint8_t idx) const;

    /// Define se o controle está habilitado (local ou remoto).
    void setHabilitado(uint8_t idx, bool habilitado);

    /// Retorna o label base do controle (sem prefixo).
    const char* getBaseLabel(uint8_t idx) const;
};

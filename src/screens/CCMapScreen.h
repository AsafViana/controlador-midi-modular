#pragma once

#include "ui/Screen.h"
#include "ui/components/TextComponent.h"
#include "hardware/HardwareMap.h"

class Storage;
class OledApp;
class UnifiedControlList;

class CCMapScreen : public Screen {
public:
    CCMapScreen(Storage* storage, UnifiedControlList* ucl = nullptr);

    void setApp(OledApp* app);

    void handleInput(NavInput input) override;
    void onMount() override;
    void render(Adafruit_SSD1306& display) override;

private:
    Storage* _storage;
    OledApp* _app = nullptr;
    UnifiedControlList* _ucl;
    TextComponent _titulo;

    uint8_t _indice = 0;

    enum class ModoEdicao : uint8_t {
        NENHUM,
        EDITAR_CC,
        EDITAR_ONOFF
    };
    ModoEdicao _modo = ModoEdicao::NENHUM;

    uint8_t _ccTemp   = 0;
    bool    _onOffTemp = true;

    char _lineBuf[4][24];

    uint8_t     getTotalControles() const;
    uint8_t     getNumLocais()      const;
    bool        isRemoto(uint8_t idx) const;
    const char* formatLabel(uint8_t idx, char* buf, uint8_t bufSize) const;
    uint8_t     getCC(uint8_t idx)  const;
    void        setCC(uint8_t idx, uint8_t cc);
    bool        isHabilitado(uint8_t idx) const;
    void        setHabilitado(uint8_t idx, bool habilitado);
    const char* getBaseLabel(uint8_t idx) const;
};

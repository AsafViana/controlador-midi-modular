#pragma once

#include <stdint.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "button/Button.h"
#include "ui/NavInput.h"
#include "ui/UIComponent.h"

class Screen {
public:
    virtual ~Screen() = default;

    virtual void onMount()  {}
    virtual void onUnmount() {}
    virtual void render(Adafruit_SSD1306& display) = 0;
    virtual void handleInput(NavInput input) {}

    void markDirty()  { _dirty = true; }
    void clearDirty() { _dirty = false; }
    bool isDirty()    const { return _dirty; }

protected:
    void addChild(UIComponent* c) {
        if (_childCount < MAX_CHILDREN) _children[_childCount++] = c;
    }
    void renderChildren(Adafruit_SSD1306& display) {
        for (uint8_t i = 0; i < _childCount; i++) _children[i]->render(display);
    }

private:
    bool _dirty = true;
    static constexpr uint8_t MAX_CHILDREN = 8;
    UIComponent* _children[MAX_CHILDREN] = {};
    uint8_t _childCount = 0;
};

#include "ui/Screen.h"
#include "ui/UIComponent.h"

void Screen::render(Adafruit_SSD1306& display) {
    for (uint8_t i = 0; i < _childCount; i++) {
        _children[i]->render(display);
    }
}

bool Screen::addChild(UIComponent* child) {
    if (child == nullptr || _childCount >= MAX_CHILDREN) {
        return false;
    }
    _children[_childCount++] = child;
    return true;
}

uint8_t Screen::getChildCount() const {
    return _childCount;
}

void Screen::markDirty() {
    _dirty = true;
}

bool Screen::isDirty() const {
    return _dirty;
}

void Screen::clearDirty() {
    _dirty = false;
}

#include "ui/components/ListComponent.h"
#include "Adafruit_SSD1306.h"

ListComponent::ListComponent(int16_t x, int16_t y, int16_t w, int16_t h,
                             uint8_t fontSize)
    : _x(x), _y(y), _w(w), _h(h), _fontSize(fontSize) {
    // Calculate visible count based on item height
    uint8_t itemHeight = _fontSize * 8;
    _visibleCount = (itemHeight > 0) ? (_h / itemHeight) : 0;
}

void ListComponent::render(Adafruit_SSD1306& display) {
    // Nothing to render if no items
    if (_items == nullptr || _itemCount == 0) {
        return;
    }

    uint8_t itemHeight = _fontSize * 8;
    // Recalculate visible count in case it wasn't set
    _visibleCount = (itemHeight > 0) ? (_h / itemHeight) : 0;

    if (_visibleCount == 0) {
        return;
    }

    display.setTextSize(_fontSize);

    // Determine how many items to render
    uint8_t endIndex = _scrollOffset + _visibleCount;
    if (endIndex > _itemCount) {
        endIndex = _itemCount;
    }

    for (uint8_t i = _scrollOffset; i < endIndex; i++) {
        int16_t itemY = _y + (i - _scrollOffset) * itemHeight;

        // Treat nullptr items as empty string
        const char* text = (_items[i] != nullptr) ? _items[i] : "";

        if (i == _selectedIndex) {
            // Selected item: draw highlight background (inverse)
            display.fillRect(_x, itemY, _w, itemHeight, SSD1306_WHITE);
            display.setTextColor(SSD1306_BLACK);
        } else {
            // Non-selected item: normal text
            display.setTextColor(SSD1306_WHITE);
        }

        display.setCursor(_x, itemY);
        display.print(text);
    }
}

bool ListComponent::handleInput(ButtonEvent event) {
    // Ignore if no items or event is NONE
    if (_itemCount == 0 || event == ButtonEvent::NONE) {
        return false;
    }

    if (event == _downEvent) {
        if (_selectedIndex < _itemCount - 1) {
            _selectedIndex++;
            scrollToSelected();
            if (_onSelectionChanged) {
                _onSelectionChanged(_selectedIndex);
            }
        }
        return true;
    }

    if (event == _upEvent) {
        if (_selectedIndex > 0) {
            _selectedIndex--;
            scrollToSelected();
            if (_onSelectionChanged) {
                _onSelectionChanged(_selectedIndex);
            }
        }
        return true;
    }

    return false;
}

void ListComponent::setItems(const char** items, uint8_t count) {
    _items = items;
    _itemCount = count;

    if (_itemCount == 0) {
        _selectedIndex = 0;
        _scrollOffset = 0;
    } else if (_selectedIndex >= _itemCount) {
        // Clamp selectedIndex to valid range
        _selectedIndex = _itemCount - 1;
        scrollToSelected();
    }
}

uint8_t ListComponent::getSelectedIndex() const {
    return _selectedIndex;
}

void ListComponent::setUpButton(ButtonEvent upEvent) {
    _upEvent = upEvent;
}

void ListComponent::setDownButton(ButtonEvent downEvent) {
    _downEvent = downEvent;
}

void ListComponent::onSelectionChanged(OnSelectionChanged callback) {
    _onSelectionChanged = callback;
}

void ListComponent::scrollToSelected() {
    // If selected item is below the visible window, scroll down
    if (_selectedIndex >= _scrollOffset + _visibleCount) {
        _scrollOffset = _selectedIndex - _visibleCount + 1;
    }
    // If selected item is above the visible window, scroll up
    if (_selectedIndex < _scrollOffset) {
        _scrollOffset = _selectedIndex;
    }
}

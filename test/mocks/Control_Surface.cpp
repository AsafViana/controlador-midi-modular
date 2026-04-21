#include "Control_Surface.h"

namespace mock_midi {
    MidiMessage lastMessage = {0, 0, 0, false, 0, 0, false};
    int         messageCount = 0;

    void reset() {
        lastMessage = {0, 0, 0, false, 0, 0, false};
        messageCount = 0;
    }
}

ControlSurfaceMock Control_Surface;

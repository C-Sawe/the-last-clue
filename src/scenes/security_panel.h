#pragma once
// Security panel and terminal art — Alvin (final build, issue #60)
//
// The study door has an electronic lock. Its wiring panel hangs on the wall
// beside the door; inside, three coloured TERMINALS on the left must be wired
// to the three matching SOCKETS on the right, without any wires crossing.
// The player writes the wires as a ClueScript program on the panel's little
// terminal, so the layout below is in PROGRAM coordinates (the 300 x 300
// preview screen). Sudheysi's validator (game/code_check) uses the same data.
#include <vector>

#include "../core/canvas.h"
#include "../transform/transform2d.h"

struct WirePort {
    PointF pos;            // program coordinates
    Color colour;          // exactly ClueScript's named colour, so `color red` matches
    const char* name;      // "RED", "GREEN", "BLUE"
};

const double PortRadius = 12;                      // sockets accept a wire end within this radius
const std::vector<WirePort>& wireTerminals();      // left side: where each wire starts
const std::vector<WirePort>& wireSockets();        // right side: where each wire must end

// The inside of the panel under the player's wires: back plate, terminals,
// sockets, labels. `powered` lights the status LED green.
void drawWiringBoard(Canvas& c, const Mat3& toScreen, bool powered);

// A monitor / terminal bezel around a screen area (the laptop close-up and the
// panel's terminal share it). `on` switches the power LED.
void drawMonitorBezel(Canvas& c, double x, double y, double w, double h, const char* label, bool on);

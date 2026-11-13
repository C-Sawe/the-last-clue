// Demo (final build, issue #50): fingerprint matching.
// The print lifted from the safe is turned back from 60 degrees to 0 (inverse-
// mapped rotation) while the overlap with each suspect's print is measured as
// Intersection over Union. Only the partner's print passes the 53% mark.
// Output: output/34_fingerprint_match.gif (+ .bmp key frames), and a table of
//         scores on the console.
#include <cmath>
#include <iomanip>

#include "demo_common.h"
#include "game/puzzles.h"

int main() {
    const double PI = 3.14159265358979;
    std::cout << "angle   butler  heiress  partner\n";
    for (int d = 60; d >= 0; d -= 15) {
        std::cout << std::setw(4) << d << "    ";
        for (int s = 0; s < 3; ++s) std::cout << std::fixed << std::setprecision(2) << game::printMatchScore(s, d * PI / 180) << "    ";
        std::cout << "\n";
    }
    // The puzzle screen: pick the partner, then turn the print back to 0 degrees.
    game::PrintsPuzzle puzzle;
    puzzle.select(2);
    return demo::animate("34_fingerprint_match", 800, 500, 36, [&](Canvas& c, double t) {
        const double steps = std::round((1 - std::min(1.0, t * 1.15)) * 12);   // 60 -> 0 degrees in 5-degree steps
        puzzle.setAngle(steps * PI / 36);
        if (t >= 1) puzzle.compare();
        puzzle.render(c);
    }, 24);
}

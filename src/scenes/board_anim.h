#pragma once
// Evidence board animation — Latiffa (CP2, issue #31)
#include "../core/canvas.h"

// progress 0 -> 1:
//   0.00 - 0.55  cards and photos drop onto the board one after another
//                (translate + OutBack easing, staggered start times)
//   0.55 - 1.00  each string is pulled from pin to pin (partial Bézier)
// progress = 1 gives exactly the static board from CP1.
void renderEvidenceBoardAt(Canvas& c, double progress);   // 900 x 560

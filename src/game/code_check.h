#pragma once
// Code puzzle validation — Sudheysi (CP3, issue #59)
//
// Puzzle B (the safe's alarm): the player must make the victim's program draw
// her star symbol. We don't compare the CODE (many programs draw the same star);
// we compare the PICTURE:
//   1. rasterise the player's drawing and the target drawing into two masks,
//      with thick lines (so 1-pixel rounding differences don't matter),
//   2. Intersection over Union:  IoU = |A and B| / |A or B|,
//   3. pass when IoU >= 0.85.
#include <string>
#include <vector>

#include "script/script.h"

namespace game {

const char* starStarterCode();              // the broken program left on the laptop
const char* starHint();                     // shown after three failed runs
script::RunOptions puzzleRunOptions();      // turtle start position for this puzzle

struct CodeCheck {
    bool pass = false;
    double iou = 0;
    std::string message;
};

CodeCheck checkStar(const script::RunResult& result);

// Exposed for demos and tests
std::vector<bool> rasterise(const std::vector<script::DrawCmd>& drawing, int size = 300, int thickness = 7);
double intersectionOverUnion(const std::vector<bool>& a, const std::vector<bool>& b);
const std::vector<script::DrawCmd>& targetStar();

}  // namespace game

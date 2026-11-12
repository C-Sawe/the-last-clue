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
//
// Final build (issue #62): two more code puzzles, and one descriptor type so
// the same terminal (Latiffa's editor) can run any of them.
//   Puzzle A (security panel wiring): every wire must run from its coloured
//     terminal to the matching socket (point-in-polygon on the socket) and no
//     two wires of different colours may cross (segment-intersection test).
//   Puzzle C (encrypted email): the program must print the decoded message
//     (the email was Caesar-shifted; `shift` undoes it). String comparison.
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

// ---- Final build: every code puzzle described the same way --------------------
struct CodePuzzle {
    std::string file;                        // tab name, e.g. "ALARM_KEY.CLUE"
    std::string goal;                        // what the console says before the first run
    std::string starter;                     // the broken program the player starts from
    std::string hint;                        // shown after three failed runs
    script::RunOptions options;
    CodeCheck (*check)(const script::RunResult&) = nullptr;
    // Optional art under the player's drawing (program -> screen mapping), e.g. the wiring board.
    void (*background)(Canvas&, const Mat3& toScreen, bool solved) = nullptr;
    bool showsText = false;                  // the preview shows printed text instead of a drawing
};

CodePuzzle starPuzzle();                     // B: the safe's alarm (CP3)
CodePuzzle wiringPuzzle();                   // A: the door's security panel
CodePuzzle emailPuzzle();                    // C: the victim's scrambled email

// Puzzle A
CodeCheck checkWiring(const script::RunResult& result);
bool segmentsCross(PointF a, PointF b, PointF c, PointF d);   // touching counts as crossing

// Puzzle C
const char* emailCipherText();
const char* emailPlainText();
CodeCheck checkEmail(const script::RunResult& result);

// Exposed for demos and tests
std::vector<bool> rasterise(const std::vector<script::DrawCmd>& drawing, int size = 300, int thickness = 7);
double intersectionOverUnion(const std::vector<bool>& a, const std::vector<bool>& b);
const std::vector<script::DrawCmd>& targetStar();

}  // namespace game

#pragma once
// Project scenes — each builds a component of "The Last Clue" from our own algorithms.
//   Alvin    — renderCrimeScene, renderCharacterLineup
//   Sudheysi — renderSafeDial, renderTornDocument, renderClippingDemo
//   Latiffa  — renderEvidenceBoard, renderCaseFilePanel
// Each function draws into the whole canvas it is given (recommended sizes below).
#include "../core/canvas.h"

// Colour coding shared by every scene: each suspect keeps one colour everywhere.
namespace SuspectColour {
    const Color Detective {125,  90,  55};   // trench-coat brown
    const Color Butler    {178,  45,  50};   // suspect 1 — red
    const Color Heiress   { 46, 139,  87};   // suspect 2 — green
    const Color Partner   { 52,  94, 168};   // suspect 3 — blue
    const Color Highlight {255, 214,  60};   // interactive objects
}

// Final build (issue #51): one palette for feedback and evidence, so every
// screen says "right", "wrong" and "new" in the same colours.
namespace Feedback {
    const Color Correct {100, 210, 120};     // solved, accepted, matched
    const Color Wrong   {235, 100,  90};     // rejected, short circuit, no match
    const Color Info    {180, 184, 200};     // neutral console text
    const Color Hint    {255, 214,  60};     // hints = the highlight yellow
    const Color Panel   { 20,  20,  26};     // dark tag / toast background
}

// Evidence categories: what each clue tells the detective.
namespace EvidenceInk {
    const Color Document {120, 140, 200};    // letters, codes, email
    const Color Physical {220, 185,  70};    // key, vial
    const Color Forensic {200,  80, 120};    // fingerprint
}

void renderCrimeScene(Canvas& c);                         // 800 x 500
void renderCharacterLineup(Canvas& c);                    // 900 x 460
void renderSafeDial(Canvas& c, double dialAngleRadians);  // 420 x 420
void renderTornDocument(Canvas& c, bool solved);          // 600 x 480
void renderClippingDemo(Canvas& c);                       // 1000 x 480
void renderEvidenceBoard(Canvas& c);                      // 900 x 560
void renderCaseFilePanel(Canvas& c);                      // 250 x 500

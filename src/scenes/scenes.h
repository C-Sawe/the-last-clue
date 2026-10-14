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

void renderCrimeScene(Canvas& c);                         // 800 x 500
void renderCharacterLineup(Canvas& c);                    // 900 x 460
void renderSafeDial(Canvas& c, double dialAngleRadians);  // 420 x 420
void renderTornDocument(Canvas& c, bool solved);          // 600 x 480
void renderClippingDemo(Canvas& c);                       // 1000 x 480
void renderEvidenceBoard(Canvas& c);                      // 900 x 560
void renderCaseFilePanel(Canvas& c);                      // 250 x 500

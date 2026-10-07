#pragma once
// Project scenes — each builds a component of "The Last Clue" from our own algorithms.
//   Alvin    — renderCrimeScene, renderCharacterLineup
//   Sudheysi — renderSafeDial, renderTornDocument, renderClippingDemo
//   Latiffa  — renderEvidenceBoard, renderCaseFilePanel
#include "../core/canvas.h"

void renderCrimeScene(Canvas& c);
void renderCharacterLineup(Canvas& c);
void renderSafeDial(Canvas& c, double dialAngleRadians);
void renderTornDocument(Canvas& c);
void renderClippingDemo(Canvas& c);
void renderEvidenceBoard(Canvas& c);
void renderCaseFilePanel(Canvas& c);

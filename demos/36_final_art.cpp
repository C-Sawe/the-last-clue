// Demo (final build, issues #49 #60 #51): the new art for the last puzzles.
//   top    - the fingerprint line-up: three suspect prints built from midpoint
//            ellipses (arch / whorl / loop) and the print lifted from the safe,
//            shown rotated by inverse mapping
//   bottom - the security panel's wiring board, and the study with the door
//            open (back room + vial), the panel rewired and the alarm off
// Output: output/36_final_art.bmp
#include "demo_common.h"
#include "polygon/polygon.h"
#include "scenes/fingerprint.h"
#include "scenes/scene_anim.h"
#include "scenes/scenes.h"
#include "scenes/security_panel.h"

int main() {
    Canvas c(1200, 760, Color{36, 38, 48});
    demo::title(c, "FINAL ART: FINGERPRINTS, SECURITY PANEL, BACK ROOM", "MIDPOINT ELLIPSES + INVERSE-MAPPED ROTATION");

    // Fingerprint line-up
    for (int i = 0; i < 3; ++i) drawPrintCard(c, 30 + i * 175, 70, i, i == 2, false);
    const Canvas found = renderPrint(foundPrint(), 90, Color{170, 40, 50}, Palette::White);
    scanlineFill(c, makeRect(600, 64, 250, 210), Color{248, 246, 238});
    for (int k = 0; k < 4; ++k)                                     // the lifted print at four angles
        drawPrintRotated(c, found, {663.0 + (k % 2) * 124, 118.0 + (k / 2) * 104}, -1.2 + k * 0.4, Palette::White);
    demo::caption(c, 610, 282, "LIFTED PRINT, ROTATED (INVERSE MAP)", Palette::White);
    demo::caption(c, 30, 282, "SUSPECT PRINTS: ARCH, WHORL, LOOP + SCAR", Palette::White);

    // Wiring board at preview scale
    drawWiringBoard(c, translate(880, 64) * scale(0.95, 0.95), false);
    demo::caption(c, 880, 356, "WIRING PANEL: RED, GREEN, BLUE", Palette::White);

    // The study, everything solved
    Canvas room(800, 500);
    SceneState s;
    s.door = 1;
    s.panelFixed = true;
    s.alarmOff = true;
    renderCrimeSceneState(room, s);
    for (int y = 0; y < 375; ++y)
        for (int x = 0; x < 600; ++x) c.setPixel(30 + x, 375 + y, room.getPixel(x * 4 / 3, y * 4 / 3));
    demo::caption(c, 640, 400, "THE STUDY, FINAL STATE:", Palette::White);
    demo::caption(c, 640, 416, "- DOOR OPEN: BACK ROOM + EMPTY VIAL", Feedback::Info);
    demo::caption(c, 640, 432, "- PANEL LED GREEN (REWIRED)", Feedback::Correct);
    demo::caption(c, 640, 448, "- SAFE ALARM LED GREEN", Feedback::Correct);

    // Shared feedback palette (#51)
    const struct { Color col; const char* name; } swatches[] = {
        {Feedback::Correct, "CORRECT"}, {Feedback::Wrong, "WRONG"}, {Feedback::Info, "INFO"}, {Feedback::Hint, "HINT"},
        {EvidenceInk::Document, "DOCUMENT"}, {EvidenceInk::Physical, "PHYSICAL"}, {EvidenceInk::Forensic, "FORENSIC"},
        {SuspectColour::Butler, "BUTLER"}, {SuspectColour::Heiress, "HEIRESS"}, {SuspectColour::Partner, "PARTNER"}};
    for (int i = 0; i < 10; ++i) {
        const int x = 640 + (i % 2) * 270, y = 490 + (i / 2) * 50;
        scanlineFill(c, makeRect(x, y, 40, 36), swatches[i].col);
        drawPolygonOutline(c, makeRect(x, y, 40, 36), Palette::Black);
        demo::caption(c, x + 50, y + 14, swatches[i].name, Palette::White);
    }
    return demo::save(c, "36_final_art");
}

// Demo: the evidence board.  Output: output/15_evidence_board.bmp
#include "demo_common.h"
#include "scenes/scenes.h"

int main() {
    Canvas c(900, 560);
    renderEvidenceBoard(c);
    return demo::save(c, "15_evidence_board");
}

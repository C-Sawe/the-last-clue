// Demo (CP3, issue #45): the live Case File panel and the interactive evidence
// board, driven by scripted input: evidence arrives, a card is dragged, and
// strings are tied pin-to-pin (Bézier) — then shown next to the Case File.
// Output: output/30_board_ui.gif / .bmp (final frame: panel + board)
#include <iostream>

#include "demo_common.h"
#include "ui/case_file_ui.h"

int main() {
    game::GameState st;
    st.screen = game::Screen::Board;
    ui::BoardUI board;
    ui::CaseFileUI panel;
    GifWriter gif(1050, 500, 6);
    Canvas frame(1050, 500), left(250, 500), right(800, 500);
    auto show = [&](int hold = 1) {
        st.update(0.06 * hold);
        panel.render(left, st);
        board.render(right, st);
        frame.blit(left, 0, 0);
        frame.blit(right, 250, 0);
        gif.addFrame(frame, hold);
    };

    show(6);
    for (auto e : {game::Evidence::TornLetter, game::Evidence::SafeCode, game::Evidence::Key, game::Evidence::Fingerprint}) {
        st.addEvidence(e);                                  // evidence found -> slot fills, card appears
        board.sync(st);
        show(6);
    }
    // Drag the fingerprint card towards the Partner
    const int fp = 10 + static_cast<int>(game::Evidence::Fingerprint);
    const PointF from = board.cardCentre(fp), to{560, 370};
    board.mouseDown(from, st);
    for (int s = 1; s <= 10; ++s) {
        board.mouseMove({from.x + (to.x - from.x) * s / 10, from.y + (to.y - from.y) * s / 10});
        show();
    }
    board.mouseUp(to);
    // Tie strings: victim-butler, letter-butler, print-partner, code-key
    const int pairs[][2] = {{0, 1}, {10, 1}, {fp, 3}, {11, 12}};
    for (const auto& p : pairs) {
        board.mouseMove(board.pinOf(p[0]));
        board.mouseDown(board.pinOf(p[0]), st);
        show(2);
        board.mouseMove(board.pinOf(p[1]));
        show(2);
        board.mouseDown(board.pinOf(p[1]), st);
        show(4);
    }
    show(20);
    std::cout << "Strings on the board: " << st.links.size() << "\n";
    if (!gif.save("output/30_board_ui.gif")) return 1;
    std::cout << "Wrote output/30_board_ui.gif (" << gif.frameCount() << " frames)\n";
    return demo::save(frame, "30_board_ui");
}

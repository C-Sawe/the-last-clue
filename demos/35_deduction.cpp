// Demo (final build, issue #48): the deduction rules.
// Three accusations are judged against the truth: two wrong ones (each costs a
// try) and the right one. A third wrong accusation would close the case.
// Output: console report + output/35_deduction.bmp (the answer table)
#include "demo_common.h"
#include "game/deduction.h"
#include "polygon/polygon.h"

int main() {
    game::GameState st;
    for (const char* f : {"solved:print", "found:vial", "solved:email"}) st.set(f);

    struct Try { const char* label; game::Accusation a; };
    Try tries[3];
    tries[0].label = "BUTLER / STRUCK / REVENGE";
    tries[0].a.pick[0] = 0; tries[0].a.pick[1] = 0; tries[0].a.pick[2] = 1;
    tries[1].label = "PARTNER / POISON / THE WILL";
    tries[1].a.pick[0] = 2; tries[1].a.pick[1] = 1; tries[1].a.pick[2] = 0;
    tries[2].label = "PARTNER / POISON / STOLEN MONEY";
    tries[2].a.pick[0] = 2; tries[2].a.pick[1] = 1; tries[2].a.pick[2] = 2;

    Canvas c(1000, 420, Palette::White);
    demo::title(c, "FINAL DEDUCTION RULES", "WHO / HOW / WHY - 3 TRIES");
    for (int q = 0; q < game::QuestionCount; ++q) {
        const auto question = static_cast<game::Question>(q);
        demo::caption(c, 20 + q * 330, 50, game::questionName(question), Palette::Black);
        for (int i = 0; i < game::AnswerCount; ++i) {
            const game::Answer& a = game::answer(question, i);
            const bool backed = game::answerSupported(st, question, i);
            const int x = 20 + q * 330, y = 66 + i * 52;
            scanlineFill(c, makeRect(x, y, 310, 46), backed ? Color{220, 245, 225} : Color{242, 242, 246});
            drawPolygonOutline(c, makeRect(x, y, 310, 46), backed ? Color{40, 150, 70} : Palette::Grey);
            demo::caption(c, x + 8, y + 7, a.title, Palette::Black);
            demo::caption(c, x + 8, y + 20, a.detail, demo::Muted);
            demo::caption(c, x + 8, y + 33, backed ? a.support : "NOT BACKED BY EVIDENCE", backed ? Color{30, 130, 60} : demo::Muted);
        }
    }
    for (int k = 0; k < 3; ++k) {
        const game::Verdict v = game::judge(st, tries[k].a);
        const int y = 240 + k * 56;
        std::cout << tries[k].label << " -> " << (v.solved ? "CASE SOLVED" : "WRONG") << "  (" << v.correct << "/3 right, "
                  << v.triesLeft << " tries left)\n";
        demo::caption(c, 20, y, std::string("ACCUSATION ") + std::to_string(k + 1) + ": " + tries[k].label, Palette::Black);
        demo::caption(c, 20, y + 14, v.solved ? "CASE SOLVED" : (std::to_string(v.correct) + " OF 3 RIGHT - " + std::to_string(v.triesLeft) + " TRIES LEFT"),
                      v.solved ? Color{30, 140, 60} : Palette::Red);
        if (!v.lines.empty()) demo::caption(c, 20, y + 28, v.lines[0], demo::Muted);
    }
    return demo::save(c, "35_deduction");
}

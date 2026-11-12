// Demo (final build, issues #48 #52): the deduction screen and the outcomes.
//   GIF   - the cards drop in, three answers are picked (cards lift), ACCUSE,
//           then the CASE SOLVED sequence: the stamp slams down (scale + rotation
//           by inverse mapping, OutBack), the story types out, the mug shot
//           slides in.
//   Sheet - the three outcomes side by side: solved, wrong accusation, failed.
// Output: output/37_final_ui.gif, output/37_final_ui.bmp, output/37_outcomes.bmp
#include "demo_common.h"
#include "ui/deduction_ui.h"

int main() {
    game::GameState st;
    for (const char* f : {"solved:print", "found:vial", "solved:email"}) st.set(f);

    ui::DeductionUI ded;
    ui::OutcomeUI out;
    const double dt = 1.0 / 15;
    GifWriter gif(800, 500, 7);
    Canvas sheet(800, 500, Color{30, 32, 40});
    int frame = 0;
    auto shot = [&](Canvas& c) {
        gif.addFrame(c);
        if (frame == 16 || frame == 40 || frame == 64 || frame == 110) {      // key frames for the report
            const int k = frame == 16 ? 0 : frame == 40 ? 1 : frame == 64 ? 2 : 3;
            for (int y = 0; y < 250; ++y)
                for (int x = 0; x < 400; ++x) sheet.setPixel((k % 2) * 400 + x, (k / 2) * 250 + y, c.getPixel(x * 2, y * 2));
        }
        ++frame;
    };
    Canvas c(800, 500);
    for (int i = 0; i < 20; ++i) { ded.update(dt); ded.render(c, st); shot(c); }              // drop in
    const int picks[3] = {2, 1, 2};
    for (int q = 0; q < 3; ++q) {
        ded.mouseMove(ded.cardCentre(q, picks[q]));
        ded.mouseDown(ded.cardCentre(q, picks[q]));
        for (int i = 0; i < 6; ++i) { ded.update(dt); ded.render(c, st); shot(c); }
    }
    ded.mouseMove(ded.accuseCentre());
    for (int i = 0; i < 4; ++i) { ded.update(dt); ded.render(c, st); shot(c); }
    if (ded.mouseDown(ded.accuseCentre()) == ui::DeductionUI::Action::Accuse) out.show(game::judge(st, ded.accusation()));
    for (int i = 0; i < 70; ++i) { out.update(dt); out.render(c); shot(c); }
    gif.addFrame(c, 30);
    if (!gif.save("output/37_final_ui.gif")) return 1;
    std::cout << "Wrote output/37_final_ui.gif (" << gif.frameCount() << " frames)\n";
    std::cout << "Verdict: " << (out.verdict().solved ? "CASE SOLVED" : "not solved") << "\n";
    demo::save(sheet, "37_final_ui");

    // The three outcomes, 3.5 s into each sequence
    Canvas three(1230, 330, Color{30, 32, 40});
    demo::title(three, "OUTCOMES", "SOLVED / WRONG ACCUSATION / FAILED");
    game::GameState s2;
    game::Accusation wrong;
    wrong.pick[0] = 0; wrong.pick[1] = 1; wrong.pick[2] = 0;
    game::Accusation right;
    right.pick[0] = 2; right.pick[1] = 1; right.pick[2] = 2;
    const game::Verdict verdicts[3] = {game::judge(s2, right), game::judge(s2, wrong), (game::judge(s2, wrong), game::judge(s2, wrong))};
    for (int k = 0; k < 3; ++k) {
        ui::OutcomeUI o;
        o.show(verdicts[k]);
        for (int i = 0; i < 55; ++i) o.update(dt);
        Canvas v(800, 500);
        o.render(v);
        for (int y = 0; y < 286; ++y)
            for (int x = 0; x < 400; ++x) three.setPixel(10 + k * 407 + x, 40 + y, v.getPixel(x * 2, y * 7 / 4));
    }
    return demo::save(three, "37_outcomes");
}

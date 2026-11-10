// Demo (CP3, issue #44): the playable crime scene with scripted mouse input.
// Hover the safe (name tag), click the door (locked), click the drawer: the
// detective walks over, the drawer slides out and the letter close-up opens.
// Then the painting close-up swings aside (as if the letter were solved).
// Output: output/31_explore.gif / .bmp (key frames)
#include <iostream>

#include "demo_common.h"
#include "game/explore.h"
#include "scenes/scenes.h"
#include "ui/text.h"

int main() {
    game::GameState st;
    game::ExploreScreen explore;
    game::PaintingView painting;
    GifWriter gif(800, 500, 5);
    std::vector<Canvas> keys;
    Canvas c(800, 500);
    const double dt = 1.0 / 20;
    auto show = [&](int frames) {
        for (int i = 0; i < frames; ++i) {
            explore.update(dt, st);
            st.update(dt);
            if (st.screen != game::Screen::Painting) explore.render(c, st);
            else { painting.update(dt, st); painting.render(c); }
            if (const game::Hotspot* h = explore.hovered(); h && st.screen != game::Screen::Painting)
                drawPolygonOutline(c, h->area, SuspectColour::Highlight);   // (the game draws this with OpenGL)
            ui::toastBar(c, 0, 470, 800, st.toast);
            gif.addFrame(c);
        }
    };

    show(8);
    explore.mouseMove({90, 290});                 // hover the safe
    show(10);
    keys.push_back(c);
    explore.mouseMove({686, 220});
    explore.click({686, 220}, st);                // door: locked
    show(24);
    keys.push_back(c);
    explore.mouseMove({454, 281});
    explore.click({454, 281}, st);                // drawer: walk, open, close-up requested
    while (st.screen == game::Screen::Explore) show(1);
    keys.push_back(c);
    std::cout << "After the drawer the game asks for: " << game::screenName(st.screen) << "\n";

    st.set("solved:letter");                      // pretend the letter puzzle was solved
    st.screen = game::Screen::Painting;
    painting.enter();
    show(40);
    keys.push_back(c);
    std::cout << "Evidence: " << st.evidence.size() << " (" << (st.hasEvidence(game::Evidence::SafeCode) ? "safe code found" : "-") << ")\n";

    if (!gif.save("output/31_explore.gif")) return 1;
    std::cout << "Wrote output/31_explore.gif (" << gif.frameCount() << " frames)\n";
    Canvas sheet(800, 500, Color{30, 32, 40});
    for (size_t k = 0; k < 4; ++k)
        for (int y = 0; y < 250; ++y)
            for (int x = 0; x < 400; ++x)
                sheet.setPixel(static_cast<int>(k % 2) * 400 + x, static_cast<int>(k / 2) * 250 + y, keys[k].getPixel(x * 2, y * 2));
    return demo::save(sheet, "31_explore");
}

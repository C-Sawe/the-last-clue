// Demo (CP3, issues #46/#61): the victim's laptop, driven by scripted input.
// RUN the broken program (pentagon, rejected) -> click into line 5 -> fix
// "72" to "144" by typing -> RUN again (star, accepted).
// Output: output/29_laptop_editor.gif / .bmp (key frames)
#include <iostream>
#include <string>

#include "demo_common.h"
#include "ui/code_editor.h"

int main() {
    ui::LaptopScreen laptop;
    GifWriter gif(800, 500, 6);
    std::vector<Canvas> keys;
    Canvas c(800, 500);
    auto show = [&](int hold = 1) {
        for (int i = 0; i < hold; ++i) { laptop.update(0.06); laptop.render(c); gif.addFrame(c); }
    };

    show(10);
    keys.push_back(c);
    laptop.mouseDown({90, 460});                          // RUN (broken code)
    show(14);
    keys.push_back(c);
    laptop.mouseDown({250, 50 + 6 + 4 * 18 + 8});         // click into line 5
    laptop.key(input::Key::End);
    show(4);
    for (int i = 0; i < 2; ++i) { laptop.key(input::Key::Backspace); show(3); }
    for (char ch : std::string("144")) { laptop.type(static_cast<unsigned>(ch)); show(3); }
    keys.push_back(c);
    show(6);
    laptop.mouseDown({90, 460});                          // RUN again
    show(20);
    keys.push_back(c);
    std::cout << "Solved: " << (laptop.solved() ? "yes" : "NO") << "  (" << laptop.takeMessage() << ")\n";
    std::cout << "Code now:\n" << laptop.editor().text() << "\n";

    if (!gif.save("output/29_laptop_editor.gif")) return 1;
    std::cout << "Wrote output/29_laptop_editor.gif (" << gif.frameCount() << " frames)\n";
    Canvas sheet(800, 500, Color{30, 32, 40});
    for (size_t k = 0; k < 4; ++k)
        for (int y = 0; y < 250; ++y)
            for (int x = 0; x < 400; ++x)
                sheet.setPixel(static_cast<int>(k % 2) * 400 + x, static_cast<int>(k / 2) * 250 + y, keys[k].getPixel(x * 2, y * 2));
    return demo::save(sheet, "29_laptop_editor");
}

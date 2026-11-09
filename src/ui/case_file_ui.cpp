// Interactive Case File panel and evidence board — Latiffa (CP3, issue #45)
#include "case_file_ui.h"

#include <algorithm>
#include <cmath>

#include "core/font.h"
#include "curves/curves.h"
#include "polygon/polygon.h"
#include "primitives/primitives.h"
#include "scenes/scenes.h"

namespace ui {

namespace {
const Color PanelBg{34, 36, 46}, Section{48, 51, 64}, TextCol{230, 230, 236}, Dim{140, 144, 160};

void box(Canvas& c, double x, double y, double w, double h, Color fill, Color edge) {
    scanlineFill(c, makeRect(x, y, w, h), fill);
    drawPolygonOutline(c, makeRect(x, y, w, h), edge);
}

// Small icon for each kind of evidence, drawn with our primitives.
void evidenceIcon(Canvas& c, game::Evidence e, int x, int y, int s, Color ink) {   // s = icon size
    const Color gold{220, 185, 70};
    switch (e) {
        case game::Evidence::TornLetter:
            scanlineFill(c, Polygon{{{x + s * 0.2, y + s * 0.15}, {x + s * 0.8, y + s * 0.15}, {x + s * 0.72, y + s * 0.5},
                                     {x + s * 0.8, y + s * 0.85}, {x + s * 0.2, y + s * 0.85}}}, Palette::Paper);
            for (int k = 0; k < 4; ++k)
                drawLineBresenham(c, {x + s / 4, y + s / 3 + k * s / 9}, {x + s * 3 / 5, y + s / 3 + k * s / 9}, Color{90, 90, 140});
            break;
        case game::Evidence::SafeCode:
            box(c, x + s * 0.15, y + s * 0.3, s * 0.7, s * 0.4, Palette::Paper, Palette::Grey);
            drawText(c, x + s / 2 - textWidth("37-12-85") / 2, y + s / 2 - 3, "37-12-85", Palette::Red);
            break;
        case game::Evidence::Key:
            drawCircleMidpoint(c, {x + s / 3, y + s / 2}, s / 7, gold);
            drawThickLine(c, {x + s / 3 + s / 7, y + s / 2}, {x + s * 4 / 5, y + s / 2}, 3, gold);
            drawThickLine(c, {x + s * 7 / 10, y + s / 2}, {x + s * 7 / 10, y + s / 2 + s / 8}, 2, gold);
            break;
        case game::Evidence::Fingerprint:
            for (int k = 0; k < 5; ++k) drawEllipseMidpoint(c, {x + s / 2, y + s / 2}, 2 + k * s / 14, 3 + k * s / 11, ink);
            break;
    }
}
}  // namespace

// =============================== Case File ===============================
CaseFileUI::CaseFileUI() {
    board_ = {12, 400, 226, 30, "EVIDENCE BOARD"};
    deduce_ = {12, 436, 226, 30, "MAKE DEDUCTION"};
    back_ = {12, 472, 226, 22, "BACK TO THE STUDY"};
}

CaseFileUI::Action CaseFileUI::click(PointF p, const game::GameState& st) const {
    if (board_.contains(p)) return Action::Board;
    if (deduce_.contains(p)) return Action::Deduction;
    if (back_.contains(p) && st.screen != game::Screen::Explore) return Action::Back;
    return Action::None;
}

void CaseFileUI::render(Canvas& c, const game::GameState& st) const {
    c.clear(PanelBg);
    drawText(c, 14, 14, "CASE FILE", Palette::White, 3);
    drawText(c, 14, 42, std::string("NOW: ") + game::screenName(st.screen), Dim);
    drawLineBresenham(c, {12, 56}, {237, 56}, Dim);

    // Suspects (status updates as the case develops)
    drawText(c, 14, 66, "SUSPECTS", SuspectColour::Highlight);
    struct Row { Color col; const char* name; const char* note; };
    const Row rows[] = {
        {SuspectColour::Butler, "THE BUTLER", st.hasEvidence(game::Evidence::TornLetter) ? "LETTER SIGNED 'R.'?" : "ALIBI: NONE"},
        {SuspectColour::Heiress, "THE HEIRESS", "MOTIVE: THE WILL"},
        {SuspectColour::Partner, "THE PARTNER", st.hasEvidence(game::Evidence::Fingerprint) ? "PRINT ON THE SAFE?" : "NOT QUESTIONED"},
    };
    for (int i = 0; i < 3; ++i) {
        const int y = 80 + i * 44;
        box(c, 12, y, 226, 38, Section, Section);
        fillCircle(c, {32, y + 19}, 12, rows[i].col);
        fillEllipse(c, {32, y + 15}, 5, 6, Color{230, 200, 170});
        drawText(c, 52, y + 8, rows[i].name, TextCol);
        drawText(c, 52, y + 22, rows[i].note, Dim);
    }

    // Evidence slots: filled from GameState in the order found
    const std::string count = "EVIDENCE  " + std::to_string(st.evidence.size()) + "/" + std::to_string(game::EvidenceTotal);
    drawText(c, 14, 220, count, SuspectColour::Highlight);
    for (int i = 0; i < game::EvidenceTotal; ++i) {
        const int x = 12 + (i % 2) * 116, y = 234 + (i / 2) * 80;
        if (i < static_cast<int>(st.evidence.size())) {
            box(c, x, y, 110, 74, Section, SuspectColour::Highlight);
            evidenceIcon(c, st.evidence[static_cast<size_t>(i)], x + 27, y + 4, 56, TextCol);
            const char* name = game::evidenceName(st.evidence[static_cast<size_t>(i)]);
            drawText(c, x + 55 - textWidth(name) / 2, y + 62, name, TextCol);
        } else {
            for (int k = 0; k < 110; k += 10) {                      // dashed empty slot
                drawLineBresenham(c, {x + k, y}, {x + k + 5, y}, Dim);
                drawLineBresenham(c, {x + k, y + 74}, {x + k + 5, y + 74}, Dim);
            }
            for (int k = 0; k < 74; k += 10) {
                drawLineBresenham(c, {x, y + k}, {x, y + k + 5}, Dim);
                drawLineBresenham(c, {x + 110, y + k}, {x + 110, y + k + 5}, Dim);
            }
            drawText(c, x + 52, y + 33, "?", Dim);
        }
    }

    std::string why;
    drawButton(c, board_, st.canEnter(game::Screen::Board, &why), board_.contains(mouse_), Color{60, 110, 180});
    drawButton(c, deduce_, st.canEnter(game::Screen::Deduction, &why), deduce_.contains(mouse_), Color{150, 70, 60});
    if (st.screen != game::Screen::Explore) drawButton(c, back_, true, back_.contains(mouse_), Color{80, 84, 100});
}

// ============================== Evidence board ==============================
BoardUI::BoardUI() {
    cards_ = {{0, 345, 190, 110, 120}, {1, 60, 40, 110, 120}, {2, 630, 40, 110, 120}, {3, 345, 360, 110, 120}};
}

void BoardUI::sync(const game::GameState& st) {
    const double spots[][2] = {{60, 330}, {630, 330}, {200, 210}, {500, 210}};
    for (size_t i = 0; i < st.evidence.size() && i < 4; ++i) {
        const int id = 10 + static_cast<int>(st.evidence[i]);
        if (std::none_of(cards_.begin(), cards_.end(), [id](const Card& c) { return c.id == id; }))
            cards_.push_back({id, spots[i][0], spots[i][1], 110, 100});
    }
}

PointF BoardUI::pinOf(int id) const {
    for (const auto& c : cards_) if (c.id == id) return {c.x + c.w / 2, c.y + 6};
    return {-100, -100};
}

PointF BoardUI::cardCentre(int id) const {
    for (const auto& c : cards_) if (c.id == id) return {c.x + c.w / 2, c.y + c.h / 2};
    return {-100, -100};
}

int BoardUI::cardAt(PointF p) const {
    for (size_t i = cards_.size(); i-- > 0;)
        if (pointInPolygon(makeRect(cards_[i].x, cards_[i].y, cards_[i].w, cards_[i].h), p)) return static_cast<int>(i);
    return -1;
}

int BoardUI::pinAt(PointF p) const {
    for (size_t i = cards_.size(); i-- > 0;)
        if (game::inCircle(pinOf(cards_[i].id), 11, p)) return cards_[i].id;
    return -1;
}

void BoardUI::mouseDown(PointF p, game::GameState& st) {
    const int pin = pinAt(p);
    if (pin >= 0) {                                       // pins tie strings
        if (selectedPin_ < 0) { selectedPin_ = pin; return; }
        if (pin != selectedPin_) {
            const std::pair<int, int> link{std::min(pin, selectedPin_), std::max(pin, selectedPin_)};
            auto it = std::find(st.links.begin(), st.links.end(), link);
            if (it == st.links.end()) { st.links.push_back(link); st.say("STRING PINNED"); }
            else { st.links.erase(it); st.say("STRING REMOVED"); }
        }
        selectedPin_ = -1;
        return;
    }
    selectedPin_ = -1;
    const int i = cardAt(p);                              // cards are dragged
    if (i < 0) return;
    std::rotate(cards_.begin() + i, cards_.begin() + i + 1, cards_.end());   // bring to front
    drag_ = {true, static_cast<int>(cards_.size()) - 1, {cards_.back().x - p.x, cards_.back().y - p.y}};
}

void BoardUI::mouseMove(PointF p) {
    mouse_ = p;
    if (!drag_.active) return;
    Card& c = cards_[static_cast<size_t>(drag_.target)];
    const PointF pos = game::clampTo({p.x + drag_.grabOffset.x, p.y + drag_.grabOffset.y},
                                     ClipRect{22, 22, 800 - 22 - c.w, 500 - 22 - c.h});   // stay on the cork
    c.x = pos.x;
    c.y = pos.y;
}

void BoardUI::mouseUp(PointF) { drag_ = game::Drag{}; }

void BoardUI::render(Canvas& c, const game::GameState& st) const {
    c.clear(Color{92, 58, 32});
    scanlineFill(c, makeRect(16, 16, c.width() - 32, c.height() - 32), Palette::Cork);
    unsigned s = 11;
    for (int i = 0; i < 4000; ++i) {
        s = s * 1103515245u + 12345u;
        const int x = 16 + static_cast<int>((s >> 8) % (c.width() - 32));
        s = s * 1103515245u + 12345u;
        const int y = 16 + static_cast<int>((s >> 8) % (c.height() - 32));
        c.setPixel(x, y, (i % 3 == 0) ? Color{160, 115, 70} : Color{210, 170, 120});
    }

    const Color suspectCol[] = {Color{30, 30, 34}, SuspectColour::Butler, SuspectColour::Heiress, SuspectColour::Partner};
    const char* suspectName[] = {"VICTIM", "THE BUTLER", "THE HEIRESS", "THE PARTNER"};
    for (const auto& k : cards_) {
        scanlineFill(c, makeRect(k.x + 4, k.y + 5, k.w, k.h), Color{110, 78, 45});
        scanlineFill(c, makeRect(k.x, k.y, k.w, k.h), Color{250, 250, 246});
        drawPolygonOutline(c, makeRect(k.x, k.y, k.w, k.h), Color{120, 110, 95});
        if (k.id < 4) {                                   // person card: head and shoulders
            scanlineFill(c, makeRect(k.x + 8, k.y + 14, k.w - 16, 74), k.id == 0 ? Color{70, 70, 80} : Color{190, 196, 206});
            fillEllipse(c, {static_cast<int>(k.x + k.w / 2), static_cast<int>(k.y + 82)}, 30, 16, suspectCol[k.id]);
            scanlineFill(c, makeRect(k.x + 1, k.y + 88, k.w - 2, k.h - 89), Color{250, 250, 246});
            fillEllipse(c, {static_cast<int>(k.x + k.w / 2), static_cast<int>(k.y + 48)}, 15, 19,
                        k.id == 0 ? Color{120, 120, 125} : Color{205, 160, 120});
            drawText(c, static_cast<int>(k.x + k.w / 2) - textWidth(suspectName[k.id]) / 2, static_cast<int>(k.y + 100), suspectName[k.id],
                     k.id == 0 ? Color{150, 30, 30} : Color{40, 40, 40});
        } else {
            const auto e = static_cast<game::Evidence>(k.id - 10);
            evidenceIcon(c, e, static_cast<int>(k.x + k.w / 2 - 32), static_cast<int>(k.y + 14), 64, Color{60, 60, 90});
            drawText(c, static_cast<int>(k.x + k.w / 2) - textWidth(game::evidenceName(e)) / 2, static_cast<int>(k.y + 84),
                     game::evidenceName(e), Color{40, 40, 40});
        }
    }
    // Strings: sagging quadratic Bézier between the two pins
    for (const auto& l : st.links) {
        const PointF a = pinOf(l.first), b = pinOf(l.second);
        const double dist = std::hypot(b.x - a.x, b.y - a.y);
        const PointF ctrl{(a.x + b.x) / 2, (a.y + b.y) / 2 + dist * 0.18};
        for (int k = 0; k < 2; ++k)                       // 2 px thick in both directions
            drawBezier(c, {{a.x + k, a.y + k}, {ctrl.x + k, ctrl.y + k}, {b.x + k, b.y + k}}, Palette::StringRed, 60);
        drawBezier(c, {{a.x + 1, a.y}, {ctrl.x + 1, ctrl.y}, {b.x + 1, b.y}}, Palette::StringRed, 60);
    }
    // String being tied: follows the mouse
    if (selectedPin_ >= 0) {
        const PointF a = pinOf(selectedPin_);
        drawBezier(c, {a, {(a.x + mouse_.x) / 2, (a.y + mouse_.y) / 2 + 30}, mouse_}, Color{220, 120, 120}, 40);
    }
    for (const auto& k : cards_) {
        const PointF p = pinOf(k.id);
        const Point pp{static_cast<int>(p.x), static_cast<int>(p.y)};
        fillCircle(c, pp, 7, k.id < 4 ? suspectCol[k.id] : Palette::Red);
        drawCircleMidpoint(c, pp, 7, Color{40, 20, 20});
        fillCircle(c, {pp.x - 2, pp.y - 2}, 2, Palette::White);
        if (k.id == selectedPin_) drawCircleMidpoint(c, pp, 11, SuspectColour::Highlight);
    }
    drawText(c, 24, 478, "DRAG CARDS.  CLICK A PIN, THEN ANOTHER, TO TIE A STRING (AGAIN TO REMOVE).", Color{60, 36, 18});
}

}  // namespace ui

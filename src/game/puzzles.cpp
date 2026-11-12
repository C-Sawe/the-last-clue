// Interactive puzzles — Sudheysi (CP3, issue #43)
#include "puzzles.h"

#include <algorithm>
#include <cmath>

#include "animation/animation.h"
#include "core/font.h"
#include "primitives/primitives.h"
#include "scenes/fingerprint.h"
#include "scenes/scenes.h"

namespace game {

namespace {
const double PI = 3.14159265358979;
const int VW = 800, VH = 500;                     // puzzle viewport size

// Where the pieces start: scattered left and right of the letter's place.
const TornPose START[] = {{-0.52, -240, -55}, {0.61, 230, -60}, {0.27, -225, 45}, {-0.71, 238, 50}};

double wrapAngle(double a) {
    a = std::fmod(a, 2 * PI);
    if (a > PI) a -= 2 * PI;
    if (a < -PI) a += 2 * PI;
    return a;
}

void button(Canvas& c, double x, double y, double w, double h, const std::string& label, Color fill) {
    scanlineFill(c, makeRect(x, y, w, h), fill);
    drawPolygonOutline(c, makeRect(x, y, w, h), Palette::Black);
    drawText(c, static_cast<int>(x + w / 2) - textWidth(label, 2) / 2, static_cast<int>(y + h / 2) - 7, label, Palette::White, 2);
}
}  // namespace

// =============================== Torn letter ===============================
LetterPuzzle::LetterPuzzle() { reset(); }

void LetterPuzzle::reset() {
    poses_.assign(std::begin(START), std::end(START));
    locked_.assign(poses_.size(), false);
    order_ = {0, 1, 2, 3};
    drag_ = Drag{};
    hover_ = -1;
}

PointF LetterPuzzle::pieceScreenCentre(int i) const {
    return apply(tornPoseMatrix(i, poses_[static_cast<size_t>(i)], VW, VH), tornPieceCentre(i));
}

PointF LetterPuzzle::solvedScreenCentre(int i) const {
    return apply(tornPoseMatrix(i, tornSolvedPose(i), VW, VH), tornPieceCentre(i));
}

// Topmost unlocked piece under p: transform each piece's polygon by its pose
// matrix and test with point-in-polygon, from the top of the draw order down.
int LetterPuzzle::pieceAt(PointF p) const {
    for (size_t k = order_.size(); k-- > 0;) {
        const int i = order_[k];
        if (locked_[static_cast<size_t>(i)]) continue;
        const Polygon shape = apply(tornPoseMatrix(i, poses_[static_cast<size_t>(i)], VW, VH), tornPiece(i));
        if (pointInPolygon(shape, p)) return i;
    }
    return -1;
}

void LetterPuzzle::bringToFront(int i) {
    for (size_t k = 0; k < order_.size(); ++k)
        if (order_[k] == i) { order_.erase(order_.begin() + static_cast<long>(k)); break; }
    order_.push_back(i);
}

void LetterPuzzle::mouseDown(PointF p) {
    const int i = pieceAt(p);
    if (i < 0) return;
    bringToFront(i);
    drag_.active = true;
    drag_.target = i;
    drag_.grabOffset = {poses_[static_cast<size_t>(i)].dx - p.x, poses_[static_cast<size_t>(i)].dy - p.y};
}

void LetterPuzzle::mouseMove(PointF p) {
    hover_ = pieceAt(p);
    if (!drag_.active) return;
    // Boundary constraint: the piece's offset may only move the piece within the desk.
    const PointF o = tornLetterOrigin(VW, VH);
    const ClipRect bounds{-o.x - 60, -o.y - 80, VW - o.x - 200, VH - o.y - 80};
    const PointF target = clampTo({p.x + drag_.grabOffset.x, p.y + drag_.grabOffset.y}, bounds);
    poses_[static_cast<size_t>(drag_.target)].dx = target.x;
    poses_[static_cast<size_t>(drag_.target)].dy = target.y;
}

void LetterPuzzle::mouseUp(PointF) {
    if (drag_.active) trySnap(drag_.target);
    drag_ = Drag{};
}

void LetterPuzzle::scroll(PointF p, double dir) {
    const int i = drag_.active ? drag_.target : pieceAt(p);
    if (i < 0) return;
    poses_[static_cast<size_t>(i)].angle = wrapAngle(poses_[static_cast<size_t>(i)].angle + dir * PI / 12);   // 15 degrees
    if (!drag_.active) trySnap(i);
}

void LetterPuzzle::key(input::Key k, PointF mouse) {
    if (k == input::Key::R) scroll(mouse, 1);
}

// Snap test: close enough in position AND in angle -> jump to the exact solved pose.
void LetterPuzzle::trySnap(int i) {
    TornPose& p = poses_[static_cast<size_t>(i)];
    const TornPose goal = tornSolvedPose(i);
    const double dist = std::hypot(p.dx - goal.dx, p.dy - goal.dy);
    if (dist < 16 && std::fabs(wrapAngle(p.angle - goal.angle)) < 0.2) {
        p = goal;
        locked_[static_cast<size_t>(i)] = true;
        order_.erase(std::find(order_.begin(), order_.end(), i));
        order_.insert(order_.begin(), i);                      // locked pieces lie underneath
    }
}

bool LetterPuzzle::solved() const { return placed() == static_cast<int>(locked_.size()); }

int LetterPuzzle::placed() const {
    int n = 0;
    for (bool b : locked_) n += b ? 1 : 0;
    return n;
}

void LetterPuzzle::render(Canvas& c) const {
    renderTornPieces(c, poses_, order_, locked_, drag_.active ? drag_.target : hover_);
    for (int y = 0; y < 30; ++y)
        for (int x = 0; x < VW; ++x) c.setPixel(x, y, Color{30, 22, 14});
    drawText(c, 10, 9, "DRAG THE PIECES INTO PLACE.  SCROLL OR R TO ROTATE.", Palette::White);
    const std::string count = std::to_string(placed()) + " / 4 PLACED";
    drawText(c, VW - textWidth(count) - 10, 9, count, solved() ? Color{120, 230, 140} : Palette::Yellow);
    if (solved()) drawText(c, 16, VH - 28, "SOLVED - THE CODE IS IN THE PAINTING", Color{140, 230, 140}, 2);
}

// ================================= Safe =================================
SafePuzzle::SafePuzzle(std::vector<int> combination) : combo_(std::move(combination)) {}

void SafePuzzle::reset() {
    entered_.clear();
    angle_ = 0;
    dragging_ = solved_ = false;
    doorT_ = shake_ = 0;
}

int SafePuzzle::number() const {
    int n = static_cast<int>(std::lround(-angle_ / (2 * PI) * 100)) % 100;   // same mapping as the dial art
    return n < 0 ? n + 100 : n;
}

void SafePuzzle::mouseDown(PointF p) {
    if (solved_) return;
    if (p.x >= 620 && p.x <= 760 && p.y >= 380 && p.y <= 424) { enterNumber(); return; }   // SET button
    if (inCircle(dialCentre(), 150, p)) {
        dragging_ = true;
        grabAngle_ = angleAt(dialCentre(), p);
        grabDial_ = angle_;
    }
}

// Turning: the dial follows the change in the mouse's angle around the centre.
void SafePuzzle::mouseMove(PointF p) {
    if (!dragging_) return;
    angle_ = grabDial_ + angleBetween(grabAngle_, angleAt(dialCentre(), p));
    if (std::fabs(angleBetween(grabAngle_, angleAt(dialCentre(), p))) > 2.5) {   // re-anchor for big sweeps
        grabAngle_ = angleAt(dialCentre(), p);
        grabDial_ = angle_;
    }
}

void SafePuzzle::mouseUp(PointF) { dragging_ = false; }

void SafePuzzle::key(input::Key k) {
    if (solved_) return;
    if (k == input::Key::Enter) enterNumber();
    if (k == input::Key::Backspace && !entered_.empty()) entered_.pop_back();
    if (k == input::Key::Left) angle_ += 2 * PI / 100;           // fine control: one number per press
    if (k == input::Key::Right) angle_ -= 2 * PI / 100;
}

void SafePuzzle::enterNumber() {
    entered_.push_back(number());
    if (entered_.size() < combo_.size()) return;
    bool ok = true;
    for (size_t i = 0; i < combo_.size(); ++i) ok = ok && std::abs(entered_[i] - combo_[i]) <= 1;   // +-1 tolerance
    if (ok) {
        solved_ = true;
        message_ = "CLICK... THE SAFE OPENS";
    } else {
        message_ = "WRONG COMBINATION";
        entered_.clear();
        shake_ = 0.4;
    }
}

void SafePuzzle::update(double dt) {
    if (solved_ && doorT_ < 1) doorT_ = std::min(1.0, doorT_ + dt / 1.4);
    if (shake_ > 0) shake_ = std::max(0.0, shake_ - dt);
}

std::string SafePuzzle::takeMessage() {
    std::string m = message_;
    message_.clear();
    return m;
}

void SafePuzzle::render(Canvas& c) const {
    c.clear(Color{34, 36, 44});
    Canvas dial(420, 420);
    renderSafeOpening(dial, angle_, anim::ease(anim::Ease::OutBack, doorT_));
    const int shakeX = shake_ > 0 ? static_cast<int>(std::lround(std::sin(shake_ * 60) * 6)) : 0;   // "wrong" feedback
    c.blit(dial, 190 + shakeX, 50);

    // Entered numbers
    drawText(c, 620, 70, "COMBINATION", Palette::White, 2);
    for (size_t i = 0; i < combo_.size(); ++i) {
        const int x = 620 + static_cast<int>(i) * 52;
        scanlineFill(c, makeRect(x, 100, 44, 40), Color{20, 20, 24});
        drawPolygonOutline(c, makeRect(x, 100, 44, 40), Palette::Grey);
        if (i < entered_.size()) drawText(c, x + 22 - textWidth(std::to_string(entered_[i]), 2) / 2, 113, std::to_string(entered_[i]), Palette::Yellow, 2);
    }
    drawText(c, 620, 160, "DRAG THE DIAL TO TURN IT", Color{180, 182, 194});
    drawText(c, 620, 176, "ARROWS = ONE STEP", Color{180, 182, 194});
    drawText(c, 620, 192, "ENTER OR SET = LOCK NUMBER", Color{180, 182, 194});
    drawText(c, 620, 208, "BACKSPACE = UNDO", Color{180, 182, 194});
    drawText(c, 620, 240, "POINTER AT: " + std::to_string(number()), Palette::White, 2);
    if (!solved_) button(c, 620, 380, 140, 44, "SET", Color{60, 110, 180});
    else drawText(c, 610, 390, doorOpen() ? "KEY + FINGERPRINT FOUND" : "OPENING...", Color{120, 230, 140});
}

// ============================ Fingerprint matching ============================
namespace {
const Color Paper{248, 246, 238}, Ink{50, 50, 70}, LiftedInk{170, 40, 50};

// Ink mask of a print canvas, thickened by one pixel in every direction.
std::vector<bool> inkMask(const Canvas& c, Color paper) {
    const int w = c.width(), h = c.height();
    std::vector<bool> m(static_cast<size_t>(w * h), false);
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x) {
            if (c.getPixel(x, y) == paper) continue;
            for (int dy = -1; dy <= 1; ++dy)
                for (int dx = -1; dx <= 1; ++dx)
                    if (x + dx >= 0 && x + dx < w && y + dy >= 0 && y + dy < h) m[static_cast<size_t>((y + dy) * w + x + dx)] = true;
        }
    return m;
}
}  // namespace

double printMatchScore(int suspect, double angle, int size) {
    const Canvas lifted = renderPrint(foundPrint(), size, LiftedInk, Paper);
    Canvas turned(size, size, Paper);
    drawPrintRotated(turned, lifted, {size / 2.0, size / 2.0}, angle, Paper);
    const auto a = inkMask(turned, Paper);
    const auto b = inkMask(renderPrint(suspectPrint(suspect), size, Ink, Paper), Paper);
    size_t both = 0, either = 0;
    for (size_t i = 0; i < a.size(); ++i) {
        both += (a[i] && b[i]);
        either += (a[i] || b[i]);
    }
    return either ? static_cast<double>(both) / static_cast<double>(either) : 0.0;
}

PrintsPuzzle::PrintsPuzzle() { reset(); }

void PrintsPuzzle::reset() {
    angle_ = StartAngle;
    dragging_ = solved_ = false;
    selected_ = hover_ = -1;
    message_.clear();
}

PointF PrintsPuzzle::cardCentre(int suspect) const { return {330 + suspect * 158 + PrintCardW / 2.0, 30 + PrintCardH / 2.0}; }

int PrintsPuzzle::cardAt(PointF p) const {
    for (int i = 0; i < 3; ++i) {
        const PointF c = cardCentre(i);
        if (std::fabs(p.x - c.x) <= PrintCardW / 2.0 && std::fabs(p.y - c.y) <= PrintCardH / 2.0) return i;
    }
    return -1;
}

double PrintsPuzzle::score() const { return selected_ < 0 ? 0.0 : printMatchScore(selected_, angle_); }

void PrintsPuzzle::mouseDown(PointF p) {
    if (solved_) return;
    mouse_ = p;
    if (p.x >= 40 && p.x <= 260 && p.y >= 420 && p.y <= 464) { compare(); return; }          // COMPARE
    if (const int i = cardAt(p); i >= 0) { selected_ = i; return; }
    if (inCircle(printCentre(), 115, p)) {                       // grab the lifted print to turn it
        dragging_ = true;
        grabAngle_ = angleAt(printCentre(), p);
        grabStart_ = angle_;
    }
}

void PrintsPuzzle::mouseMove(PointF p) {
    mouse_ = p;
    hover_ = cardAt(p);
    if (dragging_) {                                            // follow the mouse, in 5-degree steps
        const double raw = grabStart_ + angleBetween(grabAngle_, angleAt(printCentre(), p));
        angle_ = wrapAngle(std::round(raw / (PI / 36)) * (PI / 36));
    }
}

void PrintsPuzzle::mouseUp(PointF) { dragging_ = false; }

void PrintsPuzzle::scroll(PointF, double dir) {
    if (!solved_) angle_ = wrapAngle(angle_ + dir * PI / 12);   // 15 degrees per notch
}

void PrintsPuzzle::key(input::Key k) {
    if (solved_) return;
    if (k == input::Key::Left) angle_ = wrapAngle(angle_ - PI / 36);   // 5 degrees
    if (k == input::Key::Right || k == input::Key::R) angle_ = wrapAngle(angle_ + PI / 36);
    if (k == input::Key::Enter) compare();
}

void PrintsPuzzle::compare() {
    if (solved_) return;
    static const char* names[3] = {"THE BUTLER", "THE HEIRESS", "THE PARTNER"};
    if (selected_ < 0) { message_ = "PICK A SUSPECT'S CARD TO COMPARE WITH"; return; }
    const double s = score();
    const std::string pct = std::to_string(static_cast<int>(std::lround(s * 100))) + "%";
    if (s >= MatchThreshold) {
        solved_ = true;
        message_ = std::string("MATCH (") + pct + "): THE PRINT ON THE SAFE IS " + names[selected_] + "'S";
    } else if (selected_ == 2) {
        message_ = "ONLY " + pct + " - THE RIDGES DON'T LINE UP. TURN THE PRINT";
    } else {
        message_ = std::string("NO MATCH (") + pct + ") WITH " + names[selected_];
    }
}

std::string PrintsPuzzle::takeMessage() {
    std::string m = message_;
    message_.clear();
    return m;
}

void PrintsPuzzle::render(Canvas& c) const {
    c.clear(Color{36, 38, 48});
    // Left: the print lifted from the safe, on its card, turned by angle_
    scanlineFill(c, makeRect(45, 125, 210, 210), Paper);
    drawPolygonOutline(c, makeRect(45, 125, 210, 210), Color{120, 110, 95});
    static const Canvas lifted = renderPrint(foundPrint(), 160, LiftedInk, Paper);
    drawPrintRotated(c, lifted, printCentre(), angle_, Paper);
    drawCircleMidpoint(c, {150, 230}, 112, dragging_ ? SuspectColour::Highlight : Color{90, 92, 108});   // grab ring
    drawText(c, 40, 40, "LIFTED FROM THE SAFE", Palette::White, 2);
    drawText(c, 40, 64, "DRAG AROUND IT, SCROLL, R OR ARROWS TO TURN", Color{180, 182, 194});
    drawText(c, 40, 80, "THEN PICK A SUSPECT AND COMPARE", Color{180, 182, 194});
    const int deg = static_cast<int>(std::lround(angle_ * 180 / PI));
    drawText(c, 40, 360, "TURNED: " + std::to_string(deg) + " DEG", Palette::White, 2);

    // Right: the three suspects' cards
    for (int i = 0; i < 3; ++i) {
        const PointF cc = cardCentre(i);
        drawPrintCard(c, cc.x - PrintCardW / 2.0, cc.y - PrintCardH / 2.0, i, i == selected_, i == hover_);
    }

    // Overlay: the lifted print (red) laid over the chosen suspect's print (dark)
    const double ox = 500, oy = 330;
    scanlineFill(c, makeRect(ox - 90, oy - 90, 180, 180), Paper);
    drawPolygonOutline(c, makeRect(ox - 90, oy - 90, 180, 180), Color{120, 110, 95});
    if (selected_ >= 0) {
        static const Canvas suspects[3] = {renderPrint(suspectPrint(0), 160, Ink, Paper), renderPrint(suspectPrint(1), 160, Ink, Paper),
                                           renderPrint(suspectPrint(2), 160, Ink, Paper)};
        c.blit(suspects[selected_], static_cast<int>(ox - 80), static_cast<int>(oy - 80));
        drawPrintRotated(c, lifted, {ox, oy}, angle_, Paper);
        const double s = score();
        const Color col = s >= MatchThreshold ? Feedback::Correct : Feedback::Wrong;
        drawText(c, 610, 270, "OVERLAP (IOU)", Palette::White, 2);
        drawText(c, 610, 296, std::to_string(static_cast<int>(std::lround(s * 100))) + "%", col, 4);
        scanlineFill(c, makeRect(610, 340, 160, 14), Color{20, 20, 26});                     // score bar
        if (s > 0) scanlineFill(c, makeRect(610, 340, 160 * std::min(1.0, s), 14), col);
        const double tx = 610 + 160 * MatchThreshold;
        drawLineBresenham(c, {static_cast<int>(tx), 336}, {static_cast<int>(tx), 357}, Palette::White);   // pass mark
    } else {
        drawText(c, static_cast<int>(ox) - textWidth("PICK A CARD") / 2, static_cast<int>(oy) - 3, "PICK A CARD", Palette::Grey);
    }
    drawText(c, static_cast<int>(ox) - textWidth("OVERLAY") / 2, static_cast<int>(oy) + 96, "OVERLAY", Color{180, 182, 194});

    if (!solved_) button(c, 40, 420, 220, 44, "COMPARE", Color{60, 110, 180});
    else drawText(c, 40, 436, "PRINT MATCHED", Feedback::Correct, 2);
}

}  // namespace game


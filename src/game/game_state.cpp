// Game state machine — Sudheysi (CP3, issue #42)
#include "game_state.h"

#include <algorithm>

namespace game {

const char* screenName(Screen s) {
    switch (s) {
        case Screen::Explore: return "EXPLORE";
        case Screen::Letter: return "TORN LETTER";
        case Screen::Painting: return "PAINTING";
        case Screen::Computer: return "LAPTOP";
        case Screen::Safe: return "SAFE";
        case Screen::Board: return "EVIDENCE BOARD";
        case Screen::Deduction: return "DEDUCTION";
    }
    return "?";
}

const char* evidenceName(Evidence e) {
    switch (e) {
        case Evidence::TornLetter: return "TORN LETTER";
        case Evidence::SafeCode: return "SAFE CODE";
        case Evidence::Key: return "DOOR KEY";
        case Evidence::Fingerprint: return "FINGERPRINT";
    }
    return "?";
}

bool GameState::hasEvidence(Evidence e) const {
    return std::find(evidence.begin(), evidence.end(), e) != evidence.end();
}

bool GameState::addEvidence(Evidence e) {
    if (hasEvidence(e)) return false;
    evidence.push_back(e);
    say(std::string("NEW EVIDENCE: ") + evidenceName(e));
    return true;
}

void GameState::say(const std::string& message, double seconds) {
    toast = message;
    toastTime = seconds;
}

void GameState::update(double dt) {
    if (toastTime > 0) {
        toastTime -= dt;
        if (toastTime <= 0) toast.clear();
    }
}

// The rules of the investigation, in one place.
bool GameState::canEnter(Screen s, std::string* whyNot) const {
    auto no = [whyNot](const char* why) { if (whyNot) *whyNot = why; return false; };
    switch (s) {
        case Screen::Explore:
        case Screen::Computer:
            return true;
        case Screen::Letter:
            return has("open:drawer") ? true : no("THE DRAWER IS CLOSED");
        case Screen::Painting:
            return has("solved:letter") ? true : no("JUST A LANDSCAPE... NOTHING ODD YET");
        case Screen::Safe:
            return has("solved:code") ? true : no("THE SAFE'S ALARM IS ARMED - DISABLE IT ON THE LAPTOP");
        case Screen::Board:
            return !evidence.empty() ? true : no("NOTHING TO PIN ON THE BOARD YET");
        case Screen::Deduction:
            return static_cast<int>(evidence.size()) >= EvidenceTotal
                       ? true
                       : no("COLLECT ALL THE EVIDENCE FIRST");
    }
    return false;
}

bool GameState::go(Screen s) {
    std::string why;
    if (!canEnter(s, &why)) {
        say(why);
        return false;
    }
    screen = s;
    return true;
}

}  // namespace game

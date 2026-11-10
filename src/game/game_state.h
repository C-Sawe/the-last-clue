#pragma once
// Game state machine — Sudheysi (CP3, issue #42)
//
// One object remembers everything about the investigation:
//   - which screen the player is on (Explore, a puzzle, the board, ...)
//   - flags for what has happened ("open:drawer", "solved:letter", ...)
//   - the evidence collected so far, in order
//   - the strings the player has pinned on the evidence board
// Moving between screens goes through go(), which checks the rules (you can't
// open the safe while its alarm is armed, etc.) and explains why not.
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace game {

enum class Screen { Explore, Letter, Painting, Computer, Safe, Board, Deduction };
enum class Evidence { TornLetter, SafeCode, Key, Fingerprint };
const int EvidenceTotal = 4;

const char* screenName(Screen s);
const char* evidenceName(Evidence e);

struct GameState {
    Screen screen = Screen::Explore;
    std::set<std::string> flags;
    std::vector<Evidence> evidence;
    std::vector<std::pair<int, int>> links;     // evidence-board strings between card ids

    std::string toast;                          // short message shown at the bottom
    double toastTime = 0;

    bool has(const std::string& flag) const { return flags.count(flag) > 0; }
    void set(const std::string& flag) { flags.insert(flag); }
    bool hasEvidence(Evidence e) const;
    bool addEvidence(Evidence e);               // true if new (also shows a toast)

    void say(const std::string& message, double seconds = 2.8);
    void update(double dt);

    // ---- the state machine ----
    bool canEnter(Screen s, std::string* whyNot = nullptr) const;
    bool go(Screen s);                          // enter if allowed, otherwise say why
};

}  // namespace game

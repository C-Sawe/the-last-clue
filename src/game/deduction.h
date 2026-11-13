#pragma once
// Final deduction rules — Sudheysi (final build, issue #48)
//
// The case ends with three questions: WHO killed her, HOW, and WHY. Each has
// three possible answers. The player can pick any answer (the red herrings are
// always on the table), but each answer lists the evidence that SUPPORTS it,
// and only the true answers are backed by evidence found in the study:
//   WHO  - the partner      <- the fingerprint matched on the safe
//   HOW  - poison           <- the empty vial behind the study door
//   WHY  - hide the theft   <- the decoded email
// judge() compares an accusation with the truth. Three right = CASE SOLVED.
// A wrong accusation costs one of MaxAccusations tries; when they run out the
// case is closed unsolved (CASE FAILED).
#include <string>
#include <vector>

#include "game_state.h"

namespace game {

enum class Question { Who, How, Why };
const int QuestionCount = 3, AnswerCount = 3;

struct Answer {
    const char* title;          // short, shown on the card
    const char* detail;         // one line under it
    const char* supportFlag;    // evidence/flag that backs it up ("" = none)
    const char* support;        // what the case file says about it once found
};

const char* questionName(Question q);                     // "WHO?", "HOW?", "WHY?"
const Answer& answer(Question q, int index);
// true when the evidence behind this answer has been found
bool answerSupported(const GameState& st, Question q, int index);

struct Accusation {
    int pick[QuestionCount] = {-1, -1, -1};
    bool complete() const { return pick[0] >= 0 && pick[1] >= 0 && pick[2] >= 0; }
};

struct Verdict {
    bool solved = false;
    bool caseClosed = false;    // no tries left: the case is lost
    int correct = 0;            // how many of the three were right
    bool right[QuestionCount] = {false, false, false};
    int triesLeft = 0;
    std::vector<std::string> lines;   // the story shown on the outcome screen
};

// Judges the accusation and records it in the state ("accused", accusations,
// "case:solved" / "case:failed").
Verdict judge(GameState& st, const Accusation& a);

// The truth, exposed for tests and the scripted playthrough.
int truth(Question q);

}  // namespace game

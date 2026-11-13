// Final deduction rules — Sudheysi (final build, issue #48)
#include "deduction.h"

namespace game {

namespace {
const Answer ANSWERS[QuestionCount][AnswerCount] = {
    {   // WHO?
        {"THE BUTLER", "HAD NO ALIBI FOR 9 PM", "", "NO EVIDENCE PLACES HIM AT THE SAFE"},
        {"THE HEIRESS", "SHE INHERITS EVERYTHING", "", "HER PRINT DOES NOT MATCH"},
        {"THE PARTNER", "CO-OWNS THE BUSINESS", "solved:print", "HIS PRINT IS ON THE SAFE"},
    },
    {   // HOW?
        {"STRUCK FROM BEHIND", "A HEAVY OBJECT", "", "NO WEAPON, NO INJURY ON THE OUTLINE"},
        {"POISONED", "SOMETHING IN HER COFFEE", "found:vial", "EMPTY VIAL IN THE BACK ROOM"},
        {"PUSHED", "A FALL IN THE STUDY", "", "THE WINDOW WAS LOCKED"},
    },
    {   // WHY?
        {"THE WILL", "TO INHERIT HER FORTUNE", "", "THE WILL WAS NOT CHANGED"},
        {"REVENGE", "SHE WAS ABOUT TO FIRE STAFF", "", "NOTHING IN HER PAPERS SAYS SO"},
        {"THE STOLEN MONEY", "TO STOP HER EXPOSING IT", "solved:email", "HER EMAIL: CONFESS BY FRIDAY"},
    },
};
const int TRUTH[QuestionCount] = {2, 1, 2};
}  // namespace

const char* questionName(Question q) {
    switch (q) {
        case Question::Who: return "WHO?";
        case Question::How: return "HOW?";
        case Question::Why: return "WHY?";
    }
    return "?";
}

const Answer& answer(Question q, int index) {
    const int i = index < 0 ? 0 : (index >= AnswerCount ? AnswerCount - 1 : index);
    return ANSWERS[static_cast<int>(q)][i];
}

bool answerSupported(const GameState& st, Question q, int index) {
    const Answer& a = answer(q, index);
    return a.supportFlag[0] != '\0' && st.has(a.supportFlag);
}

int truth(Question q) { return TRUTH[static_cast<int>(q)]; }

Verdict judge(GameState& st, const Accusation& a) {
    Verdict v;
    for (int q = 0; q < QuestionCount; ++q) {
        v.right[q] = a.pick[q] == TRUTH[q];
        v.correct += v.right[q] ? 1 : 0;
    }
    v.solved = v.correct == QuestionCount;
    st.set("accused");
    if (v.solved) {
        st.set("case:solved");
        v.lines = {"THE PARTNER HAD BEEN TAKING MONEY FROM THE FIRM.",
                   "SHE FOUND OUT AND GAVE HIM UNTIL FRIDAY TO CONFESS.",
                   "AT 9 PM HE POISONED HER COFFEE AND EMPTIED THE SAFE.",
                   "HIS PRINT ON THE SAFE GAVE HIM AWAY."};
    } else {
        ++st.accusations;
        v.triesLeft = MaxAccusations - st.accusations;
        v.caseClosed = v.triesLeft <= 0;
        if (v.caseClosed) st.set("case:failed");
        for (int q = 0; q < QuestionCount; ++q) {
            const Answer& picked = answer(static_cast<Question>(q), a.pick[q]);
            v.lines.push_back(std::string(questionName(static_cast<Question>(q))) + " " + picked.title + " - " +
                              (v.right[q] ? "THE EVIDENCE AGREES" : picked.support));
        }
    }
    v.triesLeft = MaxAccusations - st.accusations;
    return v;
}

}  // namespace game

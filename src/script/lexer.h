#pragma once
// ClueScript lexer — Caleb (CP3, issue #58)
// Turns source text into tokens, remembering the line of each token so errors
// can say "Line 3: ...". Newlines matter: they end a statement.
#include <stdexcept>
#include <string>
#include <vector>

namespace script {

enum class Tok {
    Number, String, Ident,
    Let, Repeat, While, If, Else, And, Or, Not, True, False,
    Plus, Minus, Star, Slash, Percent,
    Assign, Eq, NotEq, Less, Greater, LessEq, GreaterEq,
    LParen, RParen, LBrace, RBrace, Comma,
    Newline, End,
};

struct Token {
    Tok type;
    std::string text;   // identifier name / string contents / original symbol
    double number = 0;
    int line = 1;
};

struct ScriptError : std::runtime_error {
    int line;
    ScriptError(int l, const std::string& msg) : std::runtime_error(msg), line(l) {}
};

std::vector<Token> lex(const std::string& source);   // throws ScriptError

}  // namespace script

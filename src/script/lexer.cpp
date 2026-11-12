// ClueScript lexer — Caleb (CP3, issue #58)
#include "lexer.h"

#include <cctype>
#include <map>

namespace script {

std::vector<Token> lex(const std::string& src) {
    static const std::map<std::string, Tok> keywords = {
        {"let", Tok::Let}, {"repeat", Tok::Repeat}, {"while", Tok::While}, {"if", Tok::If},
        {"else", Tok::Else}, {"and", Tok::And}, {"or", Tok::Or}, {"not", Tok::Not},
        {"true", Tok::True}, {"false", Tok::False},
    };
    std::vector<Token> out;
    int line = 1;
    size_t i = 0;
    auto push = [&](Tok t, std::string text = "", double n = 0) { out.push_back({t, std::move(text), n, line}); };

    while (i < src.size()) {
        const char c = src[i];
        if (c == '\n' || c == ';') {                         // statement separators
            push(Tok::Newline, "\\n");
            if (c == '\n') ++line;
            ++i;
        } else if (c == ' ' || c == '\t' || c == '\r') {
            ++i;
        } else if (c == '#') {                               // comment to end of line
            while (i < src.size() && src[i] != '\n') ++i;
        } else if (std::isdigit(static_cast<unsigned char>(c)) ||
                   (c == '.' && i + 1 < src.size() && std::isdigit(static_cast<unsigned char>(src[i + 1])))) {
            size_t j = i;
            while (j < src.size() && (std::isdigit(static_cast<unsigned char>(src[j])) || src[j] == '.')) ++j;
            const std::string num = src.substr(i, j - i);
            push(Tok::Number, num, std::stod(num));
            i = j;
        } else if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
            size_t j = i;
            while (j < src.size() && (std::isalnum(static_cast<unsigned char>(src[j])) || src[j] == '_')) ++j;
            std::string word = src.substr(i, j - i);
            for (auto& ch : word) ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
            auto kw = keywords.find(word);
            push(kw != keywords.end() ? kw->second : Tok::Ident, word);
            i = j;
        } else if (c == '"') {
            size_t j = i + 1;
            while (j < src.size() && src[j] != '"' && src[j] != '\n') ++j;
            if (j >= src.size() || src[j] != '"') throw ScriptError(line, "Line " + std::to_string(line) + ": text is missing its closing \"");
            push(Tok::String, src.substr(i + 1, j - i - 1));
            i = j + 1;
        } else {
            const char n = i + 1 < src.size() ? src[i + 1] : '\0';
            auto two = [&](Tok t, const char* s) { push(t, s); i += 2; };
            auto one = [&](Tok t) { push(t, std::string(1, c)); ++i; };
            switch (c) {
                case '+': one(Tok::Plus); break;
                case '-': one(Tok::Minus); break;
                case '*': one(Tok::Star); break;
                case '/': one(Tok::Slash); break;
                case '%': one(Tok::Percent); break;
                case '(': one(Tok::LParen); break;
                case ')': one(Tok::RParen); break;
                case '{': one(Tok::LBrace); break;
                case '}': one(Tok::RBrace); break;
                case ',': one(Tok::Comma); break;
                case '=': if (n == '=') two(Tok::Eq, "=="); else one(Tok::Assign); break;
                case '!': if (n == '=') { two(Tok::NotEq, "!="); break; }
                          throw ScriptError(line, "Line " + std::to_string(line) + ": unexpected '!' (did you mean '!='?)");
                case '<': if (n == '=') two(Tok::LessEq, "<="); else one(Tok::Less); break;
                case '>': if (n == '=') two(Tok::GreaterEq, ">="); else one(Tok::Greater); break;
                default:
                    throw ScriptError(line, "Line " + std::to_string(line) + ": unexpected character '" + std::string(1, c) + "'");
            }
        }
    }
    push(Tok::Newline, "\\n");
    push(Tok::End, "end of program");
    return out;
}

}  // namespace script

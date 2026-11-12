// ClueScript parser — Caleb (CP3, issue #58)
#include "parser.h"

namespace script {
namespace {

class Parser {
public:
    explicit Parser(const std::vector<Token>& t) : t_(t) {}

    Block program() {
        Block out;
        skipNewlines();
        while (peek().type != Tok::End) {
            out.push_back(statement());
            skipNewlines();
        }
        return out;
    }

private:
    const std::vector<Token>& t_;
    size_t i_ = 0;

    const Token& peek(size_t ahead = 0) const { return t_[std::min(i_ + ahead, t_.size() - 1)]; }
    const Token& next() { return t_[i_ < t_.size() - 1 ? i_++ : i_]; }
    bool match(Tok k) { if (peek().type == k) { ++i_; return true; } return false; }
    void skipNewlines() { while (peek().type == Tok::Newline) ++i_; }

    [[noreturn]] void fail(const std::string& what) const {
        const Token& tk = peek();
        const std::string got = tk.type == Tok::Newline ? "end of line" : "'" + tk.text + "'";
        throw ScriptError(tk.line, "Line " + std::to_string(tk.line) + ": expected " + what + " but found " + got);
    }
    const Token& expect(Tok k, const std::string& what) {
        if (peek().type != k) fail(what);
        return next();
    }
    void endOfStatement() {
        if (peek().type == Tok::RBrace || peek().type == Tok::End) return;   // '}' may end the line
        if (!match(Tok::Newline)) fail("a new line");
    }

    // ---- statements ----
    StmtPtr statement() {
        const Token& tk = peek();
        auto s = std::make_unique<Stmt>();
        s->line = tk.line;
        switch (tk.type) {
            case Tok::Let:
                next();
                s->kind = Stmt::Kind::Let;
                s->name = expect(Tok::Ident, "a variable name after 'let'").text;
                expect(Tok::Assign, "'=' after the variable name");
                s->value = expr();
                endOfStatement();
                return s;
            case Tok::Repeat:
            case Tok::While:
                next();
                s->kind = tk.type == Tok::Repeat ? Stmt::Kind::Repeat : Stmt::Kind::While;
                s->value = expr();
                s->body = block();
                return s;
            case Tok::If:
                return ifStatement();
            case Tok::Ident:
                if (peek(1).type == Tok::Assign) {
                    s->kind = Stmt::Kind::Assign;
                    s->name = next().text;
                    next();
                    s->value = expr();
                } else {
                    s->kind = Stmt::Kind::Command;
                    s->name = next().text;
                    while (peek().type != Tok::Newline && peek().type != Tok::RBrace && peek().type != Tok::End) {
                        s->args.push_back(expr());
                        match(Tok::Comma);                       // commas between arguments are optional
                    }
                }
                endOfStatement();
                return s;
            case Tok::RBrace:
                fail("a statement (this '}' has no matching '{')");
            default:
                fail("a command such as 'forward 100'");
        }
    }

    StmtPtr ifStatement() {
        auto s = std::make_unique<Stmt>();
        s->line = next().line;                                   // 'if'
        s->kind = Stmt::Kind::If;
        s->value = expr();
        s->body = block();
        size_t save = i_;
        skipNewlines();
        if (match(Tok::Else)) {
            if (peek().type == Tok::If) s->elseBody.push_back(ifStatement());
            else s->elseBody = block();
        } else {
            i_ = save;
        }
        return s;
    }

    Block block() {
        expect(Tok::LBrace, "'{' to start the block");
        Block b;
        skipNewlines();
        while (peek().type != Tok::RBrace) {
            if (peek().type == Tok::End) fail("'}' to close the block");
            b.push_back(statement());
            skipNewlines();
        }
        next();                                                  // '}'
        return b;
    }

    // ---- expressions (lowest precedence first) ----
    ExprPtr binary(ExprPtr l, Tok op, ExprPtr r, int line) {
        auto e = std::make_unique<Expr>();
        e->kind = Expr::Kind::Binary;
        e->op = op;
        e->left = std::move(l);
        e->right = std::move(r);
        e->line = line;
        return e;
    }
    ExprPtr expr() { return orExpr(); }
    ExprPtr orExpr() {
        ExprPtr l = andExpr();
        while (peek().type == Tok::Or) { const int ln = next().line; l = binary(std::move(l), Tok::Or, andExpr(), ln); }
        return l;
    }
    ExprPtr andExpr() {
        ExprPtr l = notExpr();
        while (peek().type == Tok::And) { const int ln = next().line; l = binary(std::move(l), Tok::And, notExpr(), ln); }
        return l;
    }
    ExprPtr notExpr() {
        if (peek().type == Tok::Not) {
            auto e = std::make_unique<Expr>();
            e->kind = Expr::Kind::Unary;
            e->op = Tok::Not;
            e->line = next().line;
            e->left = notExpr();
            return e;
        }
        return compare();
    }
    ExprPtr compare() {
        ExprPtr l = sum();
        const Tok k = peek().type;
        if (k == Tok::Eq || k == Tok::NotEq || k == Tok::Less || k == Tok::Greater || k == Tok::LessEq || k == Tok::GreaterEq) {
            const int ln = next().line;
            l = binary(std::move(l), k, sum(), ln);
        }
        return l;
    }
    ExprPtr sum() {
        ExprPtr l = product();
        while (peek().type == Tok::Plus || peek().type == Tok::Minus) {
            const Tok op = peek().type;
            const int ln = next().line;
            l = binary(std::move(l), op, product(), ln);
        }
        return l;
    }
    ExprPtr product() {
        ExprPtr l = unary();
        while (peek().type == Tok::Star || peek().type == Tok::Slash || peek().type == Tok::Percent) {
            const Tok op = peek().type;
            const int ln = next().line;
            l = binary(std::move(l), op, unary(), ln);
        }
        return l;
    }
    ExprPtr unary() {
        if (peek().type == Tok::Minus) {
            auto e = std::make_unique<Expr>();
            e->kind = Expr::Kind::Unary;
            e->op = Tok::Minus;
            e->line = next().line;
            e->left = unary();
            return e;
        }
        return primary();
    }
    ExprPtr primary() {
        const Token& tk = peek();
        auto e = std::make_unique<Expr>();
        e->line = tk.line;
        switch (tk.type) {
            case Tok::Number: e->kind = Expr::Kind::Number; e->number = next().number; return e;
            case Tok::True:   e->kind = Expr::Kind::Number; e->number = 1; next(); return e;
            case Tok::False:  e->kind = Expr::Kind::Number; e->number = 0; next(); return e;
            case Tok::String: e->kind = Expr::Kind::String; e->text = next().text; return e;
            case Tok::Ident:  e->kind = Expr::Kind::Var;    e->text = next().text; return e;
            case Tok::LParen: {
                next();
                ExprPtr inner = expr();
                expect(Tok::RParen, "')'");
                return inner;
            }
            default:
                fail("a number, a name or '('");
        }
    }
};

}  // namespace

Block parse(const std::vector<Token>& tokens) { return Parser(tokens).program(); }

}  // namespace script

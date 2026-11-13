// ClueScript interpreter — Caleb (CP3, issue #58)
//
// A tree-walking interpreter: it visits the AST node by node.
//   - Values are numbers or text.
//   - Variables live in one table (the programs are tiny).
//   - Every statement executed costs one "step"; past the limit we stop with a
//     friendly error, so an infinite loop can never freeze the game.
//   - The turtle (position + heading) moves with `forward`: the step vector
//     (d, 0) is ROTATED by the heading with our CP1 rotation matrix, exactly
//     x' = x cos(a) - y sin(a), y' = x sin(a) + y cos(a).
#include <cmath>
#include <map>
#include <sstream>

#include "parser.h"
#include "script.h"

namespace script {
namespace {

const double PI = 3.14159265358979;

struct Value {
    bool isText = false;
    double num = 0;
    std::string text;
};

std::string show(const Value& v) {
    if (v.isText) return v.text;
    std::ostringstream o;
    if (std::fabs(v.num - std::round(v.num)) < 1e-9) o << static_cast<long long>(std::llround(v.num));
    else o << v.num;
    return o.str();
}

bool namedColor(const std::string& n, Color& out) {
    static const std::map<std::string, Color> colours = {
        {"black", {20, 20, 24}}, {"white", {245, 245, 245}}, {"red", {210, 40, 40}},
        {"green", {40, 170, 70}}, {"blue", {50, 90, 220}}, {"yellow", {235, 205, 50}},
        {"orange", {235, 140, 30}}, {"purple", {140, 60, 170}}, {"grey", {130, 130, 140}},
        {"gray", {130, 130, 140}}, {"brown", {120, 80, 45}}, {"gold", {220, 185, 70}},
    };
    auto it = colours.find(n);
    if (it == colours.end()) return false;
    out = it->second;
    return true;
}

class Interpreter {
public:
    Interpreter(const RunOptions& o, RunResult& r) : opt_(o), res_(r), pos_(o.start), heading_(o.heading) {}

    void exec(const Block& b) { for (const auto& s : b) exec(*s); }

private:
    const RunOptions& opt_;
    RunResult& res_;
    std::map<std::string, Value> vars_;
    PointF pos_;
    double heading_;
    bool penDown_ = true;
    Color colour_{20, 20, 24};

    [[noreturn]] void fail(int line, const std::string& msg) {
        throw ScriptError(line, "Line " + std::to_string(line) + ": " + msg);
    }

    void tick(int line) {
        if (++res_.steps > opt_.stepLimit)
            fail(line, "the program ran too long (" + std::to_string(opt_.stepLimit) + " steps) - is there a loop that never ends?");
    }

    double number(const Expr& e) {
        Value v = eval(e);
        if (v.isText) fail(e.line, "expected a number but got text \"" + v.text + "\"");
        return v.num;
    }

    Value eval(const Expr& e) {
        switch (e.kind) {
            case Expr::Kind::Number: return {false, e.number, ""};
            case Expr::Kind::String: return {true, 0, e.text};
            case Expr::Kind::Var: {
                auto it = vars_.find(e.text);
                if (it == vars_.end()) fail(e.line, "'" + e.text + "' has no value yet (use 'let " + e.text + " = ...')");
                return it->second;
            }
            case Expr::Kind::Unary: {
                const double v = number(*e.left);
                return {false, e.op == Tok::Minus ? -v : (v == 0 ? 1.0 : 0.0), ""};
            }
            case Expr::Kind::Binary: {
                if (e.op == Tok::And) return {false, (number(*e.left) != 0 && number(*e.right) != 0) ? 1.0 : 0.0, ""};
                if (e.op == Tok::Or) return {false, (number(*e.left) != 0 || number(*e.right) != 0) ? 1.0 : 0.0, ""};
                if (e.op == Tok::Plus) {                                 // + also joins text
                    Value a = eval(*e.left), b = eval(*e.right);
                    if (a.isText || b.isText) return {true, 0, show(a) + show(b)};
                    return {false, a.num + b.num, ""};
                }
                const double a = number(*e.left), b = number(*e.right);
                switch (e.op) {
                    case Tok::Minus: return {false, a - b, ""};
                    case Tok::Star: return {false, a * b, ""};
                    case Tok::Slash:
                        if (b == 0) fail(e.line, "division by zero");
                        return {false, a / b, ""};
                    case Tok::Percent:
                        if (b == 0) fail(e.line, "division by zero");
                        return {false, std::fmod(a, b), ""};
                    case Tok::Eq: return {false, a == b ? 1.0 : 0.0, ""};
                    case Tok::NotEq: return {false, a != b ? 1.0 : 0.0, ""};
                    case Tok::Less: return {false, a < b ? 1.0 : 0.0, ""};
                    case Tok::Greater: return {false, a > b ? 1.0 : 0.0, ""};
                    case Tok::LessEq: return {false, a <= b ? 1.0 : 0.0, ""};
                    case Tok::GreaterEq: return {false, a >= b ? 1.0 : 0.0, ""};
                    default: fail(e.line, "unknown operator");
                }
            }
        }
        fail(e.line, "bad expression");
    }

    // A bare word like `red` or `up` is allowed as a command argument.
    std::string word(const Expr& e) {
        if (e.kind == Expr::Kind::Var && !vars_.count(e.text)) return e.text;
        return show(eval(e));
    }

    void needArgs(const Stmt& s, size_t n, const std::string& usage) {
        if (s.args.size() != n) fail(s.line, "'" + s.name + "' needs " + std::to_string(n) + " value(s): " + usage);
    }

    void moveTo(PointF to) {
        if (penDown_) res_.drawing.push_back({DrawCmd::Kind::Line, {pos_, to}, 0, 0, colour_});
        pos_ = to;
    }

    void command(const Stmt& s) {
        const std::string& n = s.name;
        if (n == "forward" || n == "back") {
            needArgs(s, 1, n + " 100");
            const double d = number(*s.args[0]) * (n == "back" ? -1 : 1);
            const PointF step = apply(rotate(heading_ * PI / 180.0), PointF{d, 0});   // rotate (d, 0) by heading
            moveTo({pos_.x + step.x, pos_.y + step.y});
        } else if (n == "turn" || n == "right") {
            needArgs(s, 1, n + " 90");
            heading_ += number(*s.args[0]);
        } else if (n == "left") {
            needArgs(s, 1, "left 90");
            heading_ -= number(*s.args[0]);
        } else if (n == "pen") {
            needArgs(s, 1, "pen up  or  pen down");
            const std::string w = word(*s.args[0]);
            if (w != "up" && w != "down") fail(s.line, "use 'pen up' or 'pen down'");
            penDown_ = (w == "down");
        } else if (n == "goto") {
            needArgs(s, 2, "goto 100 200");
            moveTo({number(*s.args[0]), number(*s.args[1])});
        } else if (n == "home") {
            pos_ = opt_.start;
            heading_ = opt_.heading;
        } else if (n == "color" || n == "colour") {
            if (s.args.size() == 3) {
                auto ch = [&](size_t i) { return static_cast<uint8_t>(std::max(0.0, std::min(255.0, number(*s.args[i])))); };
                colour_ = {ch(0), ch(1), ch(2)};
            } else {
                needArgs(s, 1, "color red  or  color 255 0 0");
                if (!namedColor(word(*s.args[0]), colour_)) fail(s.line, "unknown colour '" + word(*s.args[0]) + "'");
            }
        } else if (n == "line") {
            needArgs(s, 4, "line x1 y1 x2 y2");
            res_.drawing.push_back({DrawCmd::Kind::Line,
                                    {{number(*s.args[0]), number(*s.args[1])}, {number(*s.args[2]), number(*s.args[3])}}, 0, 0, colour_});
        } else if (n == "circle") {
            needArgs(s, 3, "circle x y radius");
            res_.drawing.push_back({DrawCmd::Kind::Circle, {{number(*s.args[0]), number(*s.args[1])}}, number(*s.args[2]), 0, colour_});
        } else if (n == "ellipse") {
            needArgs(s, 4, "ellipse x y rx ry");
            res_.drawing.push_back({DrawCmd::Kind::Ellipse, {{number(*s.args[0]), number(*s.args[1])}},
                                    number(*s.args[2]), number(*s.args[3]), colour_});
        } else if (n == "fill_poly") {
            if (s.args.size() < 6 || s.args.size() % 2) fail(s.line, "'fill_poly' needs at least 3 points: fill_poly x1 y1 x2 y2 x3 y3");
            DrawCmd d{DrawCmd::Kind::FillPoly, {}, 0, 0, colour_};
            for (size_t i = 0; i < s.args.size(); i += 2) d.pts.push_back({number(*s.args[i]), number(*s.args[i + 1])});
            res_.drawing.push_back(d);
        } else if (n == "print") {
            std::string line;
            for (const auto& a : s.args) line += (line.empty() ? "" : " ") + show(eval(*a));
            res_.printed.push_back(line);
        } else if (n == "shift") {
            // Caesar shift: move every letter n places along the alphabet (wrapping
            // Z -> A), leave everything else alone, and print the result.
            needArgs(s, 2, "shift \"TEXT\" 3");
            const Value v = eval(*s.args[0]);
            if (!v.isText) fail(s.line, "'shift' needs text in quotes first: shift \"TEXT\" 3");
            const long long k = static_cast<long long>(std::lround(number(*s.args[1])));
            std::string out = v.text;
            for (char& ch : out) {
                if (ch >= 'a' && ch <= 'z') ch = static_cast<char>(ch - 'a' + 'A');
                if (ch >= 'A' && ch <= 'Z') ch = static_cast<char>('A' + (((ch - 'A' + k) % 26) + 26) % 26);
            }
            res_.printed.push_back(out);
        } else {
            fail(s.line, "unknown command '" + n + "'");
        }
    }

    void exec(const Stmt& s) {
        tick(s.line);
        switch (s.kind) {
            case Stmt::Kind::Let: vars_[s.name] = eval(*s.value); break;
            case Stmt::Kind::Assign:
                if (!vars_.count(s.name)) fail(s.line, "'" + s.name + "' must be created with 'let' first");
                vars_[s.name] = eval(*s.value);
                break;
            case Stmt::Kind::Repeat: {
                const double n = number(*s.value);
                if (n < 0) fail(s.line, "repeat needs a count of 0 or more");
                for (long long i = 0; i < static_cast<long long>(n); ++i) exec(s.body);
                break;
            }
            case Stmt::Kind::While:
                while (number(*s.value) != 0) { exec(s.body); tick(s.line); }
                break;
            case Stmt::Kind::If:
                if (number(*s.value) != 0) exec(s.body);
                else exec(s.elseBody);
                break;
            case Stmt::Kind::Command: command(s); break;
        }
    }
};

}  // namespace

RunResult run(const std::string& source, const RunOptions& opt) {
    RunResult res;
    try {
        const Block program = parse(lex(source));
        Interpreter(opt, res).exec(program);
    } catch (const ScriptError& e) {
        res.ok = false;
        res.errorLine = e.line;
        res.error = e.what();
    }
    return res;
}

}  // namespace script

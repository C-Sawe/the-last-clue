#pragma once
// ClueScript abstract syntax tree — Caleb (CP3, issue #58)
// The parser turns tokens into this tree; the interpreter walks it.
#include <memory>
#include <string>
#include <vector>

#include "lexer.h"

namespace script {

struct Expr {
    enum class Kind { Number, String, Var, Unary, Binary } kind;
    double number = 0;
    std::string text;            // String value / Var name
    Tok op = Tok::Plus;          // Unary / Binary operator
    std::unique_ptr<Expr> left, right;
    int line = 1;
};
using ExprPtr = std::unique_ptr<Expr>;

struct Stmt;
using StmtPtr = std::unique_ptr<Stmt>;
using Block = std::vector<StmtPtr>;

struct Stmt {
    enum class Kind { Let, Assign, Repeat, While, If, Command } kind;
    std::string name;            // variable or command name
    ExprPtr value;               // Let/Assign value, Repeat count, While/If condition
    std::vector<ExprPtr> args;   // Command arguments
    Block body, elseBody;
    int line = 1;
};

}  // namespace script

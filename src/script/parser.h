#pragma once
// ClueScript parser — Caleb (CP3, issue #58)
//
// Recursive descent: one function per grammar rule, lowest precedence first.
//   program    := statement*
//   statement  := 'let' NAME '=' expr | NAME '=' expr
//               | 'repeat' expr block | 'while' expr block
//               | 'if' expr block ('else' (block | if-statement))?
//               | NAME arg*                                   (a command)
//   block      := '{' statement* '}'
//   expr       := or ;  or := and ('or' and)* ;  and := not ('and' not)*
//   not        := 'not' not | compare
//   compare    := sum (('=='|'!='|'<'|'>'|'<='|'>=') sum)?
//   sum        := product (('+'|'-') product)*
//   product    := unary (('*'|'/'|'%') unary)*
//   unary      := '-' unary | primary
//   primary    := NUMBER | STRING | NAME | true | false | '(' expr ')'
#include "ast.h"

namespace script {

Block parse(const std::vector<Token>& tokens);   // throws ScriptError

}  // namespace script

#ifndef REGEX_PATTERNS_H
#define REGEX_PATTERNS_H
#include <string>

static const char* VAL = "((-)?([1-9][0-9]*)|((0(x|X))[0-9a-fA-F]+))";
static const char* SYM = "(([a-zA-Z_])(\\w)*)";
static const char* REG = "((((r|R)15)|((r|R)((0(x|X))?[(0-7)|f|F]))|(sp)|(pc)|(psw))(h|l)?)";

#endif // !REGEX_PATTERNS_H


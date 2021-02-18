#ifndef LABEL_HANDLER
#define LABEL_HANDLER
#include <iostream>
class Assembler;

class LabelHandler {
public:
	LabelHandler(Assembler* const a) : my_asm(a), label(nullptr) {}

	void pass(std::string* lab);
private:
	Assembler* const my_asm;

	std::string* label;
};

#endif // !LABEL_HANDLER


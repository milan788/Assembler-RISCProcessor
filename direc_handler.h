#ifndef DIREC_HANDLER
#define DIREC_HANDLER
#include <iostream>
class Assembler;

class DirecHandler
{
public:
	enum DirecType {ALOC, ALIGN_SKIP, SECTION, GLOBAL, EQU, UNKOWN_DIREC_TYPE};
	static const std::string DIRECTIVES[5][4];
	static const int ROWS;
	static const int COLS;
	
	static const std::string PATTERNS[];
	static const int PATTERNS_SIZE;

	bool pass(std::string* dir);

	DirecHandler(Assembler* const a): my_asm(a) {}
private:
	Assembler* const my_asm = nullptr;

	std::string* direct = nullptr;
	std::string* params = nullptr;
	DirecType dir_type = DirecType::UNKOWN_DIREC_TYPE;
	int dir_indx = -1;

	void reset();
	void set(std::string* dir);
	void validate();

	void alocDir();
	void align_skip();
	void sectionDir();
	void globalDir();
	void equDir();
};

#endif // !DIREC_HANDLER



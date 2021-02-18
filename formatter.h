#ifndef FORMATTER_H
#define FORMATTER_H
#include <iostream>
#include "reloc_table.h"

class Assembler;

class Formatter {
public:
	enum Size { BYTE = 2, WORD = 4 };
	static const std::string REGEX_DEC;
	static const std::string REGEX_HEX;
	static bool stoint16(std::string& str_val, int16_t& val);
	//static const std::string REGEX_PARTS[];
	//static const int REGEX_PARTS_SIZE;

	Formatter(Assembler* const a);

	void write(int16_t value, Size w);
	void put(int16_t value, Size w);
	void reg_bits(unsigned& op_byte_1, std::string& reg);
	void val_bits(std::string& val_string, Size w);
	void sym_bits(std::string& symbol, RelocTable::RelocType reloc_type, int16_t add_op2_size = 0);

	void checkRange(int val, Size bytes);
private:
	Assembler* const my_asm;
	std::fstream& out;
};

#endif // !FORMATTER_H

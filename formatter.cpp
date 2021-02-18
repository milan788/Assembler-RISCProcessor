#include "formatter.h"
#include "assembler.h"
#include "sym_table.h"
#include "entries.h"
#include "reloc_table.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <regex>
#include <cstdint>
using namespace std;

bool Formatter::stoint16(std::string& val_string, int16_t& val)
{
	int base = 0;
	if (regex_match(val_string, regex(REGEX_DEC))) base = 10;
	else if (regex_match(val_string, regex(REGEX_HEX))) base = 16;
	else {
		return false;
	}
	val = (int16_t)stoi(val_string, nullptr, base);
	return true;
}

Formatter::Formatter(Assembler* const a) : my_asm(a), out(a->out) {
	out << hex << setfill('0');
}

//mozda ne sme da stvara bocne efekte
template<typename T>
constexpr auto swap16(T& x) { (x) = (((uint16_t)(x) & 0xff00) >> 8) | (((uint16_t)(x) & 0x00ff) << 8); }

const string Formatter::REGEX_DEC = "[1-9]\\d*";
const string Formatter::REGEX_HEX = "(0(x|X))[0-9a-fA-F]+";

/*const string Formatter::REGEX_PARTS[] = {
	"((-)?([1-9][0-9]*)|((0(x|X))[0-9a-fA-F]+))",						//val
	"(([a-zA-Z_])(\\w)*)",												//sym
	"((((r|R)15)|((r|R)((0(x|X))?[(0-7)|f|F]))|(sp)|(pc)|(psw))(h|l)?)"	//reg
};*/

//const int REGEX_PARTS_SIZE = 3;

void Formatter::write(int16_t value, Size w)
{
	int16_t mask = w == BYTE ? 0xff : 0xffff;
	value &= mask;
	out << setw(w) << value;
	my_asm->curr_section->location_counter += (w/2);
}

void Formatter::put(int16_t value, Size w)
{
	checkRange(value, w);
	if (w == WORD) swap16(value);
	out << setw(w) << value;
	my_asm->curr_section->location_counter += (w / 2);
}

void Formatter::reg_bits(unsigned& op_byte_1, string& reg) {
	if (reg != "") {
		int h = 0;
		char hl = reg.back();
		if (hl == 'h') { h = 1; reg.pop_back(); }
		else if (hl == 'l') reg.pop_back();

		int rind = 0;
		if (reg == "r15" || reg == "r0xf" || reg == "psw") rind = 15;
		else if (reg == "sp") rind = 6;
		else if (reg == "pc") rind = 7;
		else {
			reg.erase(0, 1); //r
			rind = stoi(reg);
		}

		rind <<= 1;
		op_byte_1 |= rind;
		//postavljanje bita h/l
		if (hl == 'h') h = 1;
		//najnizi bit je h/l bit
		op_byte_1 |= h; 
	}
	if ((op_byte_1 & ~0xff) != 0) { cout << my_asm->line_counter << "Greska u kodovanju reg_bits" << endl; exit(-1); }
	
	write(op_byte_1, BYTE);
}

void Formatter::val_bits(string& val_string, Size w) {
	int16_t val = 0;
	if(!stoint16(val_string, val)) {
		cout << my_asm->line_counter << "FATAL ERROR" << endl; exit(-1);
	}
	put(val, w);
}

void Formatter::sym_bits(string& symbol_name, RelocTable::RelocType reloc_type, int16_t add_op2_size) {
	int16_t offset = my_asm->sym_table->getCode(symbol_name, reloc_type) + add_op2_size; 
	put(offset, WORD);
}

void Formatter::checkRange(int val, Size bytes) {
	int deg = bytes == Size::BYTE ? 7 : 15;
	int minint = 2 << (deg - 1);
	int maxint = (2 << (deg - 1)) - 1;
	if (val > maxint || val < -minint) {
		cout << "[" << my_asm->line_counter << "]" << "Prekoracenje opsega" << endl;
		exit(70);
	}
}


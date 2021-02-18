#ifndef SYMTABLE_H
#define SYMTABLE_H
#include <iostream>
#include <vector>
#include <cstdint>
#include "reloc_table.h"
class Assembler;
struct Section;
struct SymTableEntry;

class SymTable {
public:
	enum Scope { LOCAL, GLOBAL };
	static const std::int16_t OFFSET_UNKNOWN;

	SymTable(Assembler* a) : my_asm(a) {}

	SymTableEntry * put(std::string& name, Section* section, std::int16_t location, Scope scope);
	SymTableEntry * getByName(std::string & name);

	std::int16_t getCode(std::string& name, RelocTable::RelocType reloc_type);

	friend std::ostream& operator<<(std::ostream& o, const SymTable& table);
private:
	Assembler* my_asm;
	//0 sluzi da se oznaci nedefinisano
	int next_symbol_id = 1;

	std::vector<SymTableEntry*> table;
};

#endif // !SYMTABLE_H


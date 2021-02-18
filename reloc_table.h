#ifndef RELOC_TABLE_H
#define RELOC_TABLE_H
#include <vector>
#include <iostream>
#include <cstdint>
class Assembler;
struct Section;
struct RelocTableEntry;

class RelocTable {
public:
	enum RelocType { R_16, R_PC16 };

	RelocTable(Assembler* a) : my_asm(a) {}

	void put(Section* to_fix, std::int16_t offset, RelocType type, int symbol_id, bool reloc_neg = false);

	friend std::ostream& operator<<(std::ostream& o, const RelocTable& tab);
private:
	Assembler* my_asm;
	std::vector<RelocTableEntry*> table;
};

#endif // !RELOC_TABLE_H


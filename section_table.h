#ifndef SECTION_TABLE_H
#define SECTION_TABLE_H
#include <vector>
#include <iostream>

class Assembler;
struct Section;
using Flags = unsigned int;

class SectionTable {
public:
	static const Flags READ;
	static const Flags WRITE;
	static const Flags EXECUTE;

	SectionTable(Assembler* a) : my_asm(a) {}

	void changeSection(std::string& name, Flags flags);
	void resetLocationCounters();

	friend std::ostream& operator<<(std::ostream& o, const SectionTable& table);
private:
	Assembler* my_asm;
	std::vector<Section*> table;
};

#endif // !SECTION_TABLE_H


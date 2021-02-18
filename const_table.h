#ifndef CONST_TABLE
#define CONST_TABLE
#include <vector>
#include <iostream>
class Assembler;
struct ConstTableEntry;

class ConstTable {
public:
	ConstTable(Assembler* a) : my_asm(a) {}
	ConstTableEntry* get(std::string& name);
	void put(std::string& name, int16_t value);
	friend std::ostream& operator<<(std::ostream& out, ConstTable& tab);
private:
	Assembler* my_asm;

	std::vector<ConstTableEntry*> table;
};

#endif // !CONST_TABLE


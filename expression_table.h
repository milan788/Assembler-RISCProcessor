#ifndef EXPRESSION_TABLE
#define EXPRESSION_TABLE
#include <vector>
#include <iostream>
class Assembler;
struct ExpressionTableEntry;

class ExpressionTable {
public:
	enum OPERATION {NONE, ADD, SUB};

	ExpressionTable(Assembler* a) : my_asm(a) {}

	//void put(string& name);

	ExpressionTableEntry* get(std::string& name);

	void put(ExpressionTableEntry* entry);

	void resolve();
	friend std::ostream& operator<<(std::ostream& out, ExpressionTable& tab);
private:
	Assembler* my_asm;
	//0 sluzi da se oznaci nedefinisano
	//mozda nije potrebno
	int next_symbol_id = 1;
	std::vector<ExpressionTableEntry*> table;
};

#endif // !EXPRESSION_TABLE


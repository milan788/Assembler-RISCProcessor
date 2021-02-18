#include "const_table.h"
#include "entries.h"
#include <iostream>
#include <iomanip>
using namespace std;

ConstTableEntry* ConstTable::get(string& name) {
	for (int i = 0; i < table.size(); i++) {
		if (table[i]->name == name) return table[i];
	}
	return nullptr;
}

void ConstTable::put(string& name, int16_t value) {
	table.push_back(new ConstTableEntry(name, value));
}

ostream& operator<< (ostream& out, ConstTable& tab) {
	for (int i = 0; i < tab.table.size(); i++) {
		ConstTableEntry* entry = tab.table[i];
		out << setw(15) << entry->name
			<< setw(15) << entry->value << endl;
	}
	return out;
}
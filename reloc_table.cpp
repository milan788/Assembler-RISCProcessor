#include "reloc_table.h"
#include "entries.h"
#include <iostream>
#include <vector>
#include <iomanip>
#include <cstdint>
using namespace std;

void RelocTable::put(Section * to_fix, int16_t offset, RelocType type, int symbol_id, bool reloc_neg)
{
	table.push_back(new RelocTableEntry(to_fix, offset, type, symbol_id, reloc_neg));
}

ostream& operator<<(ostream& o, const RelocTable& tab) {
	o << "#rel_table" << setfill(' ') << endl;
	
	o << setw(10) << "sym_id"
	  << setw(10) << "section"
	  << setw(10) << "type"
      << setw(10) << "location" << endl;
	
	for (int i = 0; i < tab.table.size(); i++) {
		RelocTableEntry* entry = tab.table[i];
		
		int id = entry->symbol_id;
		int section = entry->to_fix->id;
		int type = entry->type;
		int16_t offset = entry->location;
		
		o << setw(9) << " " << id
		  << setw(9) << " " << section
	      << setw(9) << " " << type
		  << "     " << " " << hex << setw(4) << setfill('0') << offset << setfill(' ') << endl;
	}
	return o;
}

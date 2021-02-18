#include <iostream>
#include <vector>
#include <iomanip>
#include "assembler.h"
#include "section_table.h"
#include "entries.h"
using namespace std;

const Flags SectionTable::READ = 0x4;
const Flags SectionTable::WRITE = 0x2;
const Flags SectionTable::EXECUTE = 0x1;
//	  int	SectionTable::next_id = 1; //

void SectionTable::changeSection(string & name, Flags flags)
{
	//id sekcije je isti kao id u tabeli simbola
	Section* ret = nullptr;
	for (int i = 0;  i < table.size(); i++) {
		if (table[i]->name == name) {
			ret = table[i];
			break;
		}
	}
	if (!ret) {
		//da bi sekcija dobila id, section polje se postavi kasnije
		this->my_asm->sym_table->put(name, nullptr, 0, SymTable::LOCAL);
		//dohvatanje novog simbola
		SymTableEntry* section_symbol = my_asm->sym_table->getByName(name);
		//njegov id
		int id = section_symbol->id;
		table.push_back(ret = new Section(name, flags, id));
		//postvaljanje polja za sekciju
		section_symbol->section = ret;
	}
	//postavljanje nove sekcije
	my_asm->curr_section = ret;
}

void SectionTable::resetLocationCounters()
{
	for (int i = 0; i < table.size(); i++)
		table[i]->location_counter = 0;
}

ostream& operator<<(ostream& o, const SectionTable& tab)
{
	o << "#sec_table" << setfill(' ') << endl;
	
	o << setw(10) << "id"
	  << setw(15) << "name"
	  << setw(7)  << "flags"
	  << setw(10) << "size" << endl;
	
	for (int i = 0; i < tab.table.size(); i++) {
		Section* entry = tab.table[i];
		
		int				id = entry->id;
		const string&	name = entry->name;
		Flags			flags = entry->flags;
		int16_t			size = entry->location_counter;

		o << " " << setw(9)  <<  id
		  << " " << setw(14) << name
		  << " " << setw(6)  << flags
		  << " " << setw(9)  << size << endl;
	}
	return o;
}

#include "sym_table.h"
#include "entries.h"
#include "assembler.h"
#include "reloc_table.h"
#include "const_table.h"
#include "expression_table.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <cstdint>
using namespace std;

const int16_t SymTable::OFFSET_UNKNOWN = -1;

SymTableEntry* SymTable::put(string & name, Section* section, int16_t location, SymTable::Scope scope)
{
	SymTableEntry* entry = getByName(name);
	if (entry && entry->scope == GLOBAL && entry->section == nullptr) {
		entry->offset = location;
		entry->section = section;
		return entry;
	}
	if (entry && entry->scope == LOCAL) {
		cout << '[' << my_asm->getLineNum() << "] " << "Simbol vec postoji u tabeli" << endl; exit(20);
	}
	SymTableEntry* ret;
	table.push_back(ret = new SymTableEntry(name, section, location, scope, next_symbol_id++));
	return ret;
}

SymTableEntry* SymTable::getByName(string& name) {
	SymTableEntry* ret = nullptr;
	for (int i = 0; i < table.size(); i++) {
		ret = table[i];
		if (name == ret->name) {
			return ret;
		}
	}
	return nullptr;
}

//Izracunati ispravno pomeraj do sledece instr
int16_t SymTable::getCode(string& symbol_name, RelocTable::RelocType reloc_type) {
	int16_t ret = 0;
	SymTableEntry* symbol = getByName(symbol_name);
	if (symbol) {
		//simbol postoji u tabeli simbola
		if (symbol->scope == SymTable::GLOBAL) {

			ret = reloc_type == RelocTable::RelocType::R_16 ? 0 : -2;
			//i pcrel se svodi na regindpom			
			//relokacioni zapis za simbol
			my_asm->rel_table->put(my_asm->curr_section, my_asm->curr_section->location_counter, reloc_type, symbol->id);
		}
		if (symbol->scope == SymTable::LOCAL && symbol->section != my_asm->curr_section) {
			ret = reloc_type == RelocTable::RelocType::R_16 ? symbol->offset : symbol->offset - 2;
			//relokacioni zapis za sekciju simbola
			my_asm->rel_table->put(my_asm->curr_section, my_asm->curr_section->location_counter, reloc_type, symbol->section->id);
		}
		if (symbol->scope == SymTable::LOCAL && symbol->section == my_asm->curr_section) {
			ret = reloc_type == RelocTable::RelocType::R_16 ? symbol->offset : symbol->offset - 2 - my_asm->curr_section->location_counter;
			if(reloc_type == RelocTable::RelocType::R_PC16) 
				my_asm->rel_table->put(my_asm->curr_section, my_asm->curr_section->location_counter, reloc_type, symbol->section->id);
		}
	}
	else {
		//simbol ne postoji u tabeli simbola
		//trazi se u tabeli konstanti
		ConstTableEntry* const_table_entry = my_asm->const_table->get(symbol_name);
		if (const_table_entry) ret = const_table_entry->value;
		//trazi se u tabeli izraza
		ExpressionTableEntry* expression_table_entry = my_asm->expression_table->get(symbol_name);
		if (expression_table_entry) {
			ret = expression_table_entry->value_to_write;
			my_asm->rel_table->put(my_asm->curr_section, my_asm->curr_section->location_counter, reloc_type, expression_table_entry->refered_symbol_id, expression_table_entry->reloc_neg);
		}
	}
	return ret;
}

ostream& operator<<(ostream& o, const SymTable& tab)
{
	o << "#sym_table" << setfill(' ') << endl;
	
	o << setw(10) << "id" 
	  << setw(14) << "name" 
	  << setw(11)  <<"offset" 
	  << setw(10) << "scope" 
	  << setw(10) << "section" << endl;
	
	for (int i = 0; i < tab.table.size(); i++) {
		SymTableEntry* entry = tab.table[i];
		
		int				id = entry->id;
		const string& name = entry->name;
		int16_t		offset = entry->offset;
		if (offset == SymTable::OFFSET_UNKNOWN) 
					offset = 0;
		else		offset = entry->offset;
		int			scope = entry->scope == SymTable::LOCAL ? 0 : 1;
		int		    section;
		if (entry->section)
				    section = tab.table[i]->section->id;
		else	    section = 0;
		
		o << " " << setw(9)  << id
		  << " " << setw(13) << name
		  << " " << "      "  << hex << setw(4) << setfill('0') << offset << setfill(' ')
		  << " " << setw(9)  << dec << scope
		  << " " << setw(9)  << section << endl;
	}
	return o;
}

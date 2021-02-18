#ifndef ENTRIES_H
#define ENTRIES_H
#include <iostream>
#include "sym_table.h"
#include "reloc_table.h"
#include "section_table.h"
#include "expression_table.h"
#include <cstdint>

struct Section {
	Section(std::string n, Flags f, int i) :
		name(n),
		flags(f),
		id(i)
	{}
	~Section() {}
	int id;
	std::string name;
	Flags flags;
	//na kraju, location counter predstavlja velicinu sekcije
	std::int16_t location_counter = 0;
	
};

struct SymTableEntry {
	SymTableEntry(std::string& n, Section* s, int o, SymTable::Scope sc, int i): 
		name(n),
		section(s),
		offset(o),
		scope(sc),
		id(i) 
	{}
	std::string name;
	Section* section;
	std::int16_t offset;
	SymTable::Scope scope;
	int id;
};

struct RelocTableEntry {
	RelocTableEntry(Section* section, int o, RelocTable::RelocType t, int s, bool neg = false) :
		to_fix(section),
		location(o),
		type(t),
		symbol_id(s),
		negative(neg)
	{}
	Section* to_fix;
	std::int16_t location;
	RelocTable::RelocType type;
	int symbol_id;
	bool negative;
};

struct ExpressionTableEntry {
	ExpressionTableEntry(std::string n, std::string o1, bool o1_res, std::string o2, bool o2_res, unsigned oper, int rsid, int16_t vtw, bool r_neg = false) :
		name(n),
		op1(o1),
		op1_resolved(o1_res),
		op2(o2),
		op2_resolved(o2_res),
		operation(oper),
		refered_symbol_id(rsid),
		value_to_write(vtw),
		reloc_neg(r_neg)
	{}
	std::string name;
	
	std::string op1;
	bool		op1_resolved;

	std::string op2;
	bool		op2_resolved;

	unsigned operation;
	int refered_symbol_id;
	std::int16_t value_to_write;
	bool reloc_neg;
};

struct ConstTableEntry {
	ConstTableEntry(std::string n, int16_t v) : name(n), value(v) {}
	std::string name;
	int16_t value;
};

#endif // !ENTRIES_H

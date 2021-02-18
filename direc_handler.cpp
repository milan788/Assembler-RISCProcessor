#include "direc_handler.h"
#include "assembler.h"
#include "sym_table.h"
#include "entries.h"
#include "const_table.h"
#include "regex_patterns.h"
#include <iostream>
#include <sstream>
#include <regex>
#include <cstdint>
using namespace std;

const string DirecHandler::DIRECTIVES[5][4] = {
	{".byte"  , ".word"  , ""       , ""}, 
	{".align" , ".skip"  , ""       , ""},
	{".text"  , ".data"  , ".bss"   , ".section"}, 
	{".global", ".extern", ""       , ""}, 
	{".equ"   , ""       , ""       , ""}
};

const int DirecHandler::ROWS = 5;
const int DirecHandler::COLS = 4;

const string DirecHandler::PATTERNS[] = {
	(string)VAL, //.byte .word
	"(-)?([1-9][0-9]*)|((0(x|X))[0-9a-fA-F]+)", // .align .skip
	"\\." + (string)SYM + "( )*,( )*r(w)?(x)?", //.section
	"(" + (string)SYM + ")(( )*,( )*(" + (string)SYM + "))*", //.global .extern
	(string)SYM + "(( )*,( )*((" + (string)SYM + "|" + (string)VAL + ")((\\+|-)(" + (string)SYM + "|" + (string)VAL + "))?))" , //.equ
};

const int DirecHandler::PATTERNS_SIZE = 4;

bool DirecHandler::pass(string* dir)
{
	reset();
	set(dir);
	if (*direct == ".end") return true;
	if(!my_asm->second_pass) validate();
	switch (dir_type)
	{
	case ALOC:
		alocDir();
		break;
	case ALIGN_SKIP:
		align_skip();
		break;
	case SECTION:
		sectionDir();
		break;
	case GLOBAL:
		globalDir();
		break;
	case EQU:
		equDir();
		break;
	}
	return false;
}

void DirecHandler::reset()
{
	if (direct) { delete direct; direct = nullptr; }
	if (params) { delete params; params = nullptr; }
	dir_type = DirecType::UNKOWN_DIREC_TYPE;
	dir_indx = -1;
}

void DirecHandler::set(std::string * dir)
{
	stringstream sstream(*dir);
	direct = new string(""); sstream >> *direct; 
	params = new string(""); //sstream >> *params;
	sstream >> ws;
	getline(sstream, *params);
	for (int i = 0; i < ROWS; i++) {
		for (int j = 0; j < COLS; j++) {
			if (DIRECTIVES[i][j] == *direct) {
				dir_type =(DirecType) i;
				dir_indx = j;
				return;
			}
		}
	}
	dir_indx = dir_type = DirecType::UNKOWN_DIREC_TYPE;
}

void DirecHandler::validate() 
{
	//nepoznata direktiva
	if (dir_type == DirecType::UNKOWN_DIREC_TYPE) 
	{ cout << "[" << my_asm->getLineNum() << "]" << "Nepoznata direktiva" << endl; exit(29); }
	//greska u parametrima za .section
	if(dir_type == DirecType::SECTION && dir_indx != 3) return;
	else { //ostale imaju parametre
		//za dir za alokaciju se samo odredi da li je broj ako postoji
		string target = *params;
		if (!regex_match(target, regex(PATTERNS[dir_type]))) {
			cout << "[" << my_asm->getLineNum() << "]" << "Greska u sintaksi" << endl; exit(30);
		}
	}
	//greske u aloc direktivama
	if (dir_type == DirecType::ALOC) {
		if (dir_indx < 2) {
			//proveriti velicinu broja
			int param_num = stoi(*params);
			my_asm->formatter.checkRange(param_num, Formatter::WORD);
		}
		else if (params->length() == 0) {
			cout << '[' << my_asm->line_counter << ']' << "Direktiva zahteva parametar" << endl;
		}
	}
	//.global ili .extern
	if (dir_type == GLOBAL && my_asm->curr_section) 
	{ cout << "[" << my_asm->line_counter << "]" << "Direktiva mora da stoji van sekcije" << endl; exit(300); }
	//aloc ili .equ direktiva je van sekcije
	if (dir_type == DirecType::ALOC && !my_asm->curr_section) 
	{ cout << '[' << my_asm->line_counter << ']' << "Direktiva mora da se nalazi u sekciji" << endl; exit(33); }
}

void DirecHandler::alocDir() 
{
	//num ima ima razlicito znacenje
	int16_t num = 0;
	if (*params != "") {
		int base = 0;
		if (regex_match(*params, regex(Formatter::REGEX_DEC))) base = 10;
		else if (regex_match(*params, regex(Formatter::REGEX_HEX))) base = 16;
		num = stoi(*params, nullptr, base);
	}
	int16_t off = 0;
	int16_t init = 0;
	//.byte ili .word
	if (dir_indx < 2) {
		Formatter::Size w = dir_indx == 0 ? Formatter::BYTE : Formatter::WORD;
		init = num;
		if (my_asm->second_pass)
			my_asm->formatter.put(init, w);
		else my_asm->curr_section->location_counter += w/2;
	}
}

void DirecHandler::align_skip() {
	int16_t num = 0;
	int base = 0;
	if (regex_match(*params, regex(Formatter::REGEX_DEC))) base = 10;
	else if (regex_match(*params, regex(Formatter::REGEX_HEX))) base = 16;
	num = stoi(*params, nullptr, base);
	int16_t off = 0;
	int16_t init = 0;
	//.align
	if (dir_indx == 0) {
		int16_t mask = ~0U << num; //...111100..
		int16_t add = 1U << num; //..000100..
		int16_t new_lc = my_asm->curr_section->location_counter;
		int16_t old_lc = my_asm->curr_section->location_counter;
		new_lc &= mask; // brisu se zajdnjih num jedinica
		if (((my_asm->curr_section->location_counter) & (~mask)) > 0) //ako je neki od obrisanih bitova 1, dodaj add
			new_lc += add;
		//my_asm->location_counter = new_lc;
		off = new_lc - old_lc;
		init = 0;
		for (int i = 0; i < off; i++) {
			if (my_asm->second_pass)
				my_asm->formatter.write(init, Formatter::BYTE);
			else my_asm->curr_section->location_counter++;
		}
		return;
	}
	//.skip
	if (dir_indx == 1) {
		off = num;
		init = 0;
		for (int i = 0; i < off; i++) {
			if (my_asm->second_pass)
				my_asm->formatter.write(init, Formatter::BYTE);
			else my_asm->curr_section->location_counter++;
		}
	}
}

void DirecHandler::sectionDir() 
{
	string sec_name;
	Flags flags = 0;
	if (dir_indx != 3) { //direktive koje nisu .section
		sec_name = *direct;
		if (dir_indx == 0) flags = SectionTable::READ | SectionTable::EXECUTE; //.text
		else flags = SectionTable::READ | SectionTable::WRITE; //.data .bss
	}
	else { //.section direktiva
		string sec_flgs;
		stringstream sstream(*params);

		getline(sstream, sec_name, ',');
		sstream >> sec_flgs;

		size_t i = 0;
		while (i < sec_flgs.length()) {
			char c = sec_flgs[i++];
			if (c == 'r') flags |= SectionTable::READ;
			if (c == 'w') flags |= SectionTable::WRITE;
			if (c == 'x') flags |= SectionTable::EXECUTE;
		}
	}
	my_asm->sec_table->changeSection(sec_name, flags);
}

void DirecHandler::globalDir() 
{
	stringstream sstream(*params);
	string sym_name;
	sstream >> ws;
	while (getline(sstream, sym_name, ',')) {
		if (!my_asm->second_pass) {
			SymTableEntry* symbol = my_asm->sym_table->getByName(sym_name);
			if (!symbol) {
				my_asm->sym_table->put(
					sym_name, 
					nullptr, 
					SymTable::OFFSET_UNKNOWN, 
					SymTable::GLOBAL
				);
			}
			else {
				//.global
				if(dir_indx == 0) symbol->scope = SymTable::GLOBAL;
				//.extern
				else {
					cout << "[" << my_asm->line_counter << "]" << "Simbol " << symbol->name << "je definisan" << endl; exit(301);
				}
			}
		}
		else {
			if (dir_indx == 1) {
				SymTableEntry* symbol = my_asm->sym_table->getByName(sym_name);
				if(symbol && symbol->section != nullptr) {
					cout << "[" << my_asm->line_counter << "]" << "Simbol " << symbol->name << "je definisan" << endl; exit(301);
				}
			}
		}
		sstream >> ws;
	}
}

void DirecHandler::equDir() {
	if (my_asm->second_pass) return;
	//operacija
	unsigned operation = ExpressionTable::NONE; //0 - ne postoji, 1 - '+', 2 - '-'
	size_t operation_index = params->find('+');
	if (operation_index != string::npos) {
		operation = ExpressionTable::ADD;
		params->replace(operation_index, 1, 1, ' ');
	}
	else {
		operation_index = params->find('-');
		if (operation_index != string::npos) {
			operation = ExpressionTable::SUB;
			params->replace(operation_index, 1, 1, ' ');
		}
	}
	size_t pos = params->find(',');
	params->replace(pos, 1, 1, ' ');
	stringstream sstream(*params);
	//ime novog simbola
	string sym_name;
	//operandi
	string op1 = "", op2 = "";
	bool rel_neg = false;
	sstream >> sym_name >> op1 >> op2;

	SymTableEntry* sym_table_entry1 = my_asm->sym_table->getByName(op1);
	ExpressionTableEntry* exp_table_entry1 = my_asm->expression_table->get(op1);
	ConstTableEntry* const_table_entry1 = my_asm->const_table->get(op1);
	int16_t const_part1 = 0;
	int sym_for_reloc1 = 0;
	bool op1_resolved = false;
	if (Formatter::stoint16(op1, const_part1)) {
		op1_resolved = true;
	}
	else if (const_table_entry1) {
		op1_resolved = true;
		const_part1 = const_table_entry1->value;
	}
	else if (sym_table_entry1) {
		op1_resolved = true;
		if (sym_table_entry1->scope == SymTable::LOCAL) {
			const_part1 = sym_table_entry1->offset;
			sym_for_reloc1 = sym_table_entry1->section->id;
		}
		else {
			const_part1 = 0;
			sym_for_reloc1 = sym_table_entry1->id;
		}
	}

	SymTableEntry* sym_table_entry2 = my_asm->sym_table->getByName(op2);
	ExpressionTableEntry* exp_table_entry2 = my_asm->expression_table->get(op2);
	ConstTableEntry* const_table_entry2 = my_asm->const_table->get(op2);
	int16_t const_part2 = 0;
	int sym_for_reloc2 = 0;
	bool op2_resolved = false;
	if (op2.length() > 0) {
		if (Formatter::stoint16(op2, const_part2)) {
			op2_resolved = true;
		}
		else if (const_table_entry2) {
			op2_resolved = true;
			const_part2 = const_table_entry2->value;
		}
		else if (sym_table_entry2) {
			op2_resolved = true;
			if (operation == ExpressionTable::SUB) rel_neg = true;
			if (sym_table_entry2->scope == SymTable::LOCAL) {
				const_part2 = sym_table_entry2->offset;
				sym_for_reloc2 = sym_table_entry2->section->id;
			}
			else {
				const_part2 = 0;
				sym_for_reloc2 = sym_table_entry2->id;
			}
		}
		if (operation == ExpressionTable::SUB) const_part2 = -const_part2;
	} else op2_resolved = true;

	int16_t value_to_write = const_part1 + const_part2;
	int refered_symbol_id = 0;
	if (sym_for_reloc1 > 0 && sym_for_reloc2 == 0) refered_symbol_id = sym_for_reloc1;
	else if (sym_for_reloc1 == 0 && sym_for_reloc2 > 0) refered_symbol_id = sym_for_reloc2;
	else if (sym_for_reloc1 == sym_for_reloc2) refered_symbol_id = 0;
	else {
		cout << "Nedozvoljen izraz" << endl; exit(50);
	}

	if (op1_resolved && op2_resolved && refered_symbol_id == 0) my_asm->const_table->put(sym_name, value_to_write); //u tabelu constanti;
	else my_asm->expression_table->put(
			new ExpressionTableEntry(sym_name, op1, op1_resolved, op2, op2_resolved, operation, refered_symbol_id, value_to_write, rel_neg
		));
}


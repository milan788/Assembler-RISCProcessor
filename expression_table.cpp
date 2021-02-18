#include "expression_table.h"
#include "entries.h"
#include "assembler.h"
#include "const_table.h"
#include <iostream>
#include <iomanip>
using namespace std;


ExpressionTableEntry* ExpressionTable::get(string& name) {
	for (int i = 0; i < table.size(); i++) {
		if (table[i]->name == name) return table[i];
	}
	return nullptr;
}

void ExpressionTable::put(ExpressionTableEntry* entry) {
	table.push_back(entry);
}

void ExpressionTable::resolve() {
	vector<int> entries_to_remove;
	while (true) {
		int changes_cntr = 0;
		for (int i = 0; i < table.size(); i++) {
			ExpressionTableEntry* entry = table[i];

			if (!entry->op1_resolved) {
				//da li je u tableli konstanti
				ConstTableEntry* const_tab_entry = my_asm->const_table->get(entry->op1);
				SymTableEntry* sym_tab_entry = my_asm->sym_table->getByName(entry->op1);
				ExpressionTableEntry* exp_tab_entry = my_asm->expression_table->get(entry->op1);
				if (const_tab_entry) {
					entry->value_to_write += const_tab_entry->value;
					if (entry->op2_resolved && entry->refered_symbol_id == 0) {
						//u tabelu kontatni ide
						my_asm->const_table->put(entry->name, entry->value_to_write);
						entries_to_remove.push_back(i);
					}
					entry->op1_resolved = true; changes_cntr++;
				}
				//da li je u tabli simbola
				else if (sym_tab_entry) {
					int sym_for_reloc = sym_tab_entry->scope == SymTable::LOCAL ? sym_tab_entry->section->id : sym_tab_entry->id;
					entry->value_to_write += sym_tab_entry->scope == SymTable::LOCAL ? sym_tab_entry->offset : 0;
					if (entry->refered_symbol_id == sym_for_reloc) {
						//ide u tabelu konstanti
						my_asm->const_table->put(entry->name, entry->value_to_write);
						entries_to_remove.push_back(i);
					}
					else if (entry->refered_symbol_id == 0) entry->refered_symbol_id = sym_for_reloc;
					else {
						exit(50);
					}
					entry->op1_resolved = true; changes_cntr++;
				}
				//da li je u tabli izraza
				else if (exp_tab_entry && exp_tab_entry->op1_resolved && exp_tab_entry->op2_resolved) {
					entry->value_to_write += exp_tab_entry->value_to_write;
					
					if (entry->refered_symbol_id == 0 && exp_tab_entry->refered_symbol_id > 0)
						entry->refered_symbol_id = exp_tab_entry->refered_symbol_id;
					else if (entry->refered_symbol_id == exp_tab_entry->refered_symbol_id) {
						//u tabelu konstanti
						my_asm->const_table->put(entry->name, entry->value_to_write);
						entries_to_remove.push_back(i);
					}
					else {
						exit(50);
					}
					entry->op1_resolved = true; changes_cntr++;
				}
			}

			if (!entry->op2_resolved) {
				//da li je u tableli konstanti
				ConstTableEntry* const_tab_entry = my_asm->const_table->get(entry->op2);
				SymTableEntry* sym_tab_entry = my_asm->sym_table->getByName(entry->op2);
				ExpressionTableEntry* exp_tab_entry = my_asm->expression_table->get(entry->op2);
				if (const_tab_entry) {
					int16_t value = entry->operation == ExpressionTable::ADD ? const_tab_entry->value : -const_tab_entry->value;
					entry->value_to_write += value;
					if (entry->op1_resolved && entry->refered_symbol_id == 0) {
						//u tabelu kontatni ide
						my_asm->const_table->put(entry->name, entry->value_to_write);
						entries_to_remove.push_back(i);
					}
					entry->op2_resolved = true; changes_cntr++;
				}
				//da li je u tabli simbola
				else if (sym_tab_entry) {
					int sym_for_reloc = sym_tab_entry->scope == SymTable::LOCAL ? sym_tab_entry->section->id : sym_tab_entry->id;
					int16_t value = entry->operation == sym_tab_entry->scope == SymTable::LOCAL ? sym_tab_entry->offset : 0;
					value = entry->operation == ExpressionTable::ADD ? value : -value;
					entry->value_to_write += value;
					if (entry->refered_symbol_id == sym_for_reloc) {
						//ide u tabelu konstanti
						my_asm->const_table->put(entry->name, entry->value_to_write);
						entries_to_remove.push_back(i);
					}
					else if (entry->refered_symbol_id == 0) entry->refered_symbol_id = sym_for_reloc;
					else {
						exit(50);
					}
					entry->op2_resolved = true; changes_cntr++;
				}
				//da li je u tabli izraza
				else if (exp_tab_entry && exp_tab_entry->op1_resolved && exp_tab_entry->op2_resolved) {
					int16_t value = entry->operation == 
						ExpressionTable::ADD ? exp_tab_entry->value_to_write : -exp_tab_entry->value_to_write;
					entry->value_to_write += value;

					if (entry->refered_symbol_id == 0 && exp_tab_entry->refered_symbol_id > 0)
						entry->refered_symbol_id = exp_tab_entry->refered_symbol_id;
					else if (entry->refered_symbol_id == exp_tab_entry->refered_symbol_id) {
						//u tabelu konstanti
						if (entry->refered_symbol_id > 0 && ((exp_tab_entry->reloc_neg && entry->operation == SUB) || 
							(!exp_tab_entry->reloc_neg && entry->operation == ADD))) 
							exit(50);
						my_asm->const_table->put(entry->name, entry->value_to_write);
						entries_to_remove.push_back(i);
					}
					else {
						exit(50);
					}
					entry->op2_resolved = true; changes_cntr++;
				}
			}
			
		}
		if (changes_cntr == 0) break;
	}
	//proveravanje da li su svi simboli razreseni
	int unresolved = 0;
	for (int i = 0; i < table.size(); i++) {
		ExpressionTableEntry* entry = table[i];
		//ako je se ne zna gde je simbol koji se referise, odstampa ju se informacije
		if (!entry->op1_resolved) {
			cout << "[" << my_asm->line_counter << "] " << "Nerazresen simbol: " << entry->op1 << endl;
			unresolved++;
		}
		if (!entry->op2_resolved) {
			cout << "[" << my_asm->line_counter << "] " << "Nerazresen simbol: " << entry->op2 << endl;
			unresolved++;
		}
	}
	if (unresolved > 0) exit(1000);
	//izbacivanje ulaza koji su konstante
	for (int i = 0; i < entries_to_remove.size(); i++) {
		int min = entries_to_remove[i];
		for (int j = i + 1; j < entries_to_remove.size(); j++) {
			if (entries_to_remove[i] > entries_to_remove[j]) {
				int temp = entries_to_remove[i];
				entries_to_remove[i] = entries_to_remove[j];
				entries_to_remove[j] = temp;
			}
		}
	}
	for (int i = entries_to_remove.size() - 1; i >= 0; i--) {
		table.erase(table.begin() + entries_to_remove[i]);
	}
}

ostream& operator<< (ostream& out, ExpressionTable& tab) {
	for (int i = 0; i < tab.table.size(); i++) {
		ExpressionTableEntry* entry = tab.table[i];
		out << setw(15) << entry->name
			<< setw(15) << entry->op1
			<< setw(15) << entry->op1_resolved
			<< setw(15) << entry->op2
			<< setw(15) << entry->op2_resolved
			<< setw(15) << entry->refered_symbol_id
			<< setw(15) << entry->value_to_write << endl;
	}
	return out;
}

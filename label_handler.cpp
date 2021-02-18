#include "label_handler.h"
#include "assembler.h"
#include "entries.h"
#include "sym_table.h"
using namespace std;

void LabelHandler::pass(std::string * lab)
{
	if (my_asm->second_pass) return;
	if (!my_asm->curr_section) {
		cout << "[" << my_asm->line_counter << "] " << "Labela mora da se nalazi u sekciji" << endl; exit(40);
	}
	my_asm->sym_table->put(*lab, my_asm->curr_section, my_asm->curr_section->location_counter, SymTable::LOCAL);
}

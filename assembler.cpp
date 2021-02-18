#include <string>
#include "assembler.h"
#include "sym_table.h"
#include "reloc_table.h"
#include "line_analizer.h"
#include "direc_handler.h"
#include "label_handler.h"
#include "instr_handler.h"
#include "section_table.h"
#include "const_table.h"
#include "formatter.h"
#include "entries.h"
using namespace std;

Assembler::Assembler(fstream& i, fstream& o) :
	in(i), out(o),
	sym_table(new SymTable(this)),
	rel_table(new RelocTable(this)),
	sec_table(new SectionTable(this)),
	expression_table(new ExpressionTable(this)),
	const_table(new ConstTable(this)),
	formatter(this),
	line_analizer(this),
	direc_handler(this),
	instr_handler(this),
	label_handler(this),
	curr_section(nullptr)
{}

void Assembler::start()
{	
	//prvi prolaz
	pass();
	expression_table->resolve();
	//reset
	in.clear();
	in.seekg(0, ios::beg); //pozicioniranje na pocetak fajla
	line_counter = 0;
	sec_table->resetLocationCounters();
	//drugi prolaz
	second_pass = true;
	pass();
	out << dec << endl;
	out << *rel_table;
	out << *sym_table;
	out << *sec_table;
	out << "#expression_table" << endl << *expression_table << endl;
	out << "#const_table" << endl << *const_table << endl;
}

void Assembler::pass()
{
	string line;
	bool end = false;
	while (std::getline(in, line)) {
		if (line.length() == 0) continue;
		line_analizer.set(line);

		if (line_analizer.isLabel())
			label_handler.pass(line_analizer.getLabel());
		if (line_analizer.isInstr())
			instr_handler.pass(line_analizer.getInstr());
		if (line_analizer.isDirec())
			end = direc_handler.pass(line_analizer.getDirec());
		if (end) return;
		line_counter++;
	}
}

Assembler::~Assembler() {
	delete sym_table;
	delete rel_table;
	delete sec_table;
}

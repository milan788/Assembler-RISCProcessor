#ifndef ASSEMBLER_H
#define ASSEMBLER_H
#include <iostream>
#include <fstream>
#include "formatter.h"
#include "line_analizer.h"
#include "direc_handler.h"
#include "instr_handler.h"
#include "label_handler.h"

class RelocTable;
class SymTable;
class SectionTable;
class ConstTable;
class ExpressionTable;
struct Section;

class Assembler
{
public:
	Assembler(std::fstream& in, std::fstream& out);
	~Assembler();

	void start();
	int getLineNum() { return line_counter; }
private:
	bool second_pass = false;
	int line_counter = 1;
	//int16_t location_counter = 0;
	Section* curr_section;

	std::fstream& in;
	std::fstream& out;

	SymTable*		sym_table;
	RelocTable*		rel_table;
	SectionTable*	sec_table;
	ConstTable*		const_table;
	ExpressionTable* expression_table;
	Formatter	 formatter;
	LineAnalizer line_analizer;
	DirecHandler direc_handler;
	InstrHandler instr_handler;
	LabelHandler label_handler;

	void pass();

	friend class Formatter;
	friend class SymTable;
	friend class SectionTable;
	friend class RelocTable;
	friend class ConstTable;
	friend class ExpressionTable;
	friend class LineAnalizer;
	friend class DirecHandler;
	friend class InstrHandler;
	friend class LabelHandler;
};

#endif // !ASSEMBLER_H



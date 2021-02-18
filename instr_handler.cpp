#include "instr_handler.h"
#include "assembler.h"
#include "sym_table.h"
#include "entries.h"
#include "formatter.h"
#include "regex_patterns.h"
#include <sstream>
#include <iomanip>
#include <regex>
#include <cstdint>
using namespace std;


const string InstrHandler::INSTRUCTIONS[3][13] = { 
	{"halt", "ret", "iret", ""   , ""    , ""   , ""   , ""    , ""  ,  ""  , ""    , ""   , ""   },
	{"int" , "not", "push", "pop", "jmp" , "jeq", "jne", "jgt", "call", ""  , ""    , ""   , ""   },
    {"xchg", "mov", "add" , "sub", "mul" , "div", "cmp", "and" , "or", "xor", "test", "shl", "shr"} 
}; //13
const unsigned InstrHandler::INSTRC_CODES[3][13] = {
	{1, 24, 25,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
	{3, 10, 17, 18, 19, 20, 21, 22, 23,  0,  0,  0,  0},
	{2,  4,  5,  6,  7,  8,  9, 11, 12, 13, 14, 15, 16}
};

const int InstrHandler::ROWS = 3;
const int InstrHandler::COLS = 13;

const string InstrHandler::PATTERNS[] = {
	string(VAL),						//immed_val
	"&" + string(SYM),					//immed_sym
	string(REG),						//reg_dir
	"\\[" + string(REG) + "\\]",		//regind
	string(REG) + "\\[" + string(VAL) + "\\]",	//regind_pom_val
	string(REG) + "\\[" + string(SYM) + "\\]",	//regind_pom_sym
	"\\$" + string(SYM),				//pc_rel_sym
	string(SYM),						//mem_sym
	"\\*" + string(VAL)					//mem_val
};

const int InstrHandler::PATTERNS_SIZE = 9;


InstrHandler::InstrHandler(Assembler* const a) : my_asm(a)
{
	reset();
}

InstrHandler::~InstrHandler()
{
	reset();
}

void InstrHandler::pass(string * instr)
{
	set(instr);
	if (!my_asm->second_pass) {
		validate();
		my_asm->curr_section->location_counter += op1_size + op2_size + 1;
	}
	else {
		setCode();
		izvestaj();
	}
}

void InstrHandler::set(string * instr)
{
	reset();
	//instrukcija: name (razmak) op1,op2 (moze i razmak izmedju, a ne mora) 
	//umesto ',' se stavi ' '
	int found = instr->find(',');
	if (found > 0) instr->replace(found, 1, 1, ' ');

	stringstream sstream(*instr);
	string name;
	string o1;
	string o2;
	sstream >> name >> o1 >> o2;

	setTypeAndIndex(name);
	if(o1.length() != 0) op1 = new string(o1);
	if (o2.length() != 0) op2 = new string(o2);
	
	if(op1) setAdressMode(*op1, op1am, op1ams);
	if(op2) setAdressMode(*op2, op2am, op2ams);
	setSize();
}

void InstrHandler::reset()
{
	if(op1) delete op1; op1 = nullptr;
	if(op2) delete op2; op2 = nullptr;
	instr_type = InstrType::UNKNOWN_INSTR_TYPE;
	instr_index = -1;
	instr_size = 0;
	byte_operands = false;
	op1_size = 0;
	op2_size = 0;
	op1am = AdressMode::UNKNOWN_ADRESS_MODE;
	op2am = AdressMode::UNKNOWN_ADRESS_MODE;
	op1ams = AdressModeSyntax::UNKNOWN_ADRESS_MODE_SYNTAX;
	op2ams = AdressModeSyntax::UNKNOWN_ADRESS_MODE_SYNTAX;
	delete coded_instr;
	if(code_stream) delete code_stream;
}

void InstrHandler::setTypeAndIndex(string & name)
{
	if (name != "sub") {
		char c = name[name.length() - 1];
		if (c == 'w' || c == 'b') {
			if (c == 'b') byte_operands = true;
			name.erase(name.length() - 1);
		}
	}
	for (int i = 0; i < ROWS; i++) {
		for (int j = 0; j < COLS; j++) {
			if (name == INSTRUCTIONS[i][j]) {
				instr_type = (InstrType)i;
				instr_index = j;
				return;
			}
		}
	}
	instr_type = InstrType::UNKNOWN_INSTR_TYPE;
	instr_index = -1;
}

void InstrHandler::setAdressMode(string& op, AdressMode& opam, AdressModeSyntax& opams) 
{
	for (int i = 0; i < PATTERNS_SIZE; i++) {
		if (regex_match(op, regex(PATTERNS[i]))) {
			opams = (AdressModeSyntax)i;
			break;
		}
	}
	string val_str;
	size_t pos1, pos2, size;
	switch (opams)
	{
	case AdressModeSyntax::immed_val:
	case AdressModeSyntax::immed_sym:
		opam = AdressMode::immed;
		break;
	case AdressModeSyntax::reg_dir:
		opam = AdressMode::regdir;
		break;
	case AdressModeSyntax::reg_ind:
		opam = AdressMode::regind;
		break;
	case AdressModeSyntax::regind_pom_val:
		pos1 = op.find('[') + 1; pos2 = op.find(']');
		size = pos2 - pos1;
		val_str = op.substr(pos1, size);
		int16_t val;
		if (regex_match(val_str, regex(Formatter::REGEX_DEC)))
			val = stoi(val_str, nullptr, 10);
		else if (regex_match(val_str, regex(Formatter::REGEX_HEX)))
			val = stoi(val_str, nullptr, 16);
		else { cout << "[" << my_asm->line_counter << "]" << "Fatal error" << endl; exit(6); }
		
		if (val < 127 && val > -128) opam = AdressMode::regindpom8;
		else if (val < (2 << 15) - 1 && val > -(2 << 15)) opam = AdressMode::regindpom16;
		else { cout << "[" << my_asm->line_counter << "]" << "Prekoracenje opsega" << endl; exit(7); }
		
		break;
	case AdressModeSyntax::regind_pom_sym:
	case AdressModeSyntax::pc_rel_sym:
		opam = AdressMode::regindpom16;
		break;
	case AdressModeSyntax::mem_sym:
	case AdressModeSyntax::mem_val:
		opam = AdressMode::memdir;
		break;
	default:
		opam = AdressMode::UNKNOWN_ADRESS_MODE;
		break;
	}
}

void InstrHandler::validate()
{
	//Instrukcija nije u ispravnoj sekciji
	if (!my_asm->curr_section || (my_asm->curr_section->flags & (SectionTable::READ | SectionTable::EXECUTE)) == 0) 
	{ cout << "[" << my_asm->getLineNum() << "]" << "Instrukcija nije u ispravnoj sekciji" << endl; exit(10); }
	//Nepoznata instrukcija
	if (instr_type == InstrType::UNKNOWN_INSTR_TYPE || instr_index == -1)
	{ cout << "[" << my_asm->getLineNum() << "]" << "Nepoznata instrukcija" << endl; exit(11); }
	//Nepoznato adresiranje
	if ((op1am == AdressMode::UNKNOWN_ADRESS_MODE && !instr_type == InstrType::NOO_OPER) 
		|| (op2am == AdressMode::UNKNOWN_ADRESS_MODE && instr_type==InstrType::TWO_OPER))
	{ cout << "[" << my_asm->getLineNum() << "]" << "Nepoznato adresiranje" << endl; exit(12); }
	//Tip instr se ne slaze sa brojem operanada
	if (
		(instr_type == InstrType::NOO_OPER && (op1 || op2)) ||
		(instr_type == InstrType::ONE_OPER && (!op1 || op2)) ||
		(instr_type == InstrType::TWO_OPER && (!op1 || !op2))
		)
	{ cout << "[" << my_asm->getLineNum() << "]" << "Tip instr se ne slaze sa brojem operanada" << endl; exit(13); }
	//dst + immed
	if((instr_type == InstrType::TWO_OPER || INSTRUCTIONS[instr_type][instr_index]!="push") && 
		(op1ams==AdressModeSyntax::immed_val || op1ams==AdressModeSyntax::immed_sym)) 
	{ cout << "[" << my_asm->getLineNum() << "]" << "immed iskoriceno kao dst operand" << endl; exit(14); }
	//dva memorijska adresiranja
	int cnt = 0;
	if (op1 && !(op1am == AdressMode::immed || op1am == AdressMode::regdir)) cnt++;
	if (op2 && !(op2am == AdressMode::immed || op2am == AdressMode::regdir)) cnt++;
	if (cnt == 2) 
	{ cout << "[" << my_asm->getLineNum() << "]" << "Dva memorijska adresiranja" << endl; exit(13); exit(15); }
}

void InstrHandler::setSize()
{
	if (op1) {
		if (op1am == AdressMode::regdir || op1am == AdressMode::regind) op1_size = 1;
		if ((op1am == AdressMode::immed && byte_operands) || op1am == AdressMode::regindpom8) op1_size = 2;
		if ((op1am == AdressMode::immed && !byte_operands) || op1am == AdressMode::regindpom16 || op1am == AdressMode::memdir) op1_size = 3;
	}
	if (op2) {
		if ( op2am == AdressMode::regdir || op2am == AdressMode::regind) op2_size = 1;
		if ((op2am == AdressMode::immed && byte_operands) || op2am == AdressMode::regindpom8) op2_size = 2;
		if ((op2am == AdressMode::immed && !byte_operands) || op2am == AdressMode::regindpom16 || op2am == AdressMode::memdir) op2_size = 3;
	}
}

void InstrHandler::setCode()
{
	//kodiranje prvog bajta
	unsigned byte1 = INSTRC_CODES[instr_type][instr_index] << 3;
	unsigned s = 0;
	if (!byte_operands) s = 4;
	byte1 |= s;
	
	my_asm->formatter.write(byte1, Formatter::BYTE);
	
	//kodiranje prvog operanda
	if (op1) codeOp(*op1, op1am, op1ams);
	
	//kodiranje drugog operanda
	if (op2) codeOp(*op2, op2am, op2ams);
	
}

void InstrHandler::codeOp(string& op, InstrHandler::AdressMode& opam, InstrHandler::AdressModeSyntax& opams) 
{
	stringstream opstream(op);
	//najvisa 3b odredjuju nacin adresiranja
	unsigned op_byte_1 = (unsigned)opam << 5;
	//vrednost data u instrukciji
	string val_string;
	//ime simbola dato u instrukciji
	string symbol_name;
	//adresiranje registra
	string reg = ""; 
	//sirina za val (1 ili 2B)
	Formatter::Size w = byte_operands ? Formatter::Size::BYTE : Formatter::Size::WORD;
	//za preskakanje znakova
	char skip;
	int16_t add_op2_size;
	switch (opams)
	{
	case AdressModeSyntax::immed_val:
		//format: <val>
		opstream >> val_string;
		
		my_asm->formatter.write(op_byte_1, Formatter::BYTE);
		my_asm->formatter.val_bits(val_string, w);
		break;
	
	case AdressModeSyntax::immed_sym:
		//format: &<symbol_name>
		opstream >> skip >> symbol_name;
		
		my_asm->formatter.write(op_byte_1, Formatter::BYTE);
		my_asm->formatter.sym_bits(symbol_name, RelocTable::RelocType::R_16);
		break;
	
	case AdressModeSyntax::reg_dir:
		//format: r<num>hl
		opstream >> reg;
		
		my_asm->formatter.reg_bits(op_byte_1, reg);
		break;
	
	case AdressModeSyntax::reg_ind:
		//format: [r<num>]
		opstream >> skip >> reg;
		reg.pop_back(); //uklanjanje ]
		
		my_asm->formatter.reg_bits(op_byte_1, reg);
		break;
	
	case AdressModeSyntax::regind_pom_val:
		//format: r<num>[<val>]
		getline(opstream, reg, '['); //valjda je pokupljena i zagrada
		opstream >> val_string;
		val_string.pop_back();
		
		my_asm->formatter.reg_bits(op_byte_1, reg);
		my_asm->formatter.val_bits(val_string, w);
		break;
	
	case AdressModeSyntax::regind_pom_sym:
		//format: r<num>[<symbol_name>]
		getline(opstream, reg, '[');
		opstream >> symbol_name;
		symbol_name.pop_back(); //da se ukloni ]
		
		my_asm->formatter.reg_bits(op_byte_1, reg);
		my_asm->formatter.sym_bits(symbol_name, RelocTable::RelocType::R_16);
		break;
	
	case AdressModeSyntax::pc_rel_sym:
		//format: $<symbol_name>
		//bitovi za korisceni registar - pc == r7
		op_byte_1 |= 7 << 1;
		opstream >> skip >> symbol_name;
		
		my_asm->formatter.reg_bits(op_byte_1, reg);
		add_op2_size = (op2 && op == *op2) ? op2_size : 0;
		my_asm->formatter.sym_bits(symbol_name, RelocTable::RelocType::R_PC16, add_op2_size);
		break;
	
	case AdressModeSyntax::mem_sym:
		//format: <symbol_name>
		opstream >> symbol_name;
		
		my_asm->formatter.write(op_byte_1, Formatter::BYTE);
		my_asm->formatter.sym_bits(symbol_name, RelocTable::RelocType::R_16);
		break;
	
	case AdressModeSyntax::mem_val:
		//format: *<val>
		opstream >> skip >> val_string;
		
		my_asm->formatter.write(op_byte_1, Formatter::BYTE);
		my_asm->formatter.val_bits(val_string, w);
		break;
	}
}

void InstrHandler::izvestaj() {
	cout << setw(10);
	cout << "Instr: " << getName() << endl;
	if (op1) cout << "oper1: " << *op1 << endl << "op1am: " <<  (int)op1am << endl << "o1ams: " << (int)op1ams << endl;
	if (op2) cout << "oper2: " << *op2 << endl << "op2am: " <<  (int)op2am << endl << "o2ams: " << (int)op2ams << endl;
	//cout << "Kod:" << *coded_instr << endl;
	cout << "-------------------------" << endl;
}
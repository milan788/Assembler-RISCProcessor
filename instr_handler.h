#ifndef INSTR_HANDLER
#define INSTR_HANDLER
#include <iostream>

class Assembler;

class InstrHandler
{
public:
	enum InstrType { NOO_OPER, ONE_OPER, TWO_OPER, UNKNOWN_INSTR_TYPE };
	enum AdressMode { immed, regdir, regind, regindpom8, regindpom16, memdir, UNKNOWN_ADRESS_MODE };
	enum AdressModeSyntax { immed_val, immed_sym, reg_dir, reg_ind, regind_pom_val, regind_pom_sym, pc_rel_sym, mem_sym, mem_val, UNKNOWN_ADRESS_MODE_SYNTAX};

	static const std::string PATTERNS[];
	static const int PATTERNS_SIZE;
	
	//Matrica
	static const std::string INSTRUCTIONS[3][13];
	static const unsigned	 INSTRC_CODES[3][13];
	static const int		 COLS;
	static const int		 ROWS;
	
	//Konstr i destr
	InstrHandler(Assembler* const a);
	~InstrHandler();

	//Javne fje
	void pass(std::string* instr);
	
	InstrType			getType()  { return instr_type; }
	int					getIndex() { return instr_index; }
	const std::string&	getName() { return INSTRUCTIONS[instr_type][instr_index]; }
	int 				getSize()  { return instr_size; }
	std::string*		getOp1()   { return op1; }
	std::string*		getOp2()   { return op2; }
	std::string*		getCodedInstr() { return coded_instr; }
private:
	Assembler* const my_asm = nullptr;

	std::string* op1 = nullptr;
	std::string* op2 = nullptr;
	bool		 byte_operands;
	AdressMode	 op1am;
	AdressMode   op2am;
	AdressModeSyntax op1ams;
	AdressModeSyntax op2ams;
	std::int16_t op1_size = 0;
	std::int16_t op2_size = 0;
	InstrType	instr_type;
	int			instr_index;
	int			instr_size;
	
	std::string* coded_instr = nullptr;
	std::stringstream* code_stream = nullptr;

	//privatne fje
	void set(std::string* instr);
	void reset();

	void setTypeAndIndex(std::string& name);
	void setAdressMode(std::string& op, AdressMode& opam, AdressModeSyntax& opams);
	void validate();
	void setSize();
	void setCode();
	void codeOp(std::string& op, AdressMode& opam, AdressModeSyntax& opams);
	
	void izvestaj();
};

#endif // !INSTR_HANDLER



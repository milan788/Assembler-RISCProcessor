#ifndef LINE_ANALIZER_H
#define LINE_ANALIZER_H
#include <iostream>

class Assembler;
class LineAnalizer {
public:
	LineAnalizer(Assembler* a): my_asm(a) {}
	void set(std::string& l);

	
	bool isLabel() { return is_label; }
	bool isInstr() { return is_instr; }
	bool isDirec() { return is_direc; }

	std::string* getLabel() { return label; }
	std::string* getInstr() { return instr; }
	std::string* getDirec() { return direc; }
	
	//int getSize(); //mozda ce da se prebaci u drugu klasu
	void izvestaj();
private:
	Assembler* my_asm = nullptr;
	int cntr = 0;
	std::string* line = nullptr;
	
	std::string* label = nullptr;
	std::string* instr = nullptr;
	std::string* direc = nullptr;
	
	bool is_label = false;
	bool is_instr = false;
	bool is_direc = false;
	
	bool checkDirec(std::string& word);
	//bool checkInstr(std::string& word);
	bool checkLabel(std::string& word);

	void reset();
};
#endif // !LINE_ANALIZER_H

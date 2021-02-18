#include "line_analizer.h"
#include <sstream>
using namespace std;
void LineAnalizer::set(string& l) { 
	reset();
	line = new string(l);

	//da li je prazna linija
	const char* c = line->c_str();
	while (*c != '\0') {
		if (*c == ' ' || *c == '\t') { c++; continue; }
		else break;
	}
	if (*c == '\0') return;

	stringstream sstream(*line);
	
	string first_word;
	sstream >> first_word;
	
	//da li je direktiva
	if (checkDirec(first_word)) {
		is_direc = true;
		direc = new string(*line);
		return;
	}
	//da li je labela
	if (checkLabel(first_word)) {
		is_label = true;
		first_word.pop_back();
		label = new string(first_word);
	}
	else {
		//zakljucuje se da je instrukcija
		is_instr = true;
		instr = new string(*line);
		return;
	}
	//ako je labela, dolazi se do ovde
	//dohvatiti drugu rec
	string second_word;
	sstream >> second_word;
	//ako ne postoji, return
	if (second_word.length() == 0) return;
	
	//ako postoji dohvati ostatak reda
	string rest_of_the_line;
	sstream.seekg(0, sstream.beg);
	sstream >> first_word; //samo da bi se preskocilo
	getline(sstream, rest_of_the_line);

	if (checkDirec(second_word)) {
		is_direc = true;
		direc = new string(rest_of_the_line);
		return;
	}
	else {
		is_instr = true;
		instr = new string(rest_of_the_line);
		return;
	}
}

bool LineAnalizer::checkDirec(string& word) {
	return word[0] == '.' ? true : false;
}

bool LineAnalizer::checkLabel(string& word)
{
	return word[word.length() - 1] == ':' ? true : false;
}

void LineAnalizer::izvestaj() {
	cout << "LINIJA " << cntr << ":" << endl;
	if (is_direc) cout << "\tDirektiva: " << *direc << endl;
	if (is_instr) cout << "\tInstrukcija: " << *instr << endl;
	if (is_label) cout << "\tLabela: " << *label << endl;
}

void LineAnalizer::reset() {
	++cntr;
	if ( line != nullptr) { delete line;  line = nullptr; }
	if (direc != nullptr) { delete direc; direc = nullptr; }
	if (instr != nullptr) { delete instr; instr = nullptr; }
	if (label != nullptr) { delete label; label = nullptr; }

	is_direc = false;
	is_instr = false;
	is_label = false;

}


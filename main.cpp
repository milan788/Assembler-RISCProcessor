#include <fstream>
#include <iostream>
#include "assembler.h"

using namespace std;

int main(int argc, char** argv) {
	//preusmeravanje cout
	ofstream out("cout.txt");
	streambuf* coutbuf = std::cout.rdbuf(); //save old buf
	cout.rdbuf(out.rdbuf()); //redirect std::cout to out.txt

	fstream sfile;
	cout << argv[1] << endl;
	sfile.open(argv[1], ios::in);
	if (!sfile.is_open()) { cout << "Greska prilikom otvaranja" << endl; }

	fstream ofile;
	ofile.open(argv[2], ios::out);
	if (!sfile.is_open()) { cout << "Greska prilikom otvaranja" << endl; }

	Assembler assembler(sfile, ofile);
	assembler.start();

	//resetovanje cout
	cout.rdbuf(coutbuf);

	cout << "Completed" << endl;
	sfile.close();
	ofile.close();
}
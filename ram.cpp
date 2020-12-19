#include "ram.h"
#include <iostream>
#include <fstream>

byte SNES_RAM::read8(twobyte i) {
	if(i >= 0x0000 && i <= 0x07FF) {
		return data[i];
	}
	
	return 0x00;
}

void SNES_RAM::readFile(std::string filename) {
	std::ifstream f (filename);
	char c;
	unsigned int i = 0;
	while(f.get(c)) {
		data[i++] = c;
		std::cout << c;
	}
	std::cout << std::endl;
}
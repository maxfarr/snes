#include <stdlib.h>
#include <stdio.h>
#include <iostream>

bool getBit(const short int &value, const char k) {
	return (value >> k) & 1;
}

bool getBit(const char &value, const char k) {
	return (value >> k) & 1;
}

class SNES_CPU {
public:
	
private:
	// accumulator
	short int C;
	char* B = (char*)&C;
	char* A = B + 1;
	
	// data bank
	char DBR;
	
	// direct
	short int D;
	char* DH = (char*)&D;
	char* DL = DH + 1;
	
	// program bank
	char K;
	
	// program counter
	short int PC;
	char* PCH = (char*)&PC;
	char* PCL = PCH + 1;
	
	// stack pointer
	short int S;
	char* SH = (char*)&S;
	char* SL = SH + 1;
	
	// X
	short int X;
	char* XH = (char*)&X;
	char* XL = XH + 1;
	
	// Y
	short int Y;
	char* YH = (char*)&Y;
	char* YL = YH + 1;
	
	// flags
	bool carry;
	bool zero;
	bool disable;
	bool decimal;
	bool index_size;
	bool accum_size;
	bool overflow;
	bool negative;
};

int main () {
	std::cout << "size of short int: " << sizeof(SNES_CPU) << std::endl;
	return 0;
}

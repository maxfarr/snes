#include "ram.h"

byte SNES_RAM::read8(twobyte i) {
	if(i >= 0x0000 && i <= 0x07FF) {
		return data[i];
	}
	
	return 0x00;
}
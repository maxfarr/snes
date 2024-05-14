#include <stdint.h>
#define HEX_BYTE_PRINT(x)   std::setw(2) << std::setfill('0') << (unsigned int)(0xFF & x)
#define bind_fn(f)			(std::bind(&SNES_CPU::f, this))
#define getBit(value, k)	(((value) >> k) & 1)
#define SNES_RAM_SIZE       1024 * 64 * 256
#define SNES_ARAM_SIZE      1024 * 64
#define DEBUG
#define DEBUG_MEMORY
//#define DEBUG_ROM
//#define FORCE_RESET_TO_8000
typedef uint32_t threebyte;
typedef uint16_t twobyte;
typedef uint8_t byte;
typedef int16_t signedtwobyte;
typedef int8_t signedbyte;
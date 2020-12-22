#include <stdint.h>
#define bind_fn(f)			(std::bind(&SNES_CPU::f, this))
#define getBit(value, k)	((value >> k) & 1)
typedef uint32_t threebyte;
typedef uint16_t twobyte;
typedef uint8_t byte;
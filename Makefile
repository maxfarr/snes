build: snes.cpp cpu.cpp ram.cpp
	g++ -Wall snes.cpp cpu.cpp ram.cpp -o snes

debug: snes.cpp cpu.cpp ram.cpp
	g++ -g -Wall snes.cpp cpu.cpp ram.cpp -o snes

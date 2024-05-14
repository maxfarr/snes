build: snes.cpp cpu.cpp ram.cpp apu.cpp aram.cpp dsp.cpp
	g++ -Wall snes.cpp cpu.cpp ram.cpp apu.cpp aram.cpp dsp.cpp -o snes

debug: snes.cpp cpu.cpp ram.cpp apu.cpp aram.cpp dsp.cpp
	g++ -g -Wall snes.cpp cpu.cpp ram.cpp apu.cpp aram.cpp dsp.cpp -o snes

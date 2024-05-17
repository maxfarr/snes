build: snes.cpp cpu.cpp ram.cpp apu.cpp aram.cpp dsp.cpp spc700.cpp
	g++ -Wall snes.cpp cpu.cpp ram.cpp apu.cpp aram.cpp dsp.cpp spc700.cpp -o snes

debug: snes.cpp cpu.cpp ram.cpp apu.cpp aram.cpp dsp.cpp spc700.cpp
	g++ -g -Wall snes.cpp cpu.cpp ram.cpp apu.cpp aram.cpp dsp.cpp spc700.cpp -o snes

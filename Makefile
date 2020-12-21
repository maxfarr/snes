build: cpu.cpp ram.cpp
	g++ -Wall cpu.cpp ram.cpp -o cpu

debug: cpu.cpp ram.cpp
	g++ -g -Wall cpu.cpp ram.cpp -o cpu

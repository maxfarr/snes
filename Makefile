CXX = g++
CXXFLAGS = -Wall

# Trace output for the emulator build: DEBUG dumps registers after every
# instruction, DEBUG_MEMORY logs every memory access. Clear with `make DEBUG_FLAGS=`.
DEBUG_FLAGS = -DDEBUG -DDEBUG_MEMORY

# Everything except the entry points (snes.cpp, cputest.cpp)
CORE_SRCS = cpu.cpp ram.cpp cpu_apu_io.cpp apu.cpp aram.cpp dsp.cpp spc700.cpp

SNES_SRCS = snes.cpp $(CORE_SRCS)
CPUTEST_SRCS = cputest.cpp flat_memory.cpp $(CORE_SRCS)

.PHONY: build debug cputest

build: $(SNES_SRCS)
	$(CXX) $(CXXFLAGS) $(DEBUG_FLAGS) $(SNES_SRCS) -o snes

debug: $(SNES_SRCS)
	$(CXX) -g $(CXXFLAGS) $(DEBUG_FLAGS) $(SNES_SRCS) -o snes

# SingleStepTests harness: optimized, no trace output
cputest: $(CPUTEST_SRCS)
	$(CXX) -O2 $(CXXFLAGS) -Ideps/json $(CPUTEST_SRCS) -o cputest

CC = g++
CFLAGS = -O0 -g -std=c++20
LIBS = -lxxhash -lzstd -lpthread

# all: disasOV1 disasOV2 emulate emulatesdl extractplanets1 extractplanetsdata1 extractdata1 extractinstance1 extractinstance2 extractvessels1 emulatecomm1
all: emulatesdl


emulatesdl: src/emul/emul.cpp src/emul/call.cpp src/cpu/cpu.cpp src/cpu/8086emu.cpp src/emul/findword.cpp src/emul/callstack.c src/disasOV/global.c src/emul/graphics.cpp src/emul/fract.c
	$(CXX) $(CFLAGS) -DSTARFLT1 -DSDL src/emul/emul.cpp src/emul/call.cpp src/emul/findword.cpp src/emul/callstack.c src/cpu/cpu.cpp src/cpu/8086emu.cpp src/disasOV/global.c src/emul/graphics.cpp src/emul/fract.c -o emulatesdl -lSDL2 $(LIBS)

.PHONY: clean all

clean:
	rm -f *.o
	rm -f emulatesdl

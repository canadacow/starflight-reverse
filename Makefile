CC = g++
CFLAGS = -O2 -g -std=c++20
LIBS = -lxxhash -lzstd -lpthread

HEADERS = src/emul/call.h \
          src/cpu/cpu.h \
          src/emul/findword.h \
          src/emul/callstack.h \
          src/disasOV/global.h \
          src/emul/graphics.h \
          src/emul/fract.h \
          src/disasmX86/debugger.h \
		  src/util/lodepng.h 

SOURCES = src/emul/emul.cpp \
          src/emul/call.cpp \
          src/cpu/cpu.cpp \
          src/cpu/8086emu.cpp \
          src/emul/findword.cpp \
          src/emul/callstack.c \
          src/disasOV/global.c \
          src/emul/graphics.cpp \
          src/emul/fract.c \
          src/disasmX86/debugger.c \
		  src/util/lodepng.cpp

# all: disasOV1 disasOV2 emulate emulatesdl extractplanets1 extractplanetsdata1 extractdata1 extractinstance1 extractinstance2 extractvessels1 emulatecomm1
all: emulatesdl

emulatesdl: $(SOURCES)  $(HEADERS)
	$(CXX) $(CFLAGS) -DSTARFLT1 -DSDL $(SOURCES) -o emulatesdl -lSDL2 $(LIBS)

.PHONY: clean all

clean:
	rm -f *.o
	rm -f emulatesdl

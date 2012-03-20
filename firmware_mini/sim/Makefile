UNAME := $(shell uname)


FLAGS=--std=gnu99 -Wall  -funsigned-char -Wundef -Wsign-compare -Wunreachable-code -pedantic  -Wstrict-prototypes
LDFLAGS=
BINEXT=

ifeq ($(UNAME), Darwin)
	FLAGS   += -I/Library/Frameworks/SDL.framework/Headers
	LDFLAGS +=   -framework SDL -framework Cocoa SDLmain.m
endif

ifeq ($(UNAME), Linux)
	LDFLAGS +=  -lSDL
endif

ifeq ($(UNAME), MINGW32_NT-6.1)
	FLAGS   += -I/MinGW/msys/1.0/include/ 
	LDFLAGS += -L/MinGW/msys/1.0/lib -lmingw32 -lSDLmain -lSDL
	BINEXT = .exe
endif

SOURCES=$(wildcard ../apps/*.c)
BINARIES=$(SOURCES:../apps/%.c=%$(BINEXT))

LIB_SOURCES=$(wildcard ../libs/*.c)
LIB_HEADERS=$(wildcard ../libs/*.h)



all: $(BINARIES)

clean:
	rm -f ${BINARIES}

%$(BINEXT): ../apps/%.c main.c main.h Makefile $(LIB_HEADERS) $(LIB_SOURCES)
	gcc $(FLAGS) main.c $(LDFLAGS) $< -o $@ -I. $(LIB_SOURCES) -I../libs


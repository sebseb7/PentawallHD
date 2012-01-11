UNAME := $(shell uname)

SOURCES=$(wildcard ../apps/*.c)
BINARIES=$(SOURCES:../apps/%.c=%)

LIB_SOURCES=$(wildcard ../libs/*.c)
LIB_HEADERS=$(wildcard ../libs/*.h)

FLAGS=--std=gnu99 -Wall

ifeq ($(UNAME), Darwin)
	FLAGS +=  -I/Library/Frameworks/SDL.framework/Headers SDLmain.m -framework SDL -framework Cocoa
endif

ifeq ($(UNAME), Linux)
	FLAGS +=  -lSDL
endif

all: $(BINARIES)

clean:
	rm -f ${BINARIES}

%: ../apps/%.c main.c main.h Makefile $(LIB_HEADERS) $(LIB_SOURCES)
	gcc $(FLAGS) main.c $< -o $@ -I. $(LIB_SOURCES) -I../libs


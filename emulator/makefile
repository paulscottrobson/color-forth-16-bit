SOURCES = framework/main.cpp framework/gfx.cpp framework/debugger.cpp sys_debug_vm.cpp hardware.cpp sys_processor.cpp 
APPNAME = cf16

CC = g++

all: $(APPNAME)

clean:
	rm -f $(APPNAME) *.o

.PHONY: all clean

SDL_CFLAGS := $(shell sdl2-config --cflags)
SDL_LDFLAGS := $(shell sdl2-config --libs)

CFLAGS := $(SDL_CFLAGS) -O2 -Wall -DINCLUDE_DEBUGGING_SUPPORT -I. -I./framework
LDFLAGS := $(SDL_LDFLAGS) -lSDL2_image

$(APPNAME): $(SOURCES)
	$(CC) $(SOURCES) $(CFLAGS) $(LDFLAGS) -o $@




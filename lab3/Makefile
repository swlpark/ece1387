CC = g++
CFLAGS = -g -O0 -c -Wall -std=c++11
LDFLAGS = -g -Wall 
EXE = a3

#HDR = src/graphics.h src/easygl_constants.h

CPP_FILES:= $(wildcard src/*.cpp)
OBJ_FILES:= $(patsubst src/%.cpp,bin/%.o,$(CPP_FILES))

# Need to tell the linker to link to the libraries.
GRAPHICS_LIBS = -lX11 -lXft -lfontconfig
CFLAGS += $(shell pkg-config --cflags freetype2) # evaluates to the correct include flags for the freetype headers

default: all

all: bin $(OBJ_FILES) $(EXE)

bin:
	mkdir -p bin

bin/%.o : src/%.cpp
	$(CC) $(CFLAGS) -o $@ $<

$(EXE) : $(OBJ_FILES)
	$(CC) $(LDFLAGS) $^ $(GRAPHICS_LIBS) -o $(EXE)

clean:
	rm -r bin
	rm -f $(EXE)

CC=g++
CFLAGS=-c -Wall -O2
LDFLAGS=-c -Wall -O2

CPP_FILES:= $(wildcard src/*.cpp)
OBJ_FILES:= $(patsubst src/%.cpp,bin/%.o,$(CPP_FILES))


default: all

all: bin $(OBJ_FILES) maze_router

bin:
	mkdir bin

maze_router: $(OBJ)

obj/%.o : src/%.cpp 
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -r bin

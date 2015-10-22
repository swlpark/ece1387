CC = g++
#CFLAGS = -g -O0 -c -Wall -std=c++11 
CFLAGS = -g -O2 -c -Wall -std=c++11 
INC=-Iref/SuiteSparse/AMD/Include -Iref/SuiteSparse/UMFPACK/Include -Iref/SuiteSparse/SuiteSparse_config


LDFLAGS = -g -Wall 
EXE = ap

#HDR = src/graphics.h src/easygl_constants.h

CPP_FILES:= $(wildcard src/*.cpp)
OBJ_FILES:= $(patsubst src/%.cpp,bin/%.o,$(CPP_FILES))

# Need to tell the linker to link to the libraries.
GRAPHICS_LIBS = -lX11 -lXft -lfontconfig
UMFPACK_LIBS = -lm -lrt -llapack
UMFPACK = ref/SuiteSparse/UMFPACK/Lib/libumfpack.a ref/SuiteSparse/AMD/Lib/libamd.a ref/SuiteSparse/SuiteSparse_config/libsuitesparseconfig.a ref/SuiteSparse/CHOLMOD/Lib/libcholmod.a ref/SuiteSparse/COLAMD/Lib/libcolamd.a ref/SuiteSparse/CCOLAMD/Lib/libccolamd.a ref/SuiteSparse/CAMD/Lib/libcamd.a

CFLAGS += $(shell pkg-config --cflags freetype2) # evaluates to the correct include flags for the freetype headers

default: all

all: bin umfpack $(OBJ_FILES) $(EXE)

bin:
	mkdir -p bin

umfpack: ref/SuiteSparse/UMFPACK/Makefile ref/SuiteSparse/Makefile
ifeq (,$(wildcard  ./ref/SuiteSparse/UMFPACK/Lib/libumfpack.a))
	$(MAKE) -C ref/SuiteSparse/ library
endif
ifeq (,$(wildcard  ./ref/SuiteSparse/AMD/Lib/libamd.a))
	$(MAKE) -C ref/SuiteSparse/ library
endif

bin/%.o : src/%.cpp
	$(CC) $(CFLAGS) $(INC) -o $@ $<

$(EXE) : $(OBJ_FILES)
	$(CC) $(LDFLAGS) $^ $(GRAPHICS_LIBS) $(UMFPACK_LIBS) $(UMFPACK) -o $(EXE)

clean:
	rm -r bin
	rm -f router

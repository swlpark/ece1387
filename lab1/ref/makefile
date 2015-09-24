# Pick a platform below.  X11 for Linux/Mac/Unix, and WIN32 for windows.
# In either X11 or WIN32, Postscript is also simultaneously available.
# You can also pick NO_GRAPHICS, which will allow your code to compile without
# change on any platform, but no graphics will display (all calls are stubbed
# out).
 
# Compiling to support X11 requires the X11 development libraries, Xft and fontconfig.
#
# Ubuntu:
# `sudo apt-get install libx11-dev libxft-dev libfontconfig1-dev` to install.
#
# Other Linux:
# If you are using some other Linux distribution, search the internet for how to install those libraries.
# They are almost certainly available in your package manager.
#
# Macs:
# Please note that a Mac can run the X11 graphics if the X11 compatibility layer, XQuartz, is installed.
# And homebrew, a package manager, is probably your best bet for installing the development libraries.
# Finally note that Macs will probably also need pkg-config, unless you would like to specify the path
# to the freetype headers manually.

PLATFORM = X11
#PLATFORM = WIN32
#PLATFORM = NO_GRAPHICS

HDR = graphics.h easygl_constants.h
SRC = graphics.cpp example.cpp
EXE = example
BACKUP_FILENAME=`date "+backup-%Y%m%d-%H%M.zip"`
FLAGS = -g -Wall -Wextra -pedantic -D$(PLATFORM) -std=c++11

# Need to tell the linker to link to the libraries.
# WIN32 links to all the win32 API libraries every time (no need for flags)
ifeq ($(PLATFORM),X11)
   GRAPHICS_LIBS = -lX11 -lXft -lfontconfig
   FLAGS += $(shell pkg-config --cflags freetype2) # evaluates to the correct include flags for the freetype headers
endif

$(EXE): graphics.o example.o
	g++ $(FLAGS) $^ $(GRAPHICS_LIBS) -o $(EXE)

# A generic make rule for creating a .o file from an identically named (sans extension) .cpp file
# $< evaluates to the name of the _first_ dependency (the .cpp file in this case)
# $@ evaluates to the name of the target (the .o file in this case)
%.o: %.cpp $(HDR) 
	g++ -c $(FLAGS) $< -o $@

# graphics.o: graphics.cpp $(HDR)
# 	g++ -c $(FLAGS) graphics.cpp

# example.o: example.cpp $(HDR)
# 	g++ -c $(FLAGS) example.cpp

backup:
# Back up the source, makefile and Visual Studio project & solution. 
	echo Backing up your files into ${BACKUP_FILENAME}
	zip ${BACKUP_FILENAME} $(SRC) $(HDR) makefile easygl.sln easygl.vcxproj example_screenshot.png

clean:
	rm $(EXE) *.o

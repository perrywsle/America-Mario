CC := gcc
CXX := g++
INCLDIR := GUI/externals/cimgui
SRCDIR := src
SDL2_INCLDIR := /usr/include/SDL2
IMGUI_INCLDIR := GUI/externals/cimgui/imgui
IMGUI_IMPL_INCLDIR := GUI/imgui/impl

UNAME_S := $(shell uname -s)

# Library definitions based on platform
ifeq ($(UNAME_S), Linux) # LINUX
	CIMGUI_LIB := GUI/cimgui.so
	LFLAGS := -lSDL2 -lm -lSDL2_image -lSDL2_ttf $(CIMGUI_LIB) -lstdc++
endif

ifeq ($(UNAME_S), Darwin) # APPLE
	CIMGUI_LIB := cimgui.dylib
	LFLAGS := -lSDL2 -lm -lSDL2_image -lSDL2_ttf $(CIMGUI_LIB) -lc++
endif

ifeq ($(UNAME_S), Windows_NT) # WINDOWS
	CIMGUI_LIB := cimgui.dll
	LFLAGS := -lSDL2 -lm -lSDL2_image -lSDL2_ttf $(CIMGUI_LIB) -lc++
endif

CFLAGS := -Wall -std=c99 -I$(INCLDIR) -I$(IMGUI_INCLDIR) -I$(IMGUI_IMPL_INCLDIR) -I$(SDL2_INCLDIR) -g -DIMGUI_IMPL_API=""
CXXFLAGS := -Wall -std=c++11 -I$(INCLDIR) -I$(IMGUI_INCLDIR) -I$(IMGUI_IMPL_INCLDIR) -I$(SDL2_INCLDIR) -g -DIMGUI_IMPL_API=""

# List of object files to compile
OBJS = \
	$(SRCDIR)/main.o \
	$(SRCDIR)/render.o \
	$(SRCDIR)/shooter.o \
	$(SRCDIR)/init.o \
	$(IMGUI_IMPL_INCLDIR)/imgui_impl_sdl.o

.PHONY: all clean

all: main

main: $(OBJS)
	$(CC) $(OBJS) -o main $(LFLAGS)

# Compile imgui_impl_sdl.o from imgui_impl_sdl.cpp
$(IMGUI_IMPL_INCLDIR)/imgui_impl_sdl.o: $(IMGUI_IMPL_INCLDIR)/imgui_impl_sdl.cpp $(IMGUI_IMPL_INCLDIR)/imgui_impl_sdl.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile main.o from src/main.c
$(SRCDIR)/main.o: $(SRCDIR)/main.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compile render.o from src/render.c
$(SRCDIR)/render.o: $(SRCDIR)/render.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compile shooter.o from src/shooter.c
$(SRCDIR)/shooter.o: $(SRCDIR)/shooter.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compile init.o from src/init.c
$(SRCDIR)/init.o: $(SRCDIR)/init.c
	$(CC) $(CFLAGS) -c $< -o $@

cimgui:
	make -C externals/cimgui
	cp -p externals/cimgui/$(CIMGUI_LIB) ./

clean:
	rm -f $(SRCDIR)/*.o
	rm -f $(IMGUI_IMPL_INCLDIR)/*.o
	rm -f ./*.o
	rm -f main
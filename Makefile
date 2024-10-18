CC := gcc
CXX := g++
SRCDIR := src
SDL2_INCLDIR := /usr/include/SDL2

UNAME_S := $(shell uname -s)

# Library definitions based on platform
ifeq ($(UNAME_S), Linux) # LINUX
	LFLAGS := -lSDL2 -lm -lSDL2_image -lSDL2_ttf -lstdc++
endif

ifeq ($(UNAME_S), Darwin) # APPLE
	LFLAGS := -lSDL2 -lm -lSDL2_image -lSDL2_ttf -lc++
endif

ifeq ($(UNAME_S), Windows_NT) # WINDOWS
	LFLAGS := -lSDL2 -lm -lSDL2_image -lSDL2_ttf -lc++
endif

CFLAGS := -Wall -std=c99 -I$(SDL2_INCLDIR) -g
CXXFLAGS := -Wall -std=c++11 -I$(SDL2_INCLDIR) -g

# List of object files to compile
OBJS = \
	$(SRCDIR)/main.o \
	$(SRCDIR)/render.o \
	$(SRCDIR)/shooter.o \
	$(SRCDIR)/init.o

.PHONY: all clean

all: main

main: $(OBJS)
	$(CC) $(OBJS) -o main $(LFLAGS)

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

clean:
	rm -f $(SRCDIR)/*.o
	rm -f main
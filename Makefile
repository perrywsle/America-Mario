# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -g `sdl2-config --cflags`

# Libraries
LIBS = `sdl2-config --libs` -lSDL2_image -lSDL2_ttf -lm

# Object files
OBJS = main.o init.o shooter.o render.o

# Executable name
TARGET = game

# Target to build everything
all: $(TARGET)

# Compile each source file to object file (explicit commands for each file)
main.o: main.c init.h shooter.h render.h
	$(CC) $(CFLAGS) -c main.c -o main.o

init.o: init.c init.h shooter.h render.h
	$(CC) $(CFLAGS) -c init.c -o init.o

shooter.o: shooter.c shooter.h init.h render.h
	$(CC) $(CFLAGS) -c shooter.c -o shooter.o

render.o: render.c render.h init.h shooter.h
	$(CC) $(CFLAGS) -c render.c -o render.o

# Link all object files to create the executable
$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(LIBS)

# Clean up object files and executable
clean:
	rm -f $(OBJS) $(TARGET)

# Phony targets (force rebuilding)
.PHONY: clean all

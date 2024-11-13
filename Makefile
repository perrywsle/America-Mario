CC := gcc
INCLDIR := externals/cimgui
SRCDIR := src
SDL2_INCLDIR := /usr/local/include/SDL2
GL3W_INCLDIR := gl3w/include
GLEW_INCLDIR := /usr/local/include/GL
IMGUI_INCLDIR := externals/cimgui/imgui
IMGUI_IMPL_INCLDIR := imgui/impl
IMGUI_IMPL_DIR := imgui/impl
GL3W_DIR := gl3w
OUT_GL3W := main-gl3w
OUT_GLEW := main-glew

UNAME_S := $(shell uname -s)

# different file extension on different platform
# for cimgui library after building
ifeq ($(UNAME_S), Linux) #LINUX
	CIMGUI_LIB := cimgui.so
endif

ifeq ($(UNAME_S), Darwin) #APPLE
	CIMGUI_LIB := cimgui.dylib
endif

ifeq ($(UNAME_S), Windows_NT) #WINDOWS
	CIMGUI_LIB := cimgui.dll
endif

CFLAGS := -Wall -std=c99 -Igl3w/include -I/opt/X11/include -I$(IMGUI_INCLDIR) -I$(IMGUI_IMPL_INCLDIR) -I$(INCLDIR) -I$(SDL2_INCLDIR) -I$(SRCDIR) -g -DIMGUI_IMPL_OPENGL_LOADER_GL3W -DIMGUI_IMPL_API=""
LFLAGS := -lSDL2 -lGL -lGLU -lm -lcjson -lSDL2_image $(CIMGUI_LIB) -lSDL2_ttf -lstdc++ -Wl,-rpath,.

SDL_IMPL_CFLAGS = -I$(INCLDIR) -I$(IMGUI_INCLDIR) -I$(IMGUI_IMPL_INCLDIR) -I/opt/X11/include -I$(SDL2_INCLDIR) -I$(GLEW_INCLDIR) -DIMGUI_IMPL_API="extern \"C\""
OPENGL3_IMPL_CFLAGS = -I$(INCLDIR) -Igl3w/include -I$(IMGUI_INCLDIR) -I$(IMGUI_IMPL_INCLDIR) -DIMGUI_IMPL_API="extern \"C\"" 

OBJS_GL3W = \
	    cimgui	\
	    imgui_impl_sdl.o \
	    imgui_impl_opengl3.o \
		init.o \
		render.o \
		shooter.o \
	    main.o \
	    main


.PHONY: all gl3w clean

all: $(OBJS_GL3W) $(OBJS_GLEW)

gl3w: $(OBJS_GL3W)

main: main.o gl3w.o imgui_impl_sdl.o imgui_impl_opengl3.o cimgui $(SRCDIR)/init.o $(SRCDIR)/shooter.o $(SRCDIR)/render.o
	gcc $(SRCDIR)/main.o $(SRCDIR)/init.o $(SRCDIR)/shooter.o $(SRCDIR)/render.o $(IMGUI_IMPL_DIR)/imgui_impl_sdl.o $(IMGUI_IMPL_DIR)/imgui_impl_opengl3.o $(GL3W_DIR)/src/gl3w.o -o $(OUT_GL3W) $(LFLAGS)

imgui_impl_sdl.o: $(IMGUI_IMPL_DIR)/imgui_impl_sdl.cpp $(IMGUI_IMPL_DIR)/imgui_impl_sdl.h
	g++ $(SDL_IMPL_CFLAGS) -c $< -o $(IMGUI_IMPL_DIR)/$@

imgui_impl_opengl3.o: $(IMGUI_IMPL_DIR)/imgui_impl_opengl3.cpp $(IMGUI_IMPL_DIR)/imgui_impl_opengl3.h
	g++ $(OPENGL3_IMPL_CFLAGS) -c $< -o $(IMGUI_IMPL_DIR)/$@


gl3w.o: gl3w/src/gl3w.c gl3w/include/GL/gl3w.h gl3w/include/GL/glcorearb.h
	gcc $(CFLAGS) -c $< -o gl3w/src/$@

init.o: $(SRCDIR)/init.c $(SRCDIR)/init.h
	gcc $(CFLAGS) -c $< -o $(SRCDIR)/$@

shooter.o: $(SRCDIR)/shooter.c $(SRCDIR)/shooter.h
	gcc $(CFLAGS) -c $< -o $(SRCDIR)/$@

render.o: $(SRCDIR)/render.c $(SRCDIR)/render.h
	gcc $(CFLAGS) -c $< -o $(SRCDIR)/$@

main.o: $(SRCDIR)/main.c 
	gcc $(CFLAGS) -c $< -o $(SRCDIR)/$@

cimgui:
	make -C externals/cimgui
	cp -p externals/cimgui/$(CIMGUI_LIB) ./

clean:
	rm -f $(SRCDIR)/*.o
	rm -f $(IMGUI_IMPL_DIR)/*.o
	rm -f $(GL3W_DIR)/src/*.o
	rm -f $(OUT_GL3W)
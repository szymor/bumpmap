.PHONY: all clean

TARGET=bumpmap
CFLAGS=$(shell pkg-config --cflags sdl SDL_image) -g
LFLAGS=$(shell pkg-config --libs sdl SDL_image) -lm

all: $(TARGET)

clean:
	-rm -rf $(TARGET)

$(TARGET): main.c
	gcc main.c -o $(TARGET) $(CFLAGS) $(LFLAGS)

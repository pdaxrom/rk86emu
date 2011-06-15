TARGET = radio86rk

all: $(TARGET)

CC = gcc
#CFLAGS = -O3 -fomit-frame-pointer -Wall -g `sdl-config --cflags`
CFLAGS = -Wall -g `sdl-config --cflags`
LDFLAGS = -g `sdl-config --libs`

OBJS = i8080.o perifer.o main.o tui.o

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(OBJS) $(TARGET)

CC = gcc

CFLAGS = -Wall -Wextra -pedantic -std=c11 -I/usr/include/SDL2

LDLIBS = -lSDL2 -lSDL2_gfx -lm

TARGET = connect4

SRCS = $(wildcard *.c)

OBJS = $(SRCS:.c=.o)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDLIBS)

clean:
	rm -f $(OBJS) $(TARGET)

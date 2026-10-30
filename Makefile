CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c11 -O2 -I/usr/include/SDL2
LDLIBS = -lSDL2 -lSDL2_gfx -lm

TARGET = connect4
SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

.PHONY: all clean run debug

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET) --gui --ai 2

debug: CFLAGS = -Wall -Wextra -pedantic -std=c11 -g -fsanitize=address,undefined -I/usr/include/SDL2
debug: clean $(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)

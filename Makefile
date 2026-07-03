CC = gcc

CFLAGS = -Wall -Wextra -pedantic -std=c11

TARGET = connect_four

SRCS = $(wildcard *.c)

OBJS = $(SRCS:.c=.o)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

clean:
	rm -f $(OBJS) $(TARGET)

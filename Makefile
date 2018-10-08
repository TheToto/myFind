CC=gcc
CFLAGS=-Wall -Wextra -pedantic -std=c99

SRCS=$(wildcard src/*.c)
OBJS=$(SRCS:.c=.o)

BIN=myfind

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(BIN)

.PHONY: test
test: $(BIN)
	tests/test.sh ./$(BIN)

clean:
	$(RM) $(OBJS) $(BIN)

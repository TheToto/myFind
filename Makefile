CC=gcc
CFLAGS=-Wall -Wextra -pedantic -std=c99

VPATH=src
OBJS=myfind.o my_string.o commands.o
BIN=myfind

all: $(BIN)

$(BIN): $(OBJS)

.PHONY: test
test: $(BIN)
	tests/test.sh ./$(BIN)

clean:
	$(RM) $(OBJS) $(BIN)

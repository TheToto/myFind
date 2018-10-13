CC=gcc
CFLAGS=-Wall -Wextra -pedantic -std=c99

VPATH=src
OBJS=myfind.o libstring.o expr_action.o libgen.o expr_test.o expr_test_advanced.o parsing.o
BIN=myfind

all: $(BIN)

$(BIN): $(OBJS)

.PHONY: test
test: $(BIN)
	tests/test.sh ./$(BIN)

clean:
	$(RM) $(OBJS) $(BIN)
CC=gcc

CFLAGS=-std=c99 -pedantic -Werror -Wall -Wextra -Wvla -Isrc/

SRCDIR=src
TSTDIR=tests

TARGET=minimake
DEBUG=debug
CHECK=check
CRITERION=criterion

SRCFILES=minimake.c \
		 hash_map.c \
		 parser.c \
		 rule.c \
		 builder.c \
		 expansion.c \
		 pattern.c \
		 helpers.c \

SRC:=$(addprefix $(SRCDIR)/, $(SRCFILES))

TSTSRC=tests/tests.c
MAINSRC=$(addprefix $(SRCDIR)/, main.c)

OBJ=$(SRC:.c=.o)
TSTOBJ:=$(TSTSRC:.c=.o)
MAINOBJ:=$(MAINSRC:.c=.o)

all: $(TARGET)

$(CHECK): LDFLAGS+=-lcriterion
$(CHECK): $(TSTOBJ) $(OBJ) $(TARGET)
	#./tests/testsuite.sh $(shell pwd)/minimake $(shell pwd)/tests/
	./tests/testsuite.sh
	$(CC) $(LDFLAGS) $(TSTOBJ) $(OBJ) -o $(CRITERION)
	./$(CRITERION)

.PHONY: clean

clean:
	$(RM) $(CRITERION) $(CHECK) $(TARGET) $(DEBUG) $(OBJ) $(MAINOBJ) $(TSTOBJ) *.gcda *.gcno *.css *.html

$(TARGET): $(OBJ) $(MAINOBJ)
	$(CC) $^ -o $@

$(DEBUG): CFLAGS+=-fsanitize=address -g -Og
$(DEBUG): LDFLAGS+=-fsanitize=address
$(DEBUG): $(OBJ) $(MAINOBJ)
	$(CC) $(LDFLAGS) $^ -o $@

check-debug:
	./tests/testsuite.sh $(shell pwd)/debug $(shell pwd)/tests/

# TODO COVERAGE
#coverage: LDLIBS+=-lgov
#coverage: LDFLAGS+=--coverage

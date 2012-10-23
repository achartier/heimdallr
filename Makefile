CC?=gcc
FLAGS = -g -ggdb -std=c99 -pedantic -W -Wall -Wextra -Werror
CFLAGS += $(FLAGS) $(shell pkg-config libpci --cflags) $(shell pkg-config json --cflags)
LDFLAGS += $(shell pkg-config libpci --libs) $(shell pkg-config json --libs)
CFILES = main.c json.c pci.c
BIN = heimdallr

.PHONY: depend

all: $(BIN)

$(BIN): $(CFILES:.c=.o)
	$(CC) $(CFLAGS) $(CFILES:.c=.o) -o $(BIN) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $^

clean:
	rm -rf *.o *~ .*.swp *.core *.log *.gch

distclean: clean
	rm -rf $(BIN)

depend: $(CFILES)
	$(CC) $(FLAGS) -MM $^ > .depend

-include .depend

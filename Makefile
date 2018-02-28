.PHONY: all clean

all: runlog

CFLAGS += -Wall -Wextra

runlog: runlog.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f runlog


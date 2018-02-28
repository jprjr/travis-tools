.PHONY: all clean dist

all: runlog

dist: all
	mkdir -p dist/bin dist/src dist/share
	cp -v runlog dist/bin/runlog
	cp -v runlog.c dist/src/runlog.c
	cp -v README.md LICENSE dist/share/
	tar cvJf travis-tools.tar.xz -C dist bin src share

CFLAGS = -Wall -Wextra

runlog: runlog.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

clean:
	rm -f runlog
	rm -rf dist


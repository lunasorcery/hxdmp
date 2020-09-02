all: bin/hxdmp

bin/hxdmp: main.c Makefile
	mkdir -p bin
	${CC} main.c -o bin/hxdmp -O3 -std=c99 -Wall -Wextra

clean:
	rm -rf bin

install: bin/hxdmp
	cp bin/hxdmp /usr/local/bin/hxdmp

.PHONY: all clean install

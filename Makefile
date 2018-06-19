all: hxdmp

hxdmp: main.c Makefile
	${CC} main.c -o hxdmp -O3 -Wall -Wextra -Werror

clean:
	rm -f hxdmp

install: hxdmp
	cp hxdmp /usr/local/bin/hxdmp

all: hxdmp

hxdmp: main.c Makefile
	clang main.c -o hxdmp -O3 -lstdc++

install: hxdmp
	cp hxdmp /usr/local/bin/hxdmp

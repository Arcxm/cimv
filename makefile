CC=gcc

.PHONY: all
all: cimv

cimv:
	${CC} -o cimv cimv.c
CC=gcc

# Link the math library on linux to use the math header
# But do not link it on windows as it makes the drawing way slower (on my machine at least)
ifeq (${OS},Windows_NT)
	ADDITIONAL=
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Linux)
		ADDITIONAL=-lm
	endif
endif

.PHONY: all
all: cimv

cimv:
	${CC} -o cimv cimv.c ${ADDITIONAL}
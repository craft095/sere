objects = Language.o EvalSere.o SereTests.o

CC=g++

LL=g++

CPP_FLAGS= -O0 -g -Wall -Wextra

%.o : %.c
	$(CC) -c $(CPP_FLAGS) $< -o $@

SereTests : $(objects)
	$(LL) $(objects) -o SereTests

#utils.o : defs.h

.PHONY : clean test

clean :
	-rm -f SereTests $(objects)

test : SereTests
	-./SereTests

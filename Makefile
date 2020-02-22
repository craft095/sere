objects = Language.o EvalSere.o SereTests.o

CC=g++

LL=g++

CPP_FLAGS= -O0 -g -Wall -Wextra

%.o : %.cpp
	$(CC) -c $(CPP_FLAGS) $< -o $@

SereTests : $(objects)
	$(LL) $(objects) -o SereTests -lgtest -lpthread

SereTests.cpp : Located.hpp Letter.hpp Language.hpp EvalSere.hpp
Letter.cpp : Letter.hpp
Language.cpp : Located.hpp Language.hpp
EvalSere.cpp : Letter.hpp Language.hpp EvalSere.hpp

.PHONY : clean test

clean :
	-rm -f SereTests $(objects)

test : SereTests
	-./SereTests

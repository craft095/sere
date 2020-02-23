objects = Language.o EvalSere.o SereTests.o Letter.o Z3.o

CC=g++

LL=g++

CPP_FLAGS= -O0 -g -Wall -Wextra

%.o : %.cpp
	$(CC) -c $(CPP_FLAGS) $< -o $@

SereTests : $(objects)
	$(LL) $(objects) -o SereTests -lgtest -lpthread -lz3

SereTests.cpp : Located.hpp Letter.hpp Language.hpp EvalSere.hpp Z3.hpp
Letter.cpp : Letter.hpp
Language.cpp : Located.hpp Language.hpp
EvalSere.cpp : Letter.hpp Language.hpp EvalSere.hpp Z3.hpp
Z3.cpp : Letter.hpp Language.hpp Located.hpp Z3.hpp

.PHONY : clean test

clean :
	-rm -f SereTests $(objects)

test : SereTests
	-./SereTests

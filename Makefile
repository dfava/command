all: command

clean:
	rm -f parser.cpp parser.hpp command tokens.cpp parser.output
	rm -rf command.dSYM

parser.cpp: parser.y
	bison -d -o $@ $^ -v
	
parser.hpp: parser.cpp

tokens.cpp: tokens.l parser.hpp
	lex -o $@ $^

command: parser.cpp codegen.cpp main.cpp tokens.cpp
	g++ -o $@ `llvm-config --libs core jit native --cxxflags --ldflags` *.cpp -I$(LLVM)/include/ -w -L$(LLVM)/lib/ -lz -frtti

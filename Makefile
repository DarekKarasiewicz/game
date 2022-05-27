CXX=g++
CXXSTD=c++2a
CXXFLAGS=\
		 -std=$(CXXSTD) \
		 -g \
		 -Wall \
		 -Werror \
		 -Wfatal-errors \
		 -Iinclude

all: game.elf

watch:
	find . -name '*.cxx' | entr -c make -j

%.elf: build/%.o
	$(CXX) $(CXXFLAGS) -o $@ $^

build/%.o: src/%.cxx
	$(CXX) $(CXXFLAGS) -c -o $@ $^

game.elf: build/map.o

format:
	find . -name "*.cxx" -or -name "*.h" | xargs --verbose -n 1 clang-format -i

clean:
	rm build/*.o 
	rm *.elf

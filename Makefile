CXX=g++
CXXSTD=c++2a
CXXFLAGS=\
		 -std=$(CXXSTD) \
		 -Wall \
		 -Werror \
		 -Wfatal-errors

all: game.elf

watch:
	find . -name '*.cxx' | entr -c make -j

%.elf: %.o
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cxx
	$(CXX) $(CXXFLAGS) -c -o $@ $^

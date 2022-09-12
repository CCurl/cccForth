appname := cccForth

CXX := clang++
CXXFLAGS := -std=c++11 -D_LINUX

srcfiles := $(shell find . -name "*.cpp")
objects  := $(patsubst %.cpp, %.o, $(srcfiles))

all: $(appname)

$(appname): $(objects)
    $(CXX) $(CXXFLAGS) $(LDFLAGS) -o $(appname) $(objects) $(LDLIBS)

depend: .depend

.depend: $(srcfiles)
    rm -f ./.depend
    $(CXX) $(CXXFLAGS) -MM $^>>./.depend;

clean:
    rm -f $(objects)

dist-clean: clean
    rm -f *~ .depend

include .depend

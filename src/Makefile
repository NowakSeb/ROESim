ARCH=$(shell uname -m)

LIBARIES = $(patsubst %,%.a, $(shell ls -d lib* 2>/dev/null))
LIBLIST = $(patsubst lib%.a,-l%, $(LIBARIES))
LIBCLEAN = $(patsubst lib%.a,lib%.a.rm, $(LIBARIES))

INCLUDEFLAGS = $(patsubst %,-I%/include, $(shell ls -d lib* 2>/dev/null))
LIBPATH = $(patsubst %,-L%/../libs, $(shell ls -d lib* 2>/dev/null))

PROGSO= $(subst .cpp,.o, $(shell ls -d prog/*.cpp 2>/dev/null))
PROGSNAME= $(subst .cpp,.prog, $(shell ls -d prog/*.cpp 2>/dev/null))

CXX = g++ -fmessage-length=0 -std=c++14
CXXFLAGS= -g -Wall -MD -O2
ROOTCFLAGS=$(shell root-config --cflags)
LD_FLAGS = -lm -lpthread `root-config --libs --glibs` -lMinuit2 -lfftw3 -lngspice


# OBJ=
# SRCS=
# LIB=

all:
	mkdir -p ../bin/$(ARCH)
	make LIBS -j8
	make PROGS -j8

LIBS: $(LIBARIES)

lib%.a: lib%
	cd $</src;
	$(eval SRCS=$(shell ls $</src/*.cpp))
	make COMPLIB OBJ='$(subst .cpp,.o, $(SRCS))' LIB=$< -j8

COMPLIB: $(OBJ)
	echo $(OBJ)
	ar -rs ../bin/$(ARCH)/$(LIB).a $^

PROGS: $(PROGSO) $(PROGSNAME)

%.prog: %.o
	$(CXX) -o ../bin/$(ARCH)/$(patsubst prog/%.o,%, $<) $< -L../bin/$(ARCH) $(LIBLIST) $(LD_FLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDEFLAGS) $(ROOTCFLAGS) -c $< -o $(subst .cpp,.o, $<)
	
	
clean: $(LIBCLEAN)
	echo $(LIBARIES)
	-rm -rf ../bin/$(ARCH)
	-rm -f prog/*.o prog/*.bak prog/*.d

%.a.rm: %
	cd $</; rm -f src/*.o *.a src/*.bak src/*.d
#	cd ..;

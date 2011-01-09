CXX=g++
CFLAGS=-Wall -fpic -O2 -pg
CXXFLAGS=$(CFLAGS)
LDFLAGS=-shared -fpic  -Wl,-soname,libmystuff.so.1 -lc

OBJ=envelope.o filter.o note_skel.o infile.o programs.o
LIB=result.so

DEPENDFILE = .depend


SRC = $(OBJ:%.o=%.cpp)

all: $(LIB)


$(LIB): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(LIB) $(OBJ) $(LDFLAGS)
	

depend dep: $(SRC)
	$(CC) -MM $(SRC) > $(DEPENDFILE)

-include $(DEPENDFILE)


%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

.PHONY: clean dep depend

clean:
	rm -f $(OBJ) $(LIB)


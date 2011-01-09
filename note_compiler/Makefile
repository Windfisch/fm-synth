CXX=g++
CFLAGS=-Wall -g
CXXFLAGS=$(CFLAGS)
LDFLAGS=-lm `pkg-config --cflags --libs jack`

OBJ=main.o parser.o util.o
BIN=compiler

DEPENDFILE = .depend


SRC = $(OBJ:%.o=%.cpp)

all: $(BIN)


$(BIN): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(BIN) $(OBJ) $(LDFLAGS)
	

depend dep: $(SRC)
	$(CC) -MM $(SRC) > $(DEPENDFILE)

-include $(DEPENDFILE)


%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

.PHONY: clean dep depend

clean:
	rm -f $(OBJ) $(BIN)
	

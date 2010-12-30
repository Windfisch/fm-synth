CXX=g++
CXXFLAGS=-Wall -g
LDFLAGS=-lm `pkg-config --cflags --libs jack`

OBJ=channel.o cli.o defines.o envelope.o filter.o globals.o jack.o load.o main.o note.o parser.o programs.o readwave.o util.o
BIN=synth

DEPENDFILE = .depend


SRC = $(OBJ:%.o=%.cpp)

all: $(BIN)


$(BIN): $(OBJ)
	$(CXX) $(CFLAGS) -o $(BIN) $(OBJ) $(LDFLAGS)
	

depend dep: $(SRC)
	$(CC) -MM $(SRC) > $(DEPENDFILE)

-include $(DEPENDFILE)


%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

.PHONY: clean dep depend

clean:
	rm -f $(OBJ) $(BIN)
	


CC = g++
# --- gnu++11 uses GNU extensions. ---
CFLAGS = --std=c++14
CFLAGS += -Wall -c -O0 -DNDEBUG

LDFLAGS += -lGL -lGLU -lglut -lGLEW

FILENAME = main
BUILDFILE = stlviewer

SOURCE = $(FILENAME).cpp
HEADER = stlcmd.hpp
HEADER += utilities.hpp
HEADER += mat4.hpp
HEADER += basicelement.hpp

OBJ = $(FILENAME).o
BIN = $(BUILDFILE)

.PHONEY: all
all: $(BIN)

$(BIN): $(OBJ)
	$(CC) -o $@ $< $(LDFLAGS) 

$(OBJ): $(SOURCE) $(HEADER)
	$(CC) $(CFLAGS) $(INC) $<

.PHONEY: clean
clean:	
	$(RM) -r -f $(OBJ)	
	$(RM) -r -f $(BUILDFILE)


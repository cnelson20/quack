
FOLDER ?= game

ifeq ($(OS),Windows_NT)
CC = cl65.exe
else
CC = cl65
endif
FLAGS = -m quack.map -tcx16 -Ois --codesize 200 -Cl
PROG = QUACK.PRG
SOURCES = main.c routines.s
HEADERS = main.h routines.h

all: $(PROG)

$(PROG): $(SOURCES) $(HEADERS)
	$(CC) $(FLAGS) -o $(PROG) $(SOURCES) -lib zsound/zsound.lib

copy:
	-mkdir $(FOLDER)/
	cp *.BIN $(FOLDER)/
	cp *.ZCM $(FOLDER)/
	cp $(PROG) $(FOLDER)/

clean:
	-rm *.o
	-rm *.map


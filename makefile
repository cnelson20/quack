
ifeq ($(OS),Windows_NT)
CC = cl65.exe
else
endif
FLAGS = -m quack.map -tcx16 -Ois --codesize 200 -Cl
PROG = QUACK.PRG
SOURCES = main.c routines.s
HEADERS = main.h routines.h

all: $(PROG)

$(PROG): $(SOURCES) $(HEADERS)
	$(CC) $(FLAGS) -o $(PROG) $(SOURCES) -lib zsound/zsound.lib

copy:
	-mkdir game/
	cp *.BIN game/
	cp *.ZCM game/
	cp $(PROG) game/

clean:
	-rm *.o
	-rm *.map


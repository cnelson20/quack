all: QUACK.PRG

CC = cl65.exe
FLAGS = -m quack.map -tcx16 -Ois --codesize 200 -Cl
SOURCES = main.c routines.s
HEADERS = main.h routines.h

QUACK.PRG: $(SOURCES) $(HEADERS)
	$(CC) $(FLAGS) -o QUACK.PRG $(SOURCES) -lib zsound/zsound.lib


clean:
	-rm *.o
	-rm *.map

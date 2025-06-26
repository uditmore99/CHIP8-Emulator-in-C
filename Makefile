all:
	gcc main.c  -Isrc/include -Lsrc/lib -lSDL2main -lSDL2 -o main.exe

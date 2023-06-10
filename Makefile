all: compile link
	
compile:
	g++ -Isrc/include -c tetris.cpp

link:
	g++ tetris.o -o tetris -Lsrc/lib -lsfml-graphics -lsfml-window -lsfml-system

clean:
	rm tetris.o
	rm tetris.exe
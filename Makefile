all: rungame

rungame: game.cpp glad.c
	g++ -o game game.cpp glad.c -lGL -lglfw -lftgl -lSOIL -ldl -I/usr/local/include -I/usr/local/include/freetype2 -L/usr/local/lib/ -L/usr/lib -lsfml-audio

clean:
	rm game

all:
	gcc maze.c -o run `pkg-config --cflags --libs cairo`

clean:
	rm run

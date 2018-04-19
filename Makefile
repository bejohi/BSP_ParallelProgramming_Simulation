all: build
	./bin/task1

build: prep
	gcc -std=c99 -Wall -O3 -o bin/task1 src/task1.c src/libmcbsp1.2.0.a -pthread -lrt -lm

prep: clean
	mkdir bin/

clean:
	rm -rf bin/
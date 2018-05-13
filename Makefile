all: build
	./bin/task1

task1: build
	./bin/task1

task2: build2
	./bin/task2

build: prep
	gcc -std=c99 -Wall -O3 -o bin/task1 src/task1.c src/libmcbsp1.2.0.a -pthread -lrt -lm

build2: prep
	gcc -std=c99 -Wall -O3 -o bin/task2 src/task2.c src/libmcbsp1.2.0.a -pthread -lrt -lm

prep: clean
	mkdir bin/

clean:
	rm -rf bin/
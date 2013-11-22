all: debug release

release:
	gcc -std=gnu11 -Wall -pedantic -O2 src/First_period.c -lm -s -o bin/First_period

debug:
	gcc -std=gnu11 -DDEBUG -Wall -pedantic -O0 src/First_period.c -lm -g -o bin/First_period_dbg

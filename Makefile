all: debug release

release:
	gcc -Wall -pedantic -O2 src/First_period.c -lm -s -o bin/First_period

debug:
	gcc -g -DDEBUG -Wall -pedantic -O0 src/First_period.c -lm -o bin/First_period_dbg

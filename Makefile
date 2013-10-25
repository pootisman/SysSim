all: debug release

release:
	gcc -Wall -pedantic -O2 src/First_period.c -lm -s -o bin/First_period

debug:
	gcc -DDEBUG -Wall -pedantic -O0 src/First_period.c -lm -g -o bin/First_period_dbg

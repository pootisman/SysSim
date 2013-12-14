all: debug release

release:
	gcc -Wall -pedantic -O2 src/First_period.c -lm -s -o bin/First_period
	gcc -Wall -pedantic -O2 src/sys_recoverable.c -lm -s -o bin/SysRecoverable
debug:
	gcc -g -DDEBUG -Wall -pedantic -O0 src/First_period.c -lm -o bin/First_period_dbg
	gcc -g -DDEBUG -Wall -pedantic -O0 src/sys_recoverable.c -lm -o bin/SysRecoverable_dbg

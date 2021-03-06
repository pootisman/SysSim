all: debug release

release:
	gcc -Wall -pedantic -O2 src/First_period.c -lm -s -o bin/First_period
	gcc -Wall -pedantic -O2 src/sys_recoverable.c -lm -s -o bin/SysRecoverable
	gcc -DTHREADS -Wall -pedantic -O2 src/First_period.c -lm -pthread -s -o bin/FPT
	gcc -DTHREADS -DINVALID_TEST -Wall -pedantic -O2 src/First_period.c -lm -pthread -s -o bin/FPT_wr
debug:
	gcc -g -DDEBUG -Wall -pedantic -O0 src/First_period.c -lm -o bin/First_period_dbg
	gcc -g -DDEBUG -Wall -pedantic -O0 src/sys_recoverable.c -lm -o bin/SysRecoverable_dbg
	gcc -g -DDEBUG -DTHREADS -Wall -pedantic -O0 src/First_period.c -lm -pthread -o bin/FPT_dbg
	gcc -g -DDEBUG -DTHREADS -DINVALID_TEST -Wall -pedantic -O0 src/First_period.c -lm -pthread -o bin/FPT_wr_dbg

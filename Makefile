CC := gcc
CCFLAGS := -Ofast -msse2 -march=native -Wall -Wextra -Wpedantic

all: parse_test

parse_test: parse_test.c parser.o stable.o buffer.o asmtypes.o optable.o error.o queue.o
	$(CC) $(CCFLAGS) $< *.o -o $@

parser.o: parser.c
	$(CC) $(CCFLAGS) -c $< -o $@

buffer.o: buffer.c
	$(CC) $(CCFLAGS) -c $< -o $@

stable.o: stable.c
	$(CC) $(CCFLAGS) -c $< -o $@

asmtypes.o: asmtypes.c
	$(CC) $(CCFLAGS) -c $< -o $@

optables.o: optables.c
	$(CC) $(CCFLAGS) -c $< -o $@

error.o: error.c
	$(CC) $(CCFLAGS) -c $< -o $@

queue.o: queue.c
	$(CC) $(CCFLAGS) -c $< -o $@

clean:
	rm *.o parse_test



CC = gcc
CFLAGS = -std=c89 -pedantic-errors # -Wall -Werror

default: build

clean:
	rm -rf out

build: out/main.o out/mem.o
	$(CC) $(CFLAGS) out/*.o -o out/lab7

run: build
	out/lab7

out/%.o: src/%.c
	mkdir -p out
	$(CC) $(CFLAGS) src/$*.c -c -o out/$*.o

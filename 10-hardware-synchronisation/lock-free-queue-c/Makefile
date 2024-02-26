CC=gcc
#CFLAGS=-g -fsanitize=address
CFLAGS=-g -fsanitize=thread
#CFLAGS=-O3

all: queue-lockfree-v1 queue-lockfree-v2 queue-lockfree-v3 queue-locks \
	queue-lockfree-v4

queue-lockfree-v1: queue-lockfree-v1.c main.c queue-lockfree.h params.h
	gcc $(CFLAGS) queue-lockfree-v1.c main.c -lpthread -o $@

queue-lockfree-v2: queue-lockfree-v2.c main.c queue-lockfree.h params.h
	gcc $(CFLAGS) queue-lockfree-v2.c main.c -lpthread -o $@

queue-lockfree-v3: queue-lockfree-v3.c main.c queue-lockfree.h params.h
	gcc $(CFLAGS) -D__V3__ queue-lockfree-v3.c main.c -lpthread -o $@

queue-lockfree-v4: queue-lockfree-v4.c main.c queue-lockfree.h params.h
	gcc $(CFLAGS) -D__V4__ -std=c11 -D_DEFAULT_SOURCE queue-lockfree-v4.c main.c -lpthread -latomic -o $@

queue-locks: queue-locks.c main.c queue-lockfree.h params.h
	gcc $(CFLAGS) -D__USE_LOCKS__ queue-locks.c main.c -lpthread -o $@

test-v1: queue-lockfree-v1
	./queue-lockfree-v1

test-v2: queue-lockfree-v2
	./queue-lockfree-v2

test-v3: queue-lockfree-v3
	./queue-lockfree-v3

test-v4: queue-lockfree-v4
	./queue-lockfree-v4

test-locks: queue-locks
	./queue-locks

clean:
	rm -rf *.o queue-lockfree-v2 queue-lockfree-v3 queue-lockfree-v1 \
		queue-lockfree-v4 queue-locks

archive: clean
	zip -r queue-lockfree-c.zip *

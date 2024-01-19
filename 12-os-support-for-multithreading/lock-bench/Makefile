PROGS=lock-bench-custom-futex lock-bench-futex lock-bench-sysv-semaphores

all: $(PROGS)

lock-bench-custom-futex: lock-bench-custom-futex.c lock-bench.h
	gcc lock-bench-custom-futex.c -o lock-bench-custom-futex -lpthread

lock-bench-futex: lock-bench-futex.c lock-bench.h
	gcc lock-bench-futex.c -o lock-bench-futex -lpthread

lock-bench-sysv-semaphores: lock-bench-sysv-semaphores.c lock-bench.h
	gcc lock-bench-sysv-semaphores.c -o lock-bench-sysv-semaphores -lpthread

clean:
	rm -rf *.o $(PROGS)

test: $(PROGS)
	@echo "System V semaphores:"
	./lock-bench-sysv-semaphores
	@echo "Pthread_mutex (futex):"
	./lock-bench-futex
	@echo "custom futex lock:"
	./lock-bench-custom-futex
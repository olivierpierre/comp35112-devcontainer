# Disable the JIT compiler that messes with the performance, and give plenty
# of memory to the JVM to avoid GC as much as possible
JRE_ARGS=-Djava.compiler=none -Xmx2G -Xms2G

all: main.java Queue.java QueueLocks.java QueueLockFree.java
	javac $^
	java $(JRE_ARGS) Main

archive: Makefile main.java Queue.java QueueLocks.java QueueLockFree.java
	zip -r queue-java.zip $^

clean:
	rm *.class
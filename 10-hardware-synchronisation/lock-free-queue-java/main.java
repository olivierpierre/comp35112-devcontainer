import java.util.Random;

class Worker extends Thread {
    private Queue queue;
    private int elem_num;

    Worker(Queue q, int e_num) {
        queue = q;
        elem_num = e_num;
    }

    public void run() {
        Integer number = Integer.valueOf(42);

        for(int i=0; i<elem_num; i++)
            queue.enqueue(number);

        for(int i=0; i<elem_num; i++) {
            try {
                Integer res = (Integer)queue.dequeue();
                if(res != number) {
                    System.out.println("ERROR: non-enqueued element dequeued: " +
                        res);
                }
            } catch (Queue.EmptyException e) {
                // The program will hang if a thread don't dequeue exactly the
                // same number of element it enqueued
                i--;
            }
        }
    }
}

class Main {

    // number of element each thread will enqueue then dequeue:
    static int ELEM_NUM = 100000;
    // number of threads:
    static int THREAD_NUM = 32;

    public static void main(String[] args) {
        runLocks();
        runLockFree();
    }

    public static void runLocks() {

        // We'll reset the same seed for both experiments (locks/lockfree) so
        // that the same number of elements is used and time is comparable
        Random r = new Random();
        r.setSeed(0);

        QueueLocks q = new QueueLocks();
        Worker [] threads = new Worker[THREAD_NUM];
        for(int i=0; i<THREAD_NUM; i++)
            threads[i] = new Worker(q, r.nextInt(ELEM_NUM));

        long exec_time = doRun(threads);

        System.out.println("[LOCK-BASED] " + THREAD_NUM + 
            " thread enqueue/dequeue " + ELEM_NUM + " elements in " + 
            exec_time + "ms");
    }

    public static void runLockFree() {

        Random r = new Random();
        r.setSeed(0);

        QueueLockFree q = new QueueLockFree();
        Worker [] threads = new Worker[THREAD_NUM];
        for(int i=0; i<THREAD_NUM; i++)
            threads[i] = new Worker(q, r.nextInt(ELEM_NUM));

        long exec_time = doRun(threads);

        try {
            Integer x = (Integer)q.dequeue();
            System.out.println("ERROR: queue not empty (contains " + x + ")" +
                "after all threads are done");
        } catch (Queue.EmptyException e) {;} // this is expected

        System.out.println("[LOCK-FREE] " + THREAD_NUM + 
            " thread enqueue/dequeue " + ELEM_NUM + " elements in " + 
            exec_time + "ms");
    }

    // run all threads and return total execution time in ms
    public static long doRun(Worker[] threads) {

        // start all threads
        long start = System.nanoTime();
        for(int i=0; i<THREAD_NUM; i++)
            threads[i].start();

        try {
            // wait for all threads to finish
            for(int i=0; i<THREAD_NUM; i++)
                threads[i].join();
        } catch (InterruptedException e) {}
        long stop = System.nanoTime();

        return ((stop-start) / 1000000);
    }
}

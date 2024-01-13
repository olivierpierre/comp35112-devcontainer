class MyRunnable implements Runnable {
    int id;

    MyRunnable(int id) {
        this.id = id;
    }

    public void run() {
        System.out.println("Thread " + id + " running");
    }
}

class Demo {
    public static void main(String[] args) {
        int NOWORKERS = 5;
        Thread[] threads = new Thread[NOWORKERS];

        for (int i = 0; i < NOWORKERS; i++) {
            MyRunnable r = new MyRunnable(i);
            threads[i] = new Thread(r);
        }

        for (int i = 0; i < NOWORKERS; i++)
            threads[i].start();

        for (int i = 0; i < NOWORKERS; i++)
            try {
                threads[i].join();
            } catch (InterruptedException e) { /* do nothing */ }

        System.out.println("All done");
    }
}


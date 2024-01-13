class MyThread extends Thread {
    int id;

    MyThread(int id) {
        this.id = id;
    }

    public void run() {
        System.out.println("Thread " + id + " running");
    }
}

class Demo {
    public static void main(String[] args) {
        int NOWORKERS = 5;
        MyThread[] threads = new MyThread[NOWORKERS];

        for (int i = 0; i < NOWORKERS; i++)
            threads[i] = new MyThread(i);

        for (int i = 0; i < NOWORKERS; i++)
            threads[i].start();

        for (int i = 0; i < NOWORKERS; i++)
            try {
                threads[i].join();
            } catch (InterruptedException e) { /* do nothing */ }

        System.out.println("All done");
    }
}


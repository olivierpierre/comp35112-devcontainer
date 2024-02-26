import java.util.concurrent.locks.ReentrantLock;

class QueueLocks implements Queue {
    private Node head, tail;
    private ReentrantLock enLock, deLock;

    class Node {
        private Object value;
        private Node next;

        public Node(Object newValue) {
            value = newValue;
            next = null;
        }
    }

    public QueueLocks() {
        head = tail = new Node(null);
        enLock = new ReentrantLock();
        deLock = new ReentrantLock();
    }

    public void enqueue(Object item) {
        enLock.lock();

        try {
            Node n = new Node(item);
            tail.next = n;
            tail = n;
        } finally {
            enLock.unlock();
        }
    }

    public Object dequeue() throws EmptyException {
        Object result;
        deLock.lock();
        try {

            if(head.next == null)
                throw new EmptyException();

            head = head.next;
            result = head.value;

        } finally {
            deLock.unlock();
        }

        return result;
    }
}
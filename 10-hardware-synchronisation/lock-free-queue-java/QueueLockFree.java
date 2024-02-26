import java.util.concurrent.atomic.AtomicReference;

class EmptyException extends Exception {
    public EmptyException() {
        super();
    }
}

class QueueLockFree implements Queue {

    class Node {
        private Object value;
        private AtomicReference<Node> next;

        public Node(Object newValue) {
            value = newValue;
            next = new AtomicReference<Node>(null);
        }
    }

    private AtomicReference<Node> head, tail;

    public QueueLockFree() {
        head = new AtomicReference<Node>(new Node(null));
        tail = new AtomicReference<Node>(head.get());
    }

    public void enqueue(Object item) {
        Node n = new Node(item);

        // get a local reference to the tail then enqueue with CAS the node at
        // the end of the queue if our local reference is still valid and then
        // update the queue's tail to the newly inserted node
        while(true) {
            Node local_tail = tail.get();

            // This actually works: even if an enqueing thread is interrupted
            // after the cas and before setting tail, all other enqueing threads
            // will be blocked because tail.next will never be null, so
            // eventually the interrupted thread can resume and finish the
            // operation. During the interruption, dequeing threads may update
            // head and even dequeue the tail node referenced by local_tail, but
            // the gc will not free/reuse it because we still hold a reference
            // to it.
            if(local_tail.next.compareAndSet(null, n)) {
                // No need for a CAS here, by construction nobody can have
                // touched tail since our successful CAS on local_tail
                tail.set(n);
                return;
            }
        }
    }

    public Object dequeue() throws EmptyException {

        while(true) {
            // get local copies of the head node reference, and the next node too
            // so we can check if it's NULL (i.e. queue empty) and later use it to
            // set the new head
            Node local_head = head.get();
            Node local_head_next = local_head.next.get();

            // queue empty?
            if(local_head_next == null)
                throw new EmptyException();

            // The value we'll return if we succeed with the dequeue operation
            Object result = local_head_next.value;

            // the head may have been modified since we grabbed our local
            // copies, ensure it is not the case and update the head reference
            // to the next node if we still reference the valid head with a CAS
            if(head.compareAndSet(local_head, local_head_next)) {
                // CAS succeeded, return the value
                return result;
            }
        }
    }
}
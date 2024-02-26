interface Queue {

    class EmptyException extends Exception {
        public EmptyException() {
            super();
        }
    }

    public void enqueue(Object item);
    public Object dequeue() throws EmptyException;
}

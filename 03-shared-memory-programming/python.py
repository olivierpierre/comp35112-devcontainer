#!/usr/bin/python3

import threading

NOWORKERS=5

def print_hello(id):
    print("Thread " + str(id) + " running")

threads = []
for i in range(NOWORKERS):
    thread = threading.Thread(target=print_hello, args=(i,))
    threads.append(thread)
    thread.start()

for thread in threads:
    thread.join()

print("All done.")
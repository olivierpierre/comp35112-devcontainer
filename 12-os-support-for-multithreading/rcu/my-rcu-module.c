// This is a module that requires running the Linux kernel either natively or
// in a VM. It won't work on e.g. Docker for Mac/Windows or WSL. To build
// the module you'll need the kernel headers:
// sudo apt install linux-headers-$(uname -r)
//
// To compile use the supplied Makefile or the following command:
// make -C  /lib/modules/$(uname -r)/build M=$(pwd) modules
//
// To load the module:
// sudo insmod my-rcu-module.ko
//
// To read the kernel log:
// sudo dmesg
//
// To unload the module:
// sudo rmmod my-rcu-module.ko

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/rcupdate.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");

// Number of reading threads
#define NUM_THREADS 64
// for sanity checks: c member of the struct should always have the same value
#define C_VALUE     42

// our data structure
struct foo {
    int a;
    int b;
    int c;
};

// reader and writer threads
struct task_struct *read_threads[NUM_THREADS];
struct task_struct *write_thread;

// spinlock serialising the write accesses
DEFINE_SPINLOCK(foo_mutex);

// Global data structure protected by rcu
struct foo __rcu *gbl_foo;

// writer function
void foo_write(int new_value_for_a_and_b) {
    struct foo *new_fp, *old_fp;
    
    // Allocate space for the copy
    new_fp = kmalloc(sizeof(*new_fp),  GFP_KERNEL);
    
    spin_lock(&foo_mutex);

    // get a ref to the protected global data:
    old_fp = rcu_dereference_protected(gbl_foo, lockdep_is_held(&foo_mutex));
    *new_fp = *old_fp; // copy data

    // update data in the copy:
    new_fp->a = new_value_for_a_and_b;
    new_fp->b = new_value_for_a_and_b;
    
    // atmoic ref update:
    rcu_assign_pointer(gbl_foo, new_fp);

    spin_unlock(&foo_mutex);

    synchronize_rcu(); // wait for grace period
    kfree(old_fp);     // free old data
}

// reader function
void foo_read(void) {
    struct foo *fp;
    int a, b, c;
    
    rcu_read_lock();
    fp = rcu_dereference(gbl_foo);
    
    a = fp->a;
    b = fp->b;
    c = fp->c;
    rcu_read_unlock();

    // Sanity check: readers must always have a consistent view, because the
    // writers always set a and b to the same value, check that they are equal.
    // The writer should also just copy the value of c which is supposed to be
    // always the same
    if(a != b || c != C_VALUE)
        printk(KERN_ERR "(reader) inconsistent state! a=%d b=%d, c=%d",
            a, b, c);

}

// Thread function for readers
static int reader_thread(void *data) {
    while (!kthread_should_stop()) {
        foo_read();
        msleep(10); // sleep for 10 ms
    }

    return 0;
}

// Thread function for writers
static int writer_thread(void *data) {
    int i = 0;

    while (!kthread_should_stop()) {
        foo_write(i++);
        msleep(100); // sleep for 100 ms
    }

    return 0;
}

// Module entry point
static int __init my_rcu_init(void) {
    int i;

    pr_info("MyRCU Module Loaded\n");

    // init the global data
    gbl_foo = kmalloc(sizeof(struct foo), GFP_KERNEL);
    if (!gbl_foo) {
        pr_err("Failed to allocate memory\n");
        return -ENOMEM;
    }

    gbl_foo->a = 0;
    gbl_foo->b = 0;
    gbl_foo->c = 42;

    // Create reader threads
    for (i = 0; i < NUM_THREADS/2; ++i) {
        read_threads[i] = kthread_run(reader_thread, NULL, "reader_thread_%d", i);
        if (IS_ERR(read_threads[i])) {
            pr_err("Failed to create reader thread %d\n", i);
            return PTR_ERR(read_threads[i]);
        }
    }

    // create writer thread
    write_thread = kthread_run(writer_thread, NULL, "writer_thread_%d", i);
    if (IS_ERR(write_thread)) {
        pr_err("Failed to create writer thread\n");
        return PTR_ERR(write_thread);
    }

    return 0;
}

// Module cleanup
static void __exit my_rcu_exit(void) {
    int i;

    // Stop reader threads
    for (i = 0; i < NUM_THREADS/2; ++i)
        if (read_threads[i])
            kthread_stop(read_threads[i]);

    if(write_thread)
        kthread_stop(write_thread);

    pr_info("MyRCU Module Unloaded\n");

}

module_init(my_rcu_init);
module_exit(my_rcu_exit);

# Mergesort 

## Code Snippets for respective sorts
* **Normal mergesort**
```c
void normalMergesort(int a[], int l, int r) {
    if (r - l + 1 < 5) {
        selectionSort(a, l, r);
        return;
    }

    int mid = l + (r - l) / 2;
    normalMergesort(a, l, mid);
    normalMergesort(a, mid + 1, r);
    merge(a, l, mid, r);
}
```
* **Process mergesort**
```c
void concurrentMergesort(int a[], int l, int r) {
    if (r - l + 1 < 5) {
        selectionSort(a, l, r);
        return;
    }

    int mid = l + (r - l) / 2;
    pid_t left_pid, right_pid;
    left_pid = fork();
    if (left_pid < 0) {
        perror("Error in forking: ");
        exit(1);
    } else if (left_pid == 0) {
        concurrentMergesort(a, l, mid);
        exit(0);
    } else {
        right_pid = fork();
        if (right_pid < 0) {
            perror("Error in forking: ");
            exit(1);
        } else if (right_pid == 0) {
            concurrentMergesort(a, mid + 1, r);
            exit(0);
        }
    }
    waitpid(left_pid, NULL, 0);
    waitpid(right_pid, NULL, 0);
    merge(a, l, mid, r);
}
```
* **Threaded mergesort**
```c
void *threadedMergesort(void *p) {
    Thread *x = (Thread *) p;
    int l = x->l, r = x->r;
    if (r - l + 1 < 5) {
        selectionSort(x->array, l, r);
        return NULL;
    }

    int mid = l + (r - l) / 2;
    Thread a, b;
    a.l = l, a.r = mid;
    b.l = mid + 1, b.r = r;
    a.array = b.array = x->array;

    pthread_t tid1, tid2;
    if (pthread_create(&tid1, NULL, threadedMergesort, (void *) &a)) {
        printf("Error in thread creation");
        exit(1);
    }
    if (pthread_create(&tid2, NULL, threadedMergesort, (void *) &b)) {
        printf("Error in thread creation");
        exit(1);
    }
    if (pthread_join(tid1, NULL)) {
        printf("Error in thread join");
        exit(1);
    }
    if (pthread_join(tid2, NULL)) {
        printf("Error in thread join");
        exit(1);
    }
    merge(x->array, l, mid, r);
    return NULL;
}
```
## Conclusion
In the case of concurrent mergesort (Process and Threaded), when the left child accesses the left array, the array is loaded into the processor's cache. Due to concurrent access, when the right array is accessed, there is a cache miss since the cache is filled with the left segment, and then the right segment is copied to the cache memory. This process further degrades a high level, and thus, it performs more inferior than the normal mergesort code. 

Comparing process and threaded mergesort, there is high overhead and more time in creating a process than a thread.
* ***n = 10***
```c
Time Taken by Normal Mergesort is 0.00000195 seconds.
Time Taken by Concurrent Mergesort is 0.00124960 seconds.
Time Taken by Threaded Mergesort is 0.00069248 seconds.

Normal Mergesort is 641.803809 time faster than Concurrent Mergesort.
Normal Mergesort is 355.660967 time faster than Threaded Mergesort.
```

* ***n = 100***
```c
Time Taken by Normal Mergesort is 0.00001894 seconds.
Time Taken by Concurrent Mergesort is 0.00650762 seconds.
Time Taken by Threaded Mergesort is 0.00620244 seconds.

Normal Mergesort is 343.536676 time faster than Concurrent Mergesort.
Normal Mergesort is 327.426303 time faster than Threaded Mergesort.
```
* ***n = 1000***

```c
Time Taken by Normal Mergesort is 0.00023571 seconds.
Time Taken by Concurrent Mergesort is 0.03247264 seconds.
Time Taken by Threaded Mergesort is 0.02563417 seconds.

Normal Mergesort is 137.765215 time faster than Concurrent Mergesort.
Normal Mergesort is 108.752988 time faster than Threaded Mergesort.
```
* ***n = 10000***

```c
Time Taken by Normal Mergesort is 0.00325276 seconds.
Time Taken by Concurrent Mergesort is 0.61593297 seconds.
Time Taken by Threaded Mergesort is 0.48487531 seconds.

Normal Mergesort is 189.357030 time faster than Concurrent Mergesort.
Normal Mergesort is 149.065813 time faster than Threaded Mergesort.
```
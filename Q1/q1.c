//#define _POSIX_C_SOURCE 199309L //required for clock
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>

typedef struct thread {
    int l, r;
    int *array;
} Thread;

void selectionSort(int a[], int l, int r) {
    for (int i = l; i <= r; i++) {
        int k = i;
        for (int j = i + 1; j <= r; j++) {
            if (a[j] < a[k]) {
                k = j;
            }
        }
        int temp = a[i];
        a[i] = a[k];
        a[k] = temp;
    }
}

void merge(int a[], int l, int mid, int r) {
    int n1 = mid - l + 1;
    int n2 = r - mid;
    int L[n1], R[n2];
    for (int i = l, j = 0; i <= mid && j < n1; i++, j++) {
        L[j] = a[i];
    }
    for (int i = mid + 1, j = 0; i <= r && j < n2; i++, j++) {
        R[j] = a[i];
    }

    int k = l, i = 0, j = 0;
    while (i < n1 && j < n2) {
        a[k++] = L[i] < R[j] ? L[i++] : R[j++];
    }
    while (i < n1) {
        a[k++] = L[i++];
    }
    while (j < n2) {
        a[k++] = R[j++];
    }
}

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

signed main() {

    struct timespec ts;

    int n;
    scanf("%d", &n);

    key_t key = IPC_PRIVATE;
    int *shared_array;
    size_t SHM_SIZE = sizeof(int) * n;
    int shmid;
    if ((shmid = shmget(key, SHM_SIZE, IPC_CREAT | 0666)) < 0) {
        perror("shmget: ");
        exit(1);
    }
    if ((shared_array = shmat(shmid, NULL, 0)) == (int *) -1) {
        perror("shmat: ");
        exit(1);
    }

    int a[n], b[n];
    for (int i = 0; i < n; i++) {
        scanf("%d", a + i);
        b[i] = a[i];
        shared_array[i] = a[i];
    }

    // concurrent mergesort
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    long double st = ts.tv_nsec/(1e9)+ts.tv_sec;
    concurrentMergesort(shared_array, 0, n - 1);
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    long double en = ts.tv_nsec/(1e9)+ts.tv_sec;
    long double time1 = en - st; // concurrent
    printf("Concurrent: \n");
    for(int i = 0; i< n; i++){
        printf("%d ", shared_array[i]);
    }
    printf("\n\n");

    // normal mergesort
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    st = ts.tv_nsec/(1e9)+ts.tv_sec;
    normalMergesort(b, 0, n - 1);
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    en = ts.tv_nsec/(1e9)+ts.tv_sec;
    long double time2 = en - st;
    printf("Normal: \n");
    for(int i = 0; i< n; i++){
        printf("%d ", b[i]);
    }
    printf("\n\n");

    // threaded mergesort
    pthread_t tid;
    Thread p;
    p.l = 0, p.r = n - 1;
    p.array = a;

    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    st = ts.tv_nsec/(1e9)+ts.tv_sec;
    if (pthread_create(&tid, NULL, threadedMergesort, (void *) &p)) {
        printf("Error in creation");
        exit(1);
    }
    if (pthread_join(tid, NULL)) {
        printf("Error in thread join");
        exit(1);
    }
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    en = ts.tv_nsec/(1e9)+ts.tv_sec;
    long double time3 = en - st;
    printf("Threaded: \n");
    for(int i = 0; i< n; i++){
        printf("%d ", a[i]);
    }
    printf("\n\n");


    printf("Time Taken by Normal Mergesort is %.8Lf seconds.\n", time2);
    printf("Time Taken by Concurrent Mergesort is %.8Lf seconds.\n", time1);
    printf("Time Taken by Threaded Mergesort is %.8Lf seconds.\n\n", time3);

    printf("Normal Mergesort is %.6Lf time faster than Concurrent Mergesort.\n", time1 / time2);
    printf("Normal Mergesort is %.6Lf time faster than Threaded Mergesort.\n", time3 / time2);

    if (shmdt(shared_array) == -1) {
        perror("shmdt: ");
        exit(1);
    }
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl: ");
        exit(1);
    }

    return 0;
}
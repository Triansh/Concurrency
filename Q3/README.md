# Musical Mayhem

## Overview

Performers (Musicians and Singers) arrive at times as specified in the input. They have a specific tolerance time up given as input to which they wait. Otherwise, they leave upset. They may require
different stages as per their need. There are two stages - Acoustic and Electric. Singers can take
any of the performance stages, and also they can join an ongoing performance by a musician. Performers perform
for a random amount of time, and then they leave, clearing the stage instantly. They wait to get T-shirts ( both musicians and singers), and after that, they exit. We maintain
the record of stage numbers on which they arrive.

## Implementation

### Choosing stages
* Different number of threads based on the performer type and the type of stage he/she requires.
```c
pthread_t tid1, tid2, tid3;
if (p->type == ACS) {
    pthread_create(&tid1, NULL, findAcs, (void *) p);
} else if (p->type == ELC) {
    pthread_create(&tid1, NULL, findEle, (void *) p);
} else {
    pthread_create(&tid1, NULL, findAcs, (void *) p);
    pthread_create(&tid2, NULL, findEle, (void *) p);
    if (p->type == SNG) {
        pthread_create(&tid3, NULL, findPart, (void *) p);
    }
}
```
* `findAcs()` and `findEle()` determine whether acoustic and electric stages are free or not
for a musician to perform or for a singer with solo performance. The count of these stages are 
maintained by using two semaphores `acoustic` and `electric`.
```c
void *findAcs(void *inp) {
    Player *p = (Player *) inp;
    sem_wait(&acoustic);
    pthread_mutex_lock(&(p->com_mutex));
    if (p->gotStage == true) {
        sem_post(&acoustic);
        pthread_mutex_unlock(&(p->com_mutex));
    } else {
        ...
        p = getStage(p, ACS);
        printf(CYAN"%s %c started performing on Acoustic Stage (%d).\n" RESET, p->name, p->ins, p->stage_id);
        sem_post(&(p->par_sem));
        if (p->type != SNG) {
            sem_post(&partnership);
        } else {
            p->solo_sing = true;
        }
        pthread_mutex_unlock(&(p->com_mutex));
    }
    return NULL;
}
```
* The `partnership` is another type of semaphore which records the total number of musicians
which are available to be partnered with singers. The semaphore value is increased whenever 
a musician enters a stage and decreases when he/she exits after a solo performance.
* Singers can also choose to be partnered with musicians in `findPart()` function.
```c
for (int i = 0; i < no_player; i++) {
    pthread_mutex_lock(&(players[i].sel_mutex));
    if (players[i].partner_id == -1 && players[i].type != SNG && players[i].state == 1) {
        ...
        pthread_mutex_unlock(&(players[i].sel_mutex));
        break;
    } else {
        pthread_mutex_unlock(&(players[i].sel_mutex));
    }
}
```
* Each `Player` structure contains a semaphore `par_sem` which tells the parent thread
whether the performer has got any stage or not. There is a strict time limit for which the 
performer waits. If he/she doesn't gets any stage by then , he/she exits. This is implemented
using `sem_timedwait()` function defined in `semaphore.h`.
```c
int ret = sem_timedwait(&(p->par_sem), &ts);

pthread_mutex_lock(&(p->com_mutex));
if (ret == -1 && errno == ETIMEDOUT && (!p->gotStage)) {
    p->gotStage = true;
    p->state = 2;
    pthread_mutex_unlock(&(p->com_mutex));
    printf(RED"%s %c left due to impatience.\n" RESET, p->name, p->ins);
    return NULL;
}
pthread_mutex_unlock(&(p->com_mutex));
```
* The stage number is maintained by keeping a mutex lock and an array of `Stage` structures which
ensure no 2 performers access the same stage simultaneously for solo performances. This is
implemented in `getStage()` function.
```c
Player *getStage(Player *p, int x) {
    for (int i = 0; i < no_stage; i++) {
        pthread_mutex_lock(&(stages[i].mutex));
        if (stages[i].state == 0 && stages[i].type == x) {
            stages[i].state = 1;
            p->stage_id = i;
            pthread_mutex_unlock(&(stages[i].mutex));
            break;
        } else {
            pthread_mutex_unlock(&(stages[i].mutex));
        }
    }
    return p;
}
```
* The singer who joined the performance by a musician waits for the musician to complete
his/her performance. This is done by waiting on semaphore `sing_sem`.  When the musician
completes his/her performance, he post the `sing_sem` semaphore which is then received by singer and then they both exit.
* In the above case, the performance is also extended by 2 seconds.


### Giving T-shirts
* Whenever a performer completes his/her performance, it waits to get a T-shirt. This is
implemented by waiting on a semaphore `coordinators` in function `getShirts()`. Since each coordinator can service 
only one performer at a time, if the queue is full, then the performers have to wait.
```c
void getShirts(Player *p) {
    printf(BLUE"%s %c collecting T-shirt\n" RESET, p->name, p->ins);
    sem_wait(&coordinators);
    sleep(2);
    sem_post(&coordinators);
    printf(MAGENTA "%s %c exited after performing and collecting T-shirt.\n" RESET, p->name, p->ins);
}
```
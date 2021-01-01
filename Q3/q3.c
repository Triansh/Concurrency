#include "q3.h"

int getRandomInRange(int l, int r) {
    return l + (rand() % (r - l + 1));
}

Player *getStage(Player *p, int x) {  // gives a stage number to performer
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

void *findAcs(void *inp) { // child acoustic thread for getting acoustic stages

    Player *p = (Player *) inp;
    sem_wait(&acoustic);

    pthread_mutex_lock(&(p->com_mutex));
    if (p->gotStage == true) {
        sem_post(&acoustic);
        pthread_mutex_unlock(&(p->com_mutex));
    } else {
        p->gotStage = true;
        p->st_type = ACS;
        p->state = 1;
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


void *findEle(void *inp) { /// child electric thread for getting electric stages

    Player *p = (Player *) inp;
    sem_wait(&electric);

    pthread_mutex_lock(&(p->com_mutex));
    if (p->gotStage == true) {
        sem_post(&electric);
        pthread_mutex_unlock(&(p->com_mutex));
    } else {
        p->gotStage = true;
        p->st_type = ELC;
        p->state = 1;
        p = getStage(p, ELC);
        printf(CYAN"%s %c started performing on Electric Stage (%d).\n" RESET, p->name, p->ins, p->stage_id);
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

void *findPart(void *inp) { /// child partnership thread for getting partners for singers

    Player *p = (Player *) inp;
    sem_wait(&partnership);

    pthread_mutex_lock(&(p->com_mutex));
    if (p->gotStage == true) {
        sem_post(&partnership);
        pthread_mutex_unlock(&(p->com_mutex));
    } else {
        p->gotStage = true;
        for (int i = 0; i < no_player; i++) {
            pthread_mutex_lock(&(players[i].sel_mutex));
            if (players[i].partner_id == -1 && players[i].type != SNG && players[i].state == 1) {

                pthread_mutex_lock(&(stages[players[i].stage_id].mutex)); /////////////////////
                p->stage_id = players[i].stage_id;
                pthread_mutex_unlock(&(stages[players[i].stage_id].mutex));

                players[i].partner_id = p->id;
                p->partner_id = i;
                p->state = 1;
                p->st_type = players[i].st_type;
                printf(CYAN"%s joined %s performance on %s stage (%d), performance extended by 2 secs.\n" RESET,
                       p->name, players[i].name, p->st_type == ACS ? "Acoustic" : "Electric", p->stage_id);
                pthread_mutex_unlock(&(players[i].sel_mutex));
                break;
            } else {
                pthread_mutex_unlock(&(players[i].sel_mutex));
            }
        }
        sem_post(&(p->par_sem));
        pthread_mutex_unlock(&(p->com_mutex));
    }
    return NULL;

}

void getShirts(Player *p) { // giving shirts to performers
    printf(BLUE"%s %c collecting T-shirt\n" RESET, p->name, p->ins);
    sem_wait(&coordinators);
    sleep(2);
    sem_post(&coordinators);
    printf(MAGENTA "%s %c exited after performing and collecting T-shirt.\n" RESET, p->name, p->ins);
}

void *sendArrival(void *inp) { // arrival thread for performer
    Player *p = (Player *) inp;

    sleep(p->arrivalTime);
    p->state = 0;
    printf(GREEN"%s %c arrived. Waiting to perform for %d seconds.\n" RESET, p->name, p->ins, p->perf_time);

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

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += common_waiting_time;

    int ret = sem_timedwait(&(p->par_sem), &ts); // Timed wait

    pthread_mutex_lock(&(p->com_mutex));
    if (ret == -1 && errno == ETIMEDOUT && (!p->gotStage)) {
        p->gotStage = true;
        p->state = 2;
        pthread_mutex_unlock(&(p->com_mutex));
        printf(RED"%s %c left due to impatience.\n" RESET, p->name, p->ins);
        return NULL;
    }
    pthread_mutex_unlock(&(p->com_mutex));
    /*
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    long double st = ts.tv_nsec / (1e9) + ts.tv_sec;
     */

    if (p->type != SNG || p->solo_sing) { // performance time  + extension
        sleep(p->perf_time);

        pthread_mutex_lock(&(p->sel_mutex));
        if (p->partner_id != -1) { // checking partner
            sleep(2);
            sem_post(&(p->sing_sem));
        } else if (p->type != SNG) {
            sem_trywait(&partnership);
        }

        printf(YELLOW"%s %c performance at %s stage (%d) finished.\n"RESET, p->name, p->ins, ////////
               p->st_type == ACS ? "Acoustic" : "Electric", p->stage_id);
        pthread_mutex_lock(&(stages[p->stage_id].mutex));
        stages[p->stage_id].state = 0;
        pthread_mutex_unlock(&(stages[p->stage_id].mutex));
        p->state = 2;

        pthread_mutex_unlock(&(p->sel_mutex));

        p->st_type == ACS ? sem_post(&acoustic) : sem_post(&electric);


    } else {
        sem_wait(&(players[p->partner_id].sing_sem)); // for singer partnered
        printf(YELLOW"%s %c performance at %s stage (%d) finished.\n"RESET, p->name, p->ins,
               p->st_type == ACS ? "Acoustic" : "Electric", p->stage_id);
    }
    p->state = 2;

    /*
    printf("%s %d\n", p->name, p->partner_id);
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    long double en = ts.tv_nsec / (1e9) + ts.tv_sec;
    printf(" ###  %.5Lf\n", en - st);
    */

    getShirts(p);
    return NULL;
}

signed main() {

    srand(time(NULL));
    scanf("%d %d %d %d %d %d %d", &no_player, &no_ac, &no_el, &no_cord, &min_time, &max_time, &common_waiting_time);
    no_stage = no_el + no_ac;

    if (!(no_stage < STAGE_LIMIT && no_player < PERFORMER_LIMIT)) {
        printf(WHITE"Please enter atmost %d performers and atmost %d total stages.\n" RESET, PERFORMER_LIMIT,
               STAGE_LIMIT);
        return 0;
    }

    sem_init(&coordinators, 0, no_cord);
    sem_init(&acoustic, 0, no_ac);
    sem_init(&electric, 0, no_el);

    for (int i = 0; i < no_stage; i++) {
        stages[i].type = i < no_ac ? ACS : ELC;
        stages[i].state = 0;
        pthread_mutex_init(&(stages[i].mutex), NULL);
    }

    for (int i = 0; i < no_player; i++) {
        scanf("%s %c %d", players[i].name, &(players[i].ins), &(players[i].arrivalTime));
        if (players[i].ins == 's') {
            players[i].type = SNG;
        } else if (players[i].ins == 'v') {
            players[i].type = ACS;
        } else if (players[i].ins == 'b') {
            players[i].type = ELC;
        } else {
            players[i].type = ANY;
        }
        players[i].perf_time = getRandomInRange(min_time, max_time);
        players[i].id = i;
        players[i].st_type = players[i].partner_id = players[i].state = players[i].stage_id = -1;
        sem_init(&(players[i].par_sem), 0, 0);
        sem_init(&(players[i].sing_sem), 0, 0);
        pthread_mutex_init(&(players[i].com_mutex), NULL);
        pthread_mutex_init(&(players[i].sel_mutex), NULL);
        players[i].gotStage = false;
    }

    printf(WHITE"Simulation Started\n" RESET);

    for (int i = 0; i < no_player; i++) {
        pthread_create(&(players[i].tid), NULL, sendArrival, &players[i]);
    }

    for (int i = 0; i < no_player; i++) {
        pthread_join(players[i].tid, NULL);
    }
    printf(WHITE"Simulation Over\n" RESET);

    return 0;
}
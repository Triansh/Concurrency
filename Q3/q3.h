#ifndef CONCURRENCY_Q3_H
#define CONCURRENCY_Q3_H

#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include "stdbool.h"
#include "time.h"
#include "string.h"
#include "semaphore.h"
#include "errno.h"

#define BLACK   "\e[1;90m"
#define RED     "\e[1;91m"
#define GREEN   "\e[1;32m"
#define YELLOW  "\e[0;93m"
#define BLUE    "\e[1;34m"
#define MAGENTA "\e[1;95m"
#define CYAN    "\e[1;96m"
#define WHITE   "\e[1;97m"
#define RESET   "\e[0;0m"

#define ACS 0
#define ELC 1
#define ANY 2
#define SNG 3

#define PERFORMER_LIMIT 400
#define STAGE_LIMIT 1000

typedef struct player {
    int type, id, st_type, state; // 0 - waiting, 1 - performing , 2 - exited
    char name[50], ins;
    int arrivalTime, perf_time;
    int partner_id, stage_id;
    bool gotStage, solo_sing;
    pthread_t tid;
    pthread_mutex_t com_mutex, sel_mutex;
    sem_t par_sem, sing_sem;
} Player;

typedef struct stage {
    int type, state;
    pthread_mutex_t mutex;
} Stage;

int no_ac, no_el, no_player, no_cord, no_stage;
int min_time, max_time, common_waiting_time;

sem_t acoustic, electric, partnership, coordinators;

Player players[PERFORMER_LIMIT];
Stage stages[STAGE_LIMIT];

#endif //CONCURRENCY_Q3_H

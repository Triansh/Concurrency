#ifndef CONCURRENCY_Q2_H
#define CONCURRENCY_Q2_H

#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include "stdbool.h"
#include "time.h"
#include "string.h"

#define BLACK "\e[1;90m"
#define RED "\e[1;91m"
#define GREEN "\e[1;32m"
#define YELLOW "\e[0;93m"
#define BLUE "\e[0;94m"
#define MAGENTA "\e[0;95m"
#define CYAN "\e[1;96m"
#define WHITE "\e[1;97m"
#define RESET "\e[0;0m"

#define INPUT_LIMIT 250
#define STUDENT_ARRIVAL_TIME 20

typedef struct company {
    int id, total, batchSize;
    int batch[5];
    float pr;
    bool inProduction;
    pthread_t tid;
    pthread_mutex_t mutex_com;
} Company;

typedef struct student {
    int id, vaccinated, arrivalTime;
    bool isDone;
    pthread_t tid;
} Student;

typedef struct zone {
    int id, vaccines, slots, company_id;
    int students_id[8];
    bool is_allotted;
    pthread_t tid;
    pthread_mutex_t mutex_stu;
} Zone;

int n, m, o;
Company companies[INPUT_LIMIT];
Student students[INPUT_LIMIT];
Zone zones[INPUT_LIMIT];

pthread_mutex_t wait_stu;
int waiting_students;

#endif //CONCURRENCY_Q2_H

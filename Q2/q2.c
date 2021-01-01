#include "q2.h"

int getRandomInRange(int l, int r) {
    return l + (rand() % (r - l + 1));
}

int min(int a, int b) {
    return a > b ? b : a;
}

void createVaccines(Company *c) {

    printf(CYAN"Pharmaceutical Company %d is preparing %d batches of vaccines which "
           "have success probability %.2f\n" RESET, c->id, c->batchSize, c->pr);
    int w = getRandomInRange(2, 5);
    pthread_mutex_lock(&c->mutex_com);
    c->inProduction = true;
    sleep(w);
    c->batchSize = getRandomInRange(1, 5);
    for (int i = 0; i < c->batchSize; i++) {
        c->batch[i] = getRandomInRange(10, 20);
        c->total += c->batch[i];
    }
    printf(CYAN"Pharmaceutical Company %d has prepared %d batches of vaccines which have success probability %.2f."
           " Waiting for all the vaccines to be used to resume production.\n" RESET, c->id, c->batchSize, c->pr);
    c->inProduction = false;
    pthread_mutex_unlock(&c->mutex_com);

}

void *initiateProduction(void *p) {

    Company *c = (Company *) p;

    while (true) { //

        createVaccines(c);
        while (c->total != 0); //

        if (c->total == 0) {
            printf(CYAN"All the vaccines prepared by Pharmaceutical Company %d are emptied."
                   " Resuming production now.\n"RESET, c->id);
        }
        c->batchSize = 0;
        c->total = 0;
        for (int i = 0; i < 5; i++) {
            c->batch[i] = 0;
        }
        sleep(1);
    }

    return NULL;
}


int performVaccination(Zone *z, float pr, int slots) {

    int total = 0;
    while (z->students_id[total] != -1 && total < 8) total++;

    printf(CYAN"A total of %d students arrived in Zone %d with %d slots\n" RESET, total, z->id, slots);
    for (int i = 0; i < total; i++) {

        int id = z->students_id[i];
        int suc = getRandomInRange(0, 99);
        students[id].vaccinated++;
        if (suc < (int) (pr * 100)) {
            students[id].isDone = true;
        }
        printf(CYAN"Student %d on Vaccination Zone %d has been vaccinated"
               " which has success probability %.2lf\n" RESET, id, z->id, pr);
    }
    return total;

}

void getVaccines(Zone *z) {

    while (true) { //
        for (int i = 0; i < n; i++) {
            pthread_mutex_lock(&(companies[i].mutex_com));
            if (companies[i].batchSize) {
                for (int j = 0; j < 5; j++) {
                    if (companies[i].batch[j] > 0) {
                        z->company_id = companies[i].id;
                        z->vaccines = companies[i].batch[j];
                        companies[i].batch[j] = 0;
                        z->is_allotted = true;
                        companies[i].batchSize--;
                        printf(GREEN"Pharmaceutical Company %d is delivering a vaccine batch to Vaccination Zone %d which has"
                               " success probability %.2lf\n" RESET, companies[i].id, z->id, companies[i].pr);
                        pthread_mutex_unlock(&(companies[i].mutex_com));
                        return;
                    }
                }
            } else {
                pthread_mutex_unlock(&(companies[i].mutex_com));
            }
        }
    }

}

void startVaccination(Zone *z, int vaccines) {

    while (vaccines > 0) { ///
        sleep(1);

        int slots = 0;
        while (slots <= 0) { //
            slots = min(waiting_students, min(vaccines, getRandomInRange(1, 8)));
        }

        printf(GREEN"Vaccination Zone %d is ready to vaccinate with %d slots\n" RESET, z->id, slots);

        pthread_mutex_lock(&z->mutex_stu);
        z->slots = slots;
        pthread_mutex_unlock(&z->mutex_stu);

        sleep(1);
        while (z->students_id[0] == -1);

        z->slots = 0;

        printf(MAGENTA"Vaccination Zone %d entering Vaccination Phase\n" RESET, z->id);
        int nos = performVaccination(z, companies[z->company_id].pr, slots);
        printf(MAGENTA"Vaccination Zone %d just finished its Vaccination Phase\n" RESET, z->id);

        vaccines -= nos;
        printf(MAGENTA"A Total of %d vaccines are left in Vaccination Zone %d\n" RESET, vaccines, z->id);
        for (int i = 0; i < 8; i++) {
            z->students_id[i] = -1;
        }
    }

    if (!vaccines) {
        printf(RED "Vaccination Zone %d has run out of vaccines\n" RESET, z->id);
    }
}


void *enterVaccination(void *p) {

    Zone *z = (Zone *) p;

    while (true) { ////

        printf(YELLOW "Vaccination Zone %d is waiting for getting vaccines\n" RESET, z->id);

        if (!z->is_allotted) {
            getVaccines(z);
        }

        sleep(3);
        printf(GREEN"Pharmaceutical Company %d has delivered %d vaccines to Vaccination zone %d,"
               " resuming vaccinations now\n" RESET, z->company_id, z->vaccines, z->id);

        startVaccination(z, z->vaccines);

//        pthread_mutex_lock(&companies[z->company_id]->mutex_com);
        companies[z->company_id].total -= z->vaccines;
//        pthread_mutex_unlock(&companies[z->company_id]->mutex_com);

        z->company_id = -1;
        z->vaccines = 0;
        z->is_allotted = false;
    }

    return NULL;
}


void assignSlot(Student *s, Zone *z) {

    z->slots--;
    int j = 0;
    while (j < 8 && z->students_id[j] != -1) j++;
    z->students_id[j] = s->id;
    printf(YELLOW"Student %d is assigned a slot on the Vaccination Zone %d and waiting"
           " to be vaccinated.\n"RESET, s->id, z->id);
}

void waitingForVaccination(Student *s) {

    while (s->vaccinated < 3 && (!s->isDone)) {

        for (int i = 0; i < m; i++) {

            pthread_mutex_lock(&(zones[i].mutex_stu));
            if (zones[i].slots > 0) {
                assignSlot(s, &zones[i]);
                pthread_mutex_unlock(&(zones[i].mutex_stu));

            } else {
                pthread_mutex_unlock(&(zones[i].mutex_stu));
                continue;
            }

            sleep(3);

            s->isDone ? printf(GREEN "Student %d has tested positive for antibodies.\n"RESET, s->id) : printf(
                    RED "Student %d has tested negative for antibodies.\n"RESET, s->id);

            if (s->vaccinated < 3 && (!s->isDone)) {
                printf(BLUE"Student %d has arrived for his %d round of Vaccination\n"RESET, s->id,
                       s->vaccinated + 1);
                printf(BLUE"Student %d is waiting to be allocated a slot on a Vaccination Zone\n"RESET, s->id);
            }
            break;

        }
        if (s->isDone) {
            break;
        }
    }

    if (s->isDone) {
        pthread_mutex_lock(&wait_stu);
        waiting_students--;
        pthread_mutex_unlock(&wait_stu);

        printf(GREEN "Student %d went to college\n" RESET, s->id);
        return;
    }

    pthread_mutex_lock(&wait_stu);
    waiting_students--;
    pthread_mutex_unlock(&wait_stu);

    printf(RED"Student %d went back to home due to 3 negative tests\n"RESET, s->id);

}

void *sendStudentsArrival(void *p) {

    Student *s = (Student *) p;
    sleep(s->arrivalTime);

    pthread_mutex_lock(&wait_stu);
    waiting_students++;
    pthread_mutex_unlock(&wait_stu);

    printf(BLUE"Student %d has arrived for his %d round of Vaccination\n"RESET, s->id, s->vaccinated + 1);
    printf(BLUE"Student %d is waiting to be allocated a slot on a Vaccination Zone\n"RESET, s->id);
    waitingForVaccination(s);

    return NULL;
}

void init_company(Company *c, int id, float pr) {
    c->id = id;
    c->total = c->batchSize = 0;
    c->pr = pr;
    c->inProduction = false;
    memset(c->batch, 0, sizeof(c->batch));
    pthread_mutex_init(&(c->mutex_com), NULL);
    pthread_create(&(c->tid), NULL, initiateProduction, (void *) c);
}

void init_zone(Zone *z, int id) {
    z->id = id;
    z->slots = z->vaccines = 0;
    z->company_id = -1;
    z->is_allotted = false;
    memset(z->students_id, -1, sizeof(z->students_id));
    pthread_mutex_init(&(z->mutex_stu), NULL);
    pthread_create(&(z->tid), NULL, enterVaccination, (void *) z);
}

void init_student(Student *s, int id) {
    s->id = id;
    s->vaccinated = 0;
    s->isDone = false;
    s->arrivalTime = getRandomInRange(1, STUDENT_ARRIVAL_TIME);
    pthread_create(&(s->tid), NULL, sendStudentsArrival, (void *) s); // insert calling function
}

void startSimulation() {

    waiting_students = 0;

    srand(time(0));
    scanf("%d %d %d", &n, &m, &o);
    float pr[n];
    for (int i = 0; i < n; i++) {
        scanf("%f", pr + i);
    }

    if (!(n < INPUT_LIMIT && m < INPUT_LIMIT && o < INPUT_LIMIT)) {
        printf(WHITE "Please restrict your input to %d companies, zones and students.\n" RESET, INPUT_LIMIT);
        exit(0);
    }

    if (n == 0 || m == 0) {
        printf(RED"Students cannot be vaccinated.\n" RED);
        exit(0);
    }

    printf(WHITE"Simulation has started\n" RESET);
    pthread_mutex_init(&wait_stu, NULL);

    if (o == 0) return;

    for (int i = 0; i < n; i++) {
        init_company(&companies[i], i, pr[i]);
    }
    for (int i = 0; i < m; i++) {
        init_zone(&zones[i], i);
    }
    for (int i = 0; i < o; i++) {
        init_student(&students[i], i);
    }
}

void endSimulation() {

    pthread_mutex_destroy(&wait_stu);

    printf(WHITE"Simulation Over\n" RESET);
}


signed main() {

    startSimulation();
    for (int i = 0; i < o; i++) {
        pthread_join(students[i].tid, NULL);
    }
    endSimulation();

    return 0;

}
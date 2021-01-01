# Covid-19 Vaccination

## Overview 

All pharmaceutical companies start their production initially and distribute their batches to vaccination
zones. Once the zones receive vaccine batch, they begin providing slots to students. Students arrive at colleges at
random times between 1 to 20 seconds and they wait for getting vaccinated. Each vaccine has a probability of administering antibodies to the student, which depends upon the company that manufactured those vaccines. The student goes for an antibody test if he/she has been administered a vaccine. If the test results are positive, he goes to college, whereas if test results are negative, he/she goes again to get vaccines. A student is sent home if he/she receives three negative antibody tests. Companies only resume their production if all the vaccines prepared are delivered and exhausted in vaccination.

## Implementation

### Production 
* Produces batches and vaccines in it randomly.
* Sleeps for a random amount of time for production.
```c
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
```
### Delivering Vaccines
* Zone chooses one of the companies and locks the corresponding mutex of company to prevent 2 or more zones to
collect the same batch.
* If company has a batch of vaccines left, then it delivers it to the corresponding zone.
* Delivering vaccines takes some time.
```c
for (int i = 0; i < n; i++) {
    pthread_mutex_lock(&(companies[i].mutex_com));
    if (companies[i].batchSize) {
        for (int j = 0; j < 5; j++) {
            if (companies[i].batch[j] > 0) {
                ...
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
```
### Creating Slots
* After delivery of vaccines, zone makes a random no. of slots.
* Slots are  made on the basis of waiting students and vaccines left in the zone.
* Zone waits for some time for the slots to get filled.
```c
slots = min(waiting_students, min(vaccines, getRandomInRange(1, 8)));
printf(GREEN"Vaccination Zone %d is ready to vaccinate with %d slots\n" RESET, z->id, slots);
pthread_mutex_lock(&z->mutex_stu);
z->slots = slots;
pthread_mutex_unlock(&z->mutex_stu);
sleep(1);
``` 

### Students choosing slots
* A student locks the mutex of zone to prevent simultaneous arrival of 2 or more students in a zone.
* Each time a student chooses a slot, one slot from that zone is decreased.
* The outer loop checks that the student should not be vaccinated more than three times.
```c
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
        ...
    }
}
```
### Vaccination
* The procedure chooses a random integer in [0,99] and if the percentage of successful administration of vaccine is
greater than the chosen number, then the vaccination was successful otherwise unsuccessful.
* This occurs for each student present in the vaccination phase. The zone keeps the list of students present in the
current phase in a list named as `students_id`
```c
int id = z->students_id[i];
int suc = getRandomInRange(0, 99);
students[id].vaccinated++;
if (suc < (int) (pr * 100)) {
    students[id].isDone = true;
}
```
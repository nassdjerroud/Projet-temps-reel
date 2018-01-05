#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

struct Train{
    pthread_t thread;
    pthread_cond_t granted;

    int train_number;
    int loading_time;
    int crossing_time;

    int priority;
    char direction;
    char state;

}*newTrain;

struct Train *trains[3];

pthread_mutex_t track       = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  dispatcher  = PTHREAD_COND_INITIALIZER;

char *loading = "L";
char *ready = "R";
char *granted_t = "T";
char *gone = "G";
char *acknowledged_gone = "X";

void *dispatcher_function(void *train_count) {
    int count = *((int *) train_count);
    int trains_remaining = count; 

    /* Check for ready trains until all trains have left the station */
    while (trains_remaining > 0) {
        pthread_mutex_lock(&track);

        int t_granted = 0;
        int next = -1;

        for(int i = 0; i < count; i++){
            if (strcmp(&trains[i]->state, "T") == 0)
                t_granted = 1;
            if (strcmp(&trains[i]->state, "R") == 0)
                next = i;
            if (strcmp(&trains[i]->state, "G") == 0){
                trains_remaining--;
                trains[i]->state = *acknowledged_gone;
            }
        }

        /* Train was granted access to station wait for it to leave */
        if (t_granted) {
            pthread_cond_wait(&dispatcher, &track);
        }

        /* No trains in station. Wait for train */
        if (next == -1) {
            pthread_cond_wait(&dispatcher, &track);
        }

        /* Train ready in station grant next train track permission*/
        else{
            trains[next] -> state = *granted_t;
            pthread_cond_signal(&(trains[next] -> granted));
        }
        pthread_mutex_unlock(&track);
    }
    pthread_exit(0);
}

void *train_function(void* train) {
    struct Train *self = (struct Train*)train;

    /* Each train has its own cond var */
    pthread_cond_init(&self->granted, NULL);

    /* Load train */
    usleep(self -> loading_time);

    /* Lock track */
    pthread_mutex_lock(&track);

    /* Train ready */
    self -> state = *ready;
    printf("Train %d is ready to go %c\n", self -> train_number, self -> direction);

    /* Signal dispatcher */
    pthread_cond_signal(&dispatcher);

    while(strcmp(&self->state, "T") != 0)
        pthread_cond_wait(&(self->granted), &track);

    /* Use the track */
    printf("Train %d is ON the main track going %c\n", self -> train_number, self -> direction);
    usleep(self -> crossing_time);
    self -> state = *gone;
    printf("Train %d is OFF the main track after going %c\n", self -> train_number, self -> direction);

    pthread_cond_signal(&dispatcher);
    pthread_mutex_unlock(&track);

    pthread_exit(0);
}

int main() {

    FILE *ptr_file;
    char buff[10];
    int train_count = 0;
    char *train;
    char line[15];
    pthread_t train_threads[3];
    pthread_t dispatcher_thread;

    ptr_file = fopen("./trains.txt", "r");
    if (!ptr_file) 
    {
        perror("fopen for trains.txt failed");
        exit(EXIT_FAILURE);
    }

    /* Create train for each line of file */
    while (fgets(buff,10, ptr_file)!=NULL) {
        train = (char*)malloc(10 * sizeof(char));

        /* Build train string */
      sprintf(line, "%d:", train_count);
        strcat(line, buff);
        strcpy(train, line);

        /* Parse train information */
        int line_number = atoi(strtok(train, ":,"));
        char *direction = strtok(NULL,":,");
        int loading_time = atoi(strtok(NULL, ":,"));
        int crossing_time = atoi(strtok(NULL, ":,"));

        /* Create trains */
        newTrain = (struct Train *) malloc(sizeof(struct Train));
        newTrain -> train_number = line_number;
        newTrain -> crossing_time = crossing_time;
        newTrain -> loading_time = loading_time;
        newTrain -> direction = *direction;
        newTrain -> state = *loading;

        if(pthread_create(&train_threads[train_count], NULL, &train_function, (void *) newTrain))
        {
            perror("pthread create failed");
            exit(EXIT_FAILURE);
        }

        trains[line_number] = newTrain;
        train_count++;
    }
    fclose(ptr_file);

    /* Create dispatcher */
    if(pthread_create(&dispatcher_thread, NULL, &dispatcher_function, (void *) &train_count))
    {
        perror("pthread create failed");
        exit(EXIT_FAILURE);
    }

    /* Wait for dispatcher to finish */
    pthread_join(dispatcher_thread, NULL);
    printf("all done");

    free(train);
    for (int i = 0; i < train_count; i++) {
        free(trains[i]);
    }}
 

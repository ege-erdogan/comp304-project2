#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#include "pthread_sleep.c"

#include "queue.h"

#define RANDOM_SEED 42

struct Queue *landing_queue;
struct Queue *departing_queue;

int next_id = 1;
int total_sim_time;
double p;
time_t start_time;

// to prevent race conditions on landing & departing queues
pthread_mutex_t mutex_landing;
pthread_mutex_t mutex_departing;

pthread_mutex_t landing_available_mutex;
pthread_mutex_t departing_available_mutex;
pthread_cond_t landing_available;
pthread_cond_t departing_departing_available;

void *landing(void *thread_id) {
  struct Plane *plane = malloc(sizeof(*plane));
  pthread_cond_init(&(plane->available), NULL);
  pthread_mutex_init(&(plane->mutex), NULL);

  pthread_mutex_lock(&mutex_landing);
  plane->id = next_id++;
  printf("%d\tQUEUED %d\n", time(NULL) % start_time, plane->id);
  bool result = push(landing_queue, plane);
  if (result == false) {
    printf("Couldn't add plane to landing queue. Queue was full.\n");
  }
  if (plane->id == 1) {
    // this was the first plane
    pthread_mutex_lock(&landing_available_mutex);
    pthread_cond_signal(&landing_available);
    pthread_mutex_unlock(&landing_available_mutex);
  }
  pthread_mutex_unlock(&mutex_landing);

  pthread_mutex_lock(&(plane->mutex));
  pthread_cond_wait(&(plane->available), &(plane->mutex));
  // plane lands (do nothing)
  pthread_mutex_unlock(&(plane->mutex));

  printf("%d\t\t\tLANDED %d\n", time(NULL) % start_time, plane->id);
  pthread_exit(0);
}

void *departing(void *thread_id) {
  // enqueue the plane to departing queue
  // notify atc if this is the first plane
  // wait for signal from the atc
  // exit
}

void *traffic_control(void *thread_id) {
  // wait for signal from first plane
  pthread_mutex_lock(&landing_available_mutex);
  pthread_cond_wait(&landing_available, &landing_available_mutex);

  time_t current_time = time(NULL);
  while (current_time < start_time + total_sim_time) {
    pthread_mutex_lock(&mutex_landing);
    if (landing_queue->size > 0) {
      struct Plane *plane = pop(landing_queue);
      pthread_mutex_lock(&(plane->mutex));
      pthread_cond_signal(&(plane->available));
      pthread_mutex_unlock(&(plane->mutex));
      printf("%d\t\tAPPROVED %d\n", time(NULL) % start_time, plane->id);
      pthread_sleep(2);
    } else {
      // depart first plane in departing queue
    }
    pthread_mutex_unlock(&mutex_landing);

    //    pthread_sleep(1);
    current_time = time(NULL);
  }
  pthread_mutex_unlock(&landing_available_mutex);

  pthread_exit(0);
}

int main (int argc, char *argv[]) {

  // initialize variables
  landing_queue = malloc(sizeof(*landing_queue));
  departing_queue = malloc(sizeof(*departing_queue));
  init(landing_queue);
  init(departing_queue);

  pthread_mutex_init(&landing_available_mutex, NULL);
  pthread_mutex_init(&departing_available_mutex, NULL);
  pthread_mutex_init(&mutex_landing, NULL);
  pthread_mutex_init(&mutex_departing, NULL);

  // can perform more robust argument checking
  if (argc != 5) {
    printf("Wrong arguements. Exiting.\n");
    return 1;
  } else {
    if (strcmp(argv[1], "-s") == 0)
      total_sim_time = atoi(argv[2]);
    if (strcmp(argv[1], "-p") == 0)
      p = atof(argv[2]);
    if (strcmp(argv[3], "-s") == 0)
      total_sim_time = atoi(argv[4]);
    if (strcmp(argv[3], "-p") == 0)
      p = atof(argv[4]);
  }

  start_time = time(NULL);

  // create ATC thread
  pthread_t atc_tid;
  pthread_create(&atc_tid, NULL, traffic_control, argv[1]);

  // create landing plane thread
  pthread_t landing_id;
  pthread_create(&landing_id, NULL, landing, argv[1]);

  // create departing plane thread
  pthread_t departing_id;
  pthread_create(&departing_id, NULL, departing, argv[1]);

  srand(RANDOM_SEED);
  time_t current_time = time(NULL);

  while (current_time < start_time + total_sim_time) {
    double random = (rand() / (double) RAND_MAX);
    if (random <= p) {
      // landing plane arrives
      pthread_t landing_id;
      pthread_create(&landing_id, NULL, landing, argv[1]);
    }
    if (random <= 1 - p) {
      // departing plane arrives
      //pthread_t departing_id;
      //pthread_create(&departing_id, NULL, departing, argv[1]);
    }
    pthread_sleep(1);
    current_time = time(NULL);
  }

  return 0;
}

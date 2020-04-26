#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#include "pthread_sleep.c"

#include "queue.h"
#include "logging.h"

#define RANDOM_SEED 42

struct Queue *landing_queue;
struct Queue *departing_queue;

int next_landing_id = 2;
int next_departing_id = 1;
int total_sim_time;
int snapshot_time;
double p;
time_t start_time;

// to prevent race conditions on landing & departing queues
pthread_mutex_t mutex_landing;
pthread_mutex_t mutex_departing;

pthread_mutex_t landing_available_mutex;
pthread_mutex_t departing_available_mutex;
pthread_cond_t landing_available;
pthread_cond_t departing_available;

// helper methods

void permit_plane(struct Plane *plane) {
  pthread_mutex_lock(&(plane->mutex));
  pthread_cond_signal(&(plane->available));
  pthread_mutex_unlock(&(plane->mutex));
}

int seconds() {
  return time(NULL) % start_time;
}

void *landing(void *thread_id) {
  struct Plane *plane = malloc(sizeof(*plane));
  pthread_cond_init(&(plane->available), NULL);
  pthread_mutex_init(&(plane->mutex), NULL);

  pthread_mutex_lock(&mutex_landing);
  plane->id = next_landing_id;
  next_landing_id += 2;
  plane->status = 'L';
  plane->request_time = seconds();

  log_plane_arrival(plane);

  bool result = push(landing_queue, plane);
  if (result == false) {
    printf("Couldn't add plane to landing queue. Queue was full.\n");
  }
  if (plane->id == 2) {
    // this was the first plane
    pthread_mutex_lock(&landing_available_mutex);
    pthread_cond_signal(&landing_available);
    pthread_mutex_unlock(&landing_available_mutex);
  }
  pthread_mutex_unlock(&mutex_landing);

  pthread_mutex_lock(&(plane->mutex));
  pthread_cond_wait(&(plane->available), &(plane->mutex));
  // plane lands
  plane->completed_time = seconds();
  log_plane_approval(plane);
  pthread_mutex_unlock(&(plane->mutex));

  pthread_exit(0);
}

void *departing(void *thread_id) {
  struct Plane *plane = malloc(sizeof(*plane));
  pthread_cond_init(&(plane->available), NULL);
  pthread_mutex_init(&(plane->mutex), NULL);

  pthread_mutex_lock(&mutex_departing);
  plane->id = next_departing_id;
  next_departing_id += 2;
  plane->status = 'D';
  plane->request_time = seconds();

  log_plane_arrival(plane);

  bool result = push(departing_queue, plane);
  if (!result) {
    printf("Couldn't push plane to departing queue. Queue was full.\n");
  }
  if (plane->id == 1) {
    //  this was the first plane
    pthread_mutex_lock(&departing_available_mutex);
    pthread_cond_signal(&departing_available);
    pthread_mutex_unlock(&departing_available_mutex);
  }
  pthread_mutex_unlock(&mutex_departing);

  pthread_mutex_lock(&(plane->mutex));
  pthread_cond_wait(&(plane->available), &(plane->mutex));
  // plane departs
  plane->completed_time = seconds();
  log_plane_approval(plane);
  pthread_mutex_unlock(&(plane->mutex));

  pthread_exit(0);
}

void *traffic_control(void *thread_id) {
  // wait for signal from first plane
  pthread_mutex_lock(&landing_available_mutex);
  pthread_cond_wait(&landing_available, &landing_available_mutex);

  while (seconds() < total_sim_time) {
    if (landing_queue->size > 0 && departing_queue->size < 5) {
      struct Plane *plane = pop(landing_queue);
      permit_plane(plane);
      pthread_sleep(2);
    } else if (departing_queue->size > 0){
      struct Plane *plane = pop(departing_queue);
      permit_plane(plane);
      pthread_sleep(2);
    }
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
  init_logs();

  pthread_mutex_init(&landing_available_mutex, NULL);
  pthread_mutex_init(&departing_available_mutex, NULL);
  pthread_mutex_init(&mutex_landing, NULL);
  pthread_mutex_init(&mutex_departing, NULL);

  // can perform more robust argument checking
  if (argc != 7) {
    printf("Wrong arguements. Exiting.\n");
    return 1;
  } else {
    for (int i = 1; i < argc; i += 2) {
      if (strcmp(argv[i], "-s") == 0) {
        total_sim_time = atoi(argv[i + 1]);
      } else if (strcmp(argv[i], "-p") == 0) {
        p = atof(argv[i + 1]);
      } else if (strcmp(argv[i], "-n") == 0) {
        snapshot_time = atoi(argv[i + 1]);
      }
    }
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

  while (seconds() < total_sim_time) {
    double random = (rand() / (double) RAND_MAX);
    if (random <= p) {
      // landing plane arrives
      pthread_t landing_id;
      pthread_create(&landing_id, NULL, landing, argv[1]);
    }
    if (random <= 1 - p) {
      // departing plane arrives
      pthread_t departing_id;
      pthread_create(&departing_id, NULL, departing, argv[1]);
    }

    // display snapshot if time > n
    if (seconds() > snapshot_time) {
      printf("[%d] GROUND: ", seconds());
      print_queue(departing_queue);
      printf("\n");
      printf("[%d] ON AIR: ", seconds());
      print_queue(landing_queue);
      printf("\n\n");
    }

    pthread_sleep(1);
    current_time = time(NULL);
  }

  return 0;
}

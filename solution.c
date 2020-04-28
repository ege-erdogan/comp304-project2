#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#include "pthread_sleep.c"

#include "queue.h"
#include "logging.h"

#define RANDOM_SEED         42
#define MAX_CONSEC_DEPARTS  2

struct Queue *landing_queue;
struct Queue *departing_queue;
struct Queue *emergency_queue;

int next_landing_id = 2;
int next_departing_id = 1;
int total_sim_time;
int snapshot_time;
double p;
time_t start_time;

// each queue has a lock to prevent race conditions on that queue
pthread_mutex_t mutex_landing;
pthread_mutex_t mutex_departing;
pthread_mutex_t mutex_emergency;

// mutex, cond. variable pair for the first planes to signal the ATC.
pthread_mutex_t landing_available_mutex;
pthread_mutex_t departing_available_mutex;
pthread_cond_t landing_available;
pthread_cond_t departing_available;

// helper methods to avoid repetitive code

// returns no. of seconds since start
int seconds() {
  return time(NULL) % start_time;
}

// used by ATC to signal a plane to land/depart
void permit_plane(struct Plane *plane) {
  pthread_mutex_lock(&(plane->mutex));
  pthread_cond_signal(&(plane->available));
  pthread_mutex_unlock(&(plane->mutex));
}

// called by planes to wait
void wait_for_signal(struct Plane *plane) {
  pthread_mutex_lock(&(plane->mutex));
  pthread_cond_wait(&(plane->available), &(plane->mutex));
  plane->completed_time = seconds();
  log_plane_approval(plane);
  pthread_mutex_unlock(&(plane->mutex));
}

// emergency attribute is true if emergency landing
void *landing(void *emergency) {
  struct Plane *plane = malloc(sizeof(*plane));
  pthread_cond_init(&(plane->available), NULL);
  pthread_mutex_init(&(plane->mutex), NULL);

  bool emergency_landing = (bool*) emergency;
  if (emergency_landing) {
    pthread_mutex_lock(&mutex_emergency);
    plane->id = next_landing_id;
    next_landing_id += 2;
    plane->status = 'E';
    plane->request_time = seconds();

    log_plane_arrival(plane);
    push(emergency_queue, plane);

    pthread_mutex_unlock(&mutex_emergency);
  } else { // regular landing
    pthread_mutex_lock(&mutex_landing);
    plane->id = next_landing_id;
    next_landing_id += 2;
    plane->status = 'L';
    plane->request_time = seconds();

    log_plane_arrival(plane);
    push(landing_queue, plane);

    if (plane->id == 2) {
      // this was the first plane. should signal the atc
      pthread_mutex_lock(&landing_available_mutex);
      pthread_cond_signal(&landing_available);
      pthread_mutex_unlock(&landing_available_mutex);
    }

    pthread_mutex_unlock(&mutex_landing);
  }

  wait_for_signal(plane);
  pthread_exit(0);
}

void *departing() {
  struct Plane *plane = malloc(sizeof(*plane));
  pthread_cond_init(&(plane->available), NULL);
  pthread_mutex_init(&(plane->mutex), NULL);

  // acquire the lock to modify the departing queue
  pthread_mutex_lock(&mutex_departing);
  plane->id = next_departing_id;
  next_departing_id += 2;
  plane->status = 'D';
  plane->request_time = seconds();

  log_plane_arrival(plane);
  push(departing_queue, plane);

  if (plane->id == 1) {
    //  this was the first plane. signal the ATC.
    pthread_mutex_lock(&departing_available_mutex);
    pthread_cond_signal(&departing_available);
    pthread_mutex_unlock(&departing_available_mutex);
  }

  // release the depating queue lock
  pthread_mutex_unlock(&mutex_departing);

  wait_for_signal(plane);
  pthread_exit(0);
}

void *traffic_control() {
  // wait for signal from first plane
  pthread_mutex_lock(&landing_available_mutex);
  pthread_cond_wait(&landing_available, &landing_available_mutex);

  // local variable to keep track of consecutive departing planes
  int departs = 0;

  while (seconds() < total_sim_time) {
    if (emergency_queue->size > 0) {
      // if there is an emergency flight, allow it to land.
      struct Plane *plane = pop(emergency_queue);
      departs = 0;
      permit_plane(plane);
      pthread_sleep(2);
    } else if (landing_queue->size > 0 && (departs == MAX_CONSEC_DEPARTS || departing_queue->size < 5)) {
      // allow landing plane when a certain number of consecutive departures has been reached, or there are < 5 planes in departing queue.
      struct Plane *plane = pop(landing_queue);
      departs = 0;
      permit_plane(plane);
      pthread_sleep(2);
    } else if (departing_queue->size > 0) {
      // allow departing planes if above conditions don't hold and queue is not empty.
      struct Plane *plane = pop(departing_queue);
      permit_plane(plane);
      departs++;
      pthread_sleep(2);
    }
  }
  pthread_mutex_unlock(&landing_available_mutex);

  pthread_exit(0);
}

int main (int argc, char *argv[]) {

  // parse arguments
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

  // initialize queues & logging
  landing_queue = malloc(sizeof(*landing_queue));
  departing_queue = malloc(sizeof(*departing_queue));
  emergency_queue = malloc(sizeof(*emergency_queue));
  init(landing_queue);
  init(departing_queue);
  init(emergency_queue);
  init_logs();

  // initialize mutex and cond. variables
  pthread_mutex_init(&landing_available_mutex, NULL);
  pthread_mutex_init(&departing_available_mutex, NULL);
  pthread_mutex_init(&mutex_landing, NULL);
  pthread_mutex_init(&mutex_departing, NULL);
  pthread_mutex_init(&mutex_emergency, NULL);

  start_time = time(NULL);

  // create ATC thread
  pthread_t atc_tid;
  pthread_create(&atc_tid, NULL, traffic_control, NULL);

  // create first landing plane thread
  pthread_t landing_id;
  bool emergency = false;
  pthread_create(&landing_id, NULL, landing, (void *) emergency);

  // create first departing plane thread
  pthread_t departing_id;
  pthread_create(&departing_id, NULL, departing, NULL);

  srand(RANDOM_SEED);

  while (seconds() < total_sim_time) {
    double random = (rand() / (double) RAND_MAX);
    if (random <= p) {
      // landing plane arrives
      pthread_t landing_id;
      bool emergency = false;
      pthread_create(&landing_id, NULL, landing, (void *) emergency);
    }
    if (random <= 1 - p) {
      // departing plane arrives
      pthread_t departing_id;
      pthread_create(&departing_id, NULL, departing, NULL);
    }
    if (seconds() % 40 == 0 && seconds() != 0) {
      // emergency landing plane arrives
      pthread_t emergency_id;
      bool emergency = true;
      pthread_create(&emergency_id, NULL, landing, (void *) emergency);
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
  }

  return 0;
}

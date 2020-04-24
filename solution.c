#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <time.h>

#define RANDOM_SEED 42

struct Plane {
  pthread_t id;
  // arrival time, lock condOD
};

struct Queue {
  // capacity, size, front, rear, element_array
};

void *landing(void *thread_id) {
  printf("Created landing plane thread.\n");
}

void *departing(void *thread_id) {
  printf("Created departing plane thread.\n");
}

void *traffic_control(void *thread_id) {
  printf("Created ATC thread.\n");
}

int main (int argc, char *argv[]) {

  int total_sim_time;
  float p;

  // can perform more robust argument checking
  if (argc != 5) {
    printf("Wrong arguements. Exiting.");
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

  time_t start_time = time(NULL);

  // create ATC thread
  pthread_t atc_tid;

  pthread_create(&atc_tid, NULL, traffic_control, argv[1]);
  pthread_join(atc_tid, NULL);

  // create landing plane thread
  struct Plane landing_plane;
  pthread_create(&(landing_plane.id), NULL, landing, argv[1]);
  pthread_join(landing_plane.id, NULL);

  // create landing plane thread
  struct Plane departing_plane;
  pthread_create(&(departing_plane.id), NULL, departing, argv[1]);
  pthread_join(departing_plane.id, NULL);

  srand(RANDOM_SEED);
  time_t current_time = time(NULL);
  while (current_time < start_time + total_sim_time) {
    double random = (rand() / (double) RAND_MAX);
    printf("%f\n", random);
    if (random <= p) {
      // create landing plane thread
      printf("LANDING\n");
    }
    if (random <= 1 - p) {
      // create departing plane thread
      printf("DEPARTING\n");
    }
    sleep(1);
    current_time = time(NULL);
  }

  return 0;
}

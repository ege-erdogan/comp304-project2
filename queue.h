/*
  Ege Erdogan 64004
  COMP 304 Project 2
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define CAPACITY 1000

#ifndef QUEUE_H
#define QUEUE_H

// circular queue array implementation with fixed capacity.
struct Queue {
  int capacity;
  int size;
  int head;
  int tail;
  struct Plane *queue_array[CAPACITY];
};

struct Plane {
  int id;
  char status; // L for landing, D for departing, E for emergency
  int request_time;
  int completed_time;
  pthread_cond_t available;
  pthread_mutex_t mutex;
};

// initialize variables
void init(struct Queue *qptr) {
  qptr->capacity = CAPACITY;
  qptr->size = 0;
  qptr->head = -1;
  qptr->tail = -1;
}

bool is_full(struct Queue *qptr) {
  return qptr->size == qptr->capacity;
}

bool push(struct Queue *qptr, struct Plane *plane) {
  if (is_full(qptr)) {
    return false;
  } else {
    qptr->tail = (qptr->tail + 1) % qptr->capacity;
    if (qptr->head == -1) {
      qptr->head = 0;
    }
    qptr->queue_array[qptr->tail] = plane;
    qptr->size++;
    return true;
  }
}

struct Plane *pop(struct Queue *qptr) {
  if (qptr->head == -1) {
    return NULL;
  } else {
    struct Plane *planeptr = qptr->queue_array[qptr->head];
    if (qptr->head == qptr->tail) {
      qptr->head = -1;
      qptr->tail = -1;
    } else {
      qptr->head = (qptr->head + 1) % qptr->capacity;
    }
    qptr->size--;
    return planeptr;
  }
}

void print_queue(struct Queue *queue) {
  int size = queue->size;
  for (int i = 0; i < size; i++) {
    int index = (i + queue->head) % queue->capacity;
    printf("%d ", queue->queue_array[index]->id);
  }
}

#endif // QUEUE_H

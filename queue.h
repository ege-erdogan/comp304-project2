#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define CAPACITY 1024

struct Queue {
  int capacity;
  int size;
  int head;
  int tail;
  struct Plane *queue_array[CAPACITY];
};

struct Plane {
  int id;
  pthread_cond_t available;
  pthread_mutex_t mutex;
};

// initiate with fixed capacity of 1024
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
    qptr->size -= 1;
    return planeptr;
  }
}

struct Plane *top(struct Queue *qptr) {
  if (qptr->head == -1) {
    return NULL;
  }
  else {
    return qptr->queue_array[qptr->head];
  }
}

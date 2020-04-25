#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "plane.c"

struct Queue {
  int capacity;
  int size;
  int head;
  int tail;
  struct Plane *queue_array[];
};

struct Queue *init(int capacity) {
  struct Queue q;
  q.capacity = capacity;
  q.size = 0;
  q.head = -1;
  q.tail = -1;
  q.queue_array[capacity];
  return &q;
}

bool is_full(struct Queue *qptr) {
  return qptr->size == qptr->capacity;
}

bool push(struct Queue *qptr, struct Plane *plane) {
  if (is_full(qptr)) {
    return false;
  } else {
    qptr->tail = (qptr->tail + 1) % qptr->capacity;
    qptr->queue_array[qptr->tail] = plane;
    qptr->size++;
    return true;
  }
}

struct Plane *pop(struct Queue *qptr) {
  if (qptr->head == -1) {
    return NULL;
  } else {
    struct Plane *planeptr = &(qptr->queue_array[qptr->head]);
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
    return &(qptr->queue_array[qptr->head]);
  }
}

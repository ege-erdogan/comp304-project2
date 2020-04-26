#include <stdio.h>
#include <stdlib.h>

#include "queue.h"

void init_log_file() {
  FILE *fp = fopen("planes.log", "ab+");
  fprintf(fp, "ID\tSTATUS\tREQUEST\tRUNWAY\tTURNAROUND\n");
  fclose(fp);
}

void log_plane(struct Plane *plane) {
  FILE *fp = fopen("planes.log", "a");
  fprintf(fp, "%d\t%c\t%d\t%d\t%d\n", plane->id, plane->status, plane->request_time,
          plane->completed_time, plane->completed_time - plane->request_time);
  fclose(fp);
}

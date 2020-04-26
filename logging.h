#include <stdio.h>
#include <stdlib.h>

#include "queue.h"

void init_logs() {
  FILE *fp = fopen("planes.log", "wb");
  fprintf(fp, "DEPARTED OR LANDED PLANES SORTED BY APPROVAL TIME\n\n");
  fprintf(fp, "ID\tSTATUS\tREQUEST\tRUNWAY\tTURNAROUND\n");
  fclose(fp);

  fp = fopen("tower.log", "wb");
  fprintf(fp, "LOG OF ALL PLANES SORTED BY ARRIVAL TIME\n\n");
  fprintf(fp, "ID\tSTATUS\tREQUEST\n");
  fclose(fp);
}

void log_plane_approval(struct Plane *plane) {
  FILE *fp = fopen("planes.log", "a");
  fprintf(fp, "%d\t%c\t%d\t%d\t%d\n", plane->id, plane->status, plane->request_time,
          plane->completed_time, plane->completed_time - plane->request_time);
  fclose(fp);
}

void log_plane_arrival(struct Plane *plane) {
  FILE *fp = fopen("tower.log", "a");
  fprintf(fp, "%d\t%c\t%d\n", plane->id, plane->status, plane->request_time);
  fclose(fp);
}

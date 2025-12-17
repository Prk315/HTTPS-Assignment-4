#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>

#include "record.h"
#include "coord_query.h"

struct naive_data {
  struct record *rs;
  int n;
};

struct naive_data* mk_naive(struct record* rs, int n) {
  struct naive_data *data = malloc(sizeof(struct naive_data));
  if (!data) {
    return NULL;
  }
  data->rs = rs;
  data->n = n;
  return data;
}

void free_naive(struct naive_data* data) {
  free(data);
}

const struct record* lookup_naive(struct naive_data *data, double lon, double lat) {
  if (data->n == 0) {
    return NULL;
  }

  const struct record *closest = &data->rs[0];
  double min_dist_sq = (data->rs[0].lon - lon) * (data->rs[0].lon - lon) +
                       (data->rs[0].lat - lat) * (data->rs[0].lat - lat);

  for (int i = 1; i < data->n; i++) {
    double dx = data->rs[i].lon - lon;
    double dy = data->rs[i].lat - lat;
    double dist_sq = dx * dx + dy * dy;

    if (dist_sq < min_dist_sq) {
      min_dist_sq = dist_sq;
      closest = &data->rs[i];
    }
  }

  return closest;
}

int main(int argc, char** argv) {
  return coord_query_loop(argc, argv,
                          (mk_index_fn)mk_naive,
                          (free_index_fn)free_naive,
                          (lookup_fn)lookup_naive);
}

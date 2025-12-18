#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>

#include "record.h"
#include "id_query.h"

struct index_record {
  int64_t osm_id;
  const struct record *record;
};

struct indexed_data {
  struct index_record *irs;
  int n;
};

// Comparison function for qsort
static int compare_index_records(const void *a, const void *b) {
  const struct index_record *ia = (const struct index_record *)a;
  const struct index_record *ib = (const struct index_record *)b;

  if (ia->osm_id < ib->osm_id) {
    return -1;
  } else if (ia->osm_id > ib->osm_id) {
    return 1;
  } else {
    return 0;
  }
}

struct indexed_data* mk_indexed(struct record* rs, int n) {
  struct indexed_data *data = malloc(sizeof(struct indexed_data));
  if (!data) {
    return NULL;
  }

  data->irs = malloc((size_t)n * sizeof(struct index_record));
  if (!data->irs) {
    free(data);
    return NULL;
  }

  // Build the index
  for (int i = 0; i < n; i++) {
    data->irs[i].osm_id = rs[i].osm_id;
    data->irs[i].record = &rs[i];
  }

  data->n = n;

  // Sort the index by osm_id
  qsort(data->irs, (size_t)n, sizeof(struct index_record), compare_index_records);

  return data;
}

void free_indexed(struct indexed_data* data) {
  free(data->irs);
  free(data);
}

const struct record* lookup_indexed(struct indexed_data *data, int64_t needle) {
  int left = 0;
  int right = data->n - 1;

  while (left <= right) {
    int mid = left + (right - left) / 2;

    if (data->irs[mid].osm_id == needle) {
      return data->irs[mid].record;
    } else if (data->irs[mid].osm_id < needle) {
      left = mid + 1;
    } else {
      right = mid - 1;
    }
  }

  return NULL;
}

int main(int argc, char** argv) {
  return id_query_loop(argc, argv,
                    (mk_index_fn)mk_indexed,
                    (free_index_fn)free_indexed,
                    (lookup_fn)lookup_indexed);
}

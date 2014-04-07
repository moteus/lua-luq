#include "luq_pthread.h"

#ifndef QVOID_LENGTH
#  define QVOID_LENGTH 255
#endif

typedef struct qvoid_tag{
  pthread_cond_t   cond;
  pthread_mutex_t  mutex;
  int              count;
  void            *arr[QVOID_LENGTH];
} qvoid_t;

int qvoid_init(qvoid_t *q);

int qvoid_destroy(qvoid_t *q);

int qvoid_capacity(qvoid_t *q);

int qvoid_size(qvoid_t *q);

int qvoid_empty(qvoid_t *q);

int qvoid_full(qvoid_t *q);

int qvoid_put(qvoid_t *q, void *data);

int qvoid_put_nolock(qvoid_t *q, void *data);

int qvoid_put_timeout(qvoid_t *q, void *data, int ms);

int qvoid_lock(qvoid_t *q);

int qvoid_unlock(qvoid_t *q);

int qvoid_notify(qvoid_t *q);

int qvoid_get(qvoid_t *q, void **data);

int qvoid_get_nolock(qvoid_t *q, void **data);

int qvoid_get_timeout(qvoid_t *q, void **data, int ms);

int qvoid_clear(qvoid_t *q);


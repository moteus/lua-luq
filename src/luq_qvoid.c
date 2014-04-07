#include <assert.h>
#include <errno.h>
#include <memory.h>
#include <stdlib.h>
#include "luq_qvoid.h"

int qvoid_init(qvoid_t *q){
  pthread_condattr_t *pcondattr = NULL;

#ifdef USE_PTHREAD
  pthread_condattr_t condattr;
  pcondattr = &condattr;
#endif

  memset((void*)q, 0, sizeof(qvoid_t));

#ifdef USE_PTHREAD
  pthread_condattr_init(pcondattr);
#ifdef CLOCK_MONOTONIC_RAW
  pthread_condattr_setclock(pcondattr, CLOCK_MONOTONIC_RAW);
#elif defined CLOCK_MONOTONIC
  pthread_condattr_setclock(pcondattr, CLOCK_MONOTONIC);
#endif
#endif

  if(0 != pthread_mutex_init(&q->mutex, NULL)){
    return -1;
  }
  if(0 != pthread_cond_init(&q->cond, pcondattr)){
    pthread_mutex_destroy(&q->mutex);
    return -1;
  }

  return 0;
}

int qvoid_destroy(qvoid_t *q){
  pthread_mutex_destroy(&q->mutex);
  pthread_cond_destroy(&q->cond);
  memset((void*)q, 0, sizeof(qvoid_t));
  return 0;
}

int qvoid_capacity(qvoid_t *q){
  return QVOID_LENGTH;
}

int qvoid_size(qvoid_t *q){
  return q->count;
}

int qvoid_empty(qvoid_t *q){
  return qvoid_size(q) == 0;
}

int qvoid_full(qvoid_t *q){
  return qvoid_size(q) == qvoid_capacity(q);
}

int qvoid_put(qvoid_t *q, void *data){
  int ret = pthread_mutex_lock(&q->mutex);
  if(ret != 0) return ret;
  while(qvoid_full(q)){
    ret = pthread_cond_wait(&q->cond, &q->mutex);
    if(ret != 0){
      pthread_mutex_unlock(&q->mutex);
      return ret;
    }
  }

  q->arr[q->count++] = data;
  assert(q->count <= qvoid_capacity(q));

  PTHREAD_COND_NOTIFY(&q->cond);
  pthread_mutex_unlock(&q->mutex);
  return 0;
}

int qvoid_put_nolock(qvoid_t *q, void *data){
  if(qvoid_full(q)){
    return -1;
  }

  q->arr[q->count++] = data;
  assert(q->count <= qvoid_capacity(q));

  return 0;
}

int qvoid_put_timeout(qvoid_t *q, void *data, int ms){
  int ret = pthread_mutex_lock(&q->mutex);
  if(ret != 0) return ret;
  while(qvoid_full(q)){
    ret = pthread_cond_timedwait_timeout(&q->cond, &q->mutex, ms);
    if(ret != 0){
      pthread_mutex_unlock(&q->mutex);
      return ret;
    }
  }

  q->arr[q->count++] = data;
  assert(q->count <= qvoid_capacity(q));

  PTHREAD_COND_NOTIFY(&q->cond);
  pthread_mutex_unlock(&q->mutex);
  return 0;
}

int qvoid_lock(qvoid_t *q){
  return pthread_mutex_lock(&q->mutex);
}

int qvoid_unlock(qvoid_t *q){
  return pthread_mutex_unlock(&q->mutex);
}

int qvoid_notify(qvoid_t *q){
  return PTHREAD_COND_NOTIFY(&q->cond);
}

int qvoid_get(qvoid_t *q, void **data){
  int ret = pthread_mutex_lock(&q->mutex);
  if(ret != 0) return ret;
  while(qvoid_empty(q)){
    ret = pthread_cond_wait(&q->cond, &q->mutex);
    if(ret != 0){
      pthread_mutex_unlock(&q->mutex);
      return ret;
    }
  }

  assert(q->count > 0);
  *data = q->arr[--q->count];

  PTHREAD_COND_NOTIFY(&q->cond);
  pthread_mutex_unlock(&q->mutex);
  return 0;
}

int qvoid_get_nolock(qvoid_t *q, void **data){
  if(qvoid_empty(q)){
    *data = NULL;
    return -1;
  }

  assert(q->count > 0);
  *data = q->arr[--q->count];

  return 0;
}

int qvoid_get_timeout(qvoid_t *q, void **data, int ms){
  int ret = pthread_mutex_lock(&q->mutex);
  if(ret != 0) return ret;
  while(qvoid_empty(q)){
    ret = pthread_cond_timedwait_timeout(&q->cond, &q->mutex, ms);
    if(ret != 0){
      pthread_mutex_unlock(&q->mutex);
      return ret;
    }
  }

  assert(q->count > 0);
  *data = q->arr[--q->count];

  PTHREAD_COND_NOTIFY(&q->cond);
  pthread_mutex_unlock(&q->mutex);
  return 0;
}

int qvoid_clear(qvoid_t *q){
  int ret = pthread_mutex_lock(&q->mutex);
  if(ret != 0) return ret;
  q->count = 0;
  pthread_mutex_unlock(&q->mutex);
  return 0;
}

#include <assert.h>
#include <memory.h>
#include <stdlib.h>
#include "luq_pthread.h"

#ifdef USE_PTHREAD

#include <sys/time.h>

// CLOCK_MONOTONIC_RAW does not work on travis-ci and on LinuxMint 15
// it returns ETIMEDOUT immediately
#ifdef CLOCK_MONOTONIC_RAW
#  undef CLOCK_MONOTONIC_RAW
#endif

static int timeout_to_timespec(int timeout, struct timespec* ts){
  int sc = timeout / 1000;
  int ns = (timeout % 1000) * 1000 * 1000;
  int ret;

  if (timeout < 0) return 1;

#ifdef CLOCK_MONOTONIC_RAW
  ret = clock_gettime(CLOCK_MONOTONIC_RAW, ts);
  if(ret) return -1;
  ts->tv_sec  += sc;
  ts->tv_nsec += ns;
#elif defined CLOCK_MONOTONIC
  ret = clock_gettime(CLOCK_MONOTONIC, ts);
  if(ret) return -1;
  ts->tv_sec  += sc;
  ts->tv_nsec += ns;
#else
  struct timeval now;
  ret = gettimeofday(&now, 0);
  ts->tv_sec  = now.tv_sec + sc;         // sec
  ts->tv_nsec = now.tv_usec * 1000 + ns; // nsec
#endif

  ts->tv_sec  += ts->tv_nsec / 1000000000L;
  ts->tv_nsec %= 1000000000L;

  return 0;
}

int pthread_cond_timedwait_timeout(pthread_cond_t *cond, pthread_mutex_t *mutex, int timeout){
  struct timespec ts;
  timeout_to_timespec(timeout, &ts);

  return pthread_cond_timedwait(cond, mutex, &ts);
}

#else

int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr){
  assert(attr == NULL);

  *mutex = CreateMutex(NULL, FALSE, NULL);
  if(*mutex == NULL){
    DWORD ret = GetLastError();
    return (ret == 0)? -1 : ret;
  }
  return 0;
}

int pthread_mutex_lock(pthread_mutex_t *mutex){
  DWORD ret = WaitForSingleObject(*mutex, INFINITE);
  if((WAIT_OBJECT_0 == ret) || (WAIT_ABANDONED_0 == ret))
    return 0;
  ret = GetLastError();
  return (ret == 0)? -1 : ret;
}

int pthread_mutex_unlock(pthread_mutex_t *mutex){
  if(TRUE == ReleaseMutex(*mutex))
    return 0;
  else{
    DWORD ret = GetLastError();
    return (ret == 0)? -1 : ret;
  }
}

int pthread_mutex_destroy(pthread_mutex_t *mutex){
  CloseHandle(*mutex);
  *mutex = NULL;
  return 0;
}

int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr){
  assert(attr == NULL);

#ifdef USE_BROADCAST
  *cond = CreateEvent(NULL, TRUE, FALSE, NULL);
#else
  *cond = CreateEvent(NULL, FALSE, FALSE, NULL);
#endif

  if(*cond == NULL){
    DWORD ret = GetLastError();
    return (ret == 0)? -1 : ret;
  }
  return 0;
}

int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex){
  SignalObjectAndWait(*mutex, *cond, INFINITE, FALSE);
  return pthread_mutex_lock(mutex);
}

int pthread_cond_timedwait_timeout(pthread_cond_t *cond, pthread_mutex_t *mutex, DWORD timeout){
  DWORD ret = SignalObjectAndWait(*mutex, *cond, timeout, FALSE);
  int rc = pthread_mutex_lock(mutex);
  if(rc != 0) return rc;
  return (ret == WAIT_TIMEOUT)? ETIMEDOUT : 0;
}

#ifdef USE_BROADCAST
/* We can not support both function on same cond variable so we implement
 * only one just in case.
 */

int pthread_cond_broadcast(pthread_cond_t *cond){
  if(TRUE == PulseEvent(*cond))
    return 0;
  else{
    DWORD ret = GetLastError();
    return (ret == 0)? -1 : ret;
  }
}

#else

int pthread_cond_signal(pthread_cond_t *cond){
  if(TRUE == SetEvent(*cond))
    return 0;
  else{
    DWORD ret = GetLastError();
    return (ret == 0)? -1 : ret;
  }
}

#endif

int pthread_cond_destroy(pthread_cond_t *cond){
  CloseHandle(*cond);
  *cond = NULL;
  return 0;
}

#endif


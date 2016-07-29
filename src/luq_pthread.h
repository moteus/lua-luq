#ifndef _LUQ_PTHREAD_90B92D12_9291_43C1_B5B4_BDBCD325D99C_
#define _LUQ_PTHREAD_90B92D12_9291_43C1_B5B4_BDBCD325D99C_

#include <errno.h>

#if !defined(_WIN32) && !defined(USE_PTHREAD)
#  define USE_PTHREAD
#endif

#if !defined(USE_BROADCAST) && !defined(USE_SIGNAL)
#  define USE_SIGNAL
#endif

#if defined(USE_BROADCAST) && defined(USE_SIGNAL)
#  error You must specify ether USE_BROADCAST or USE_SIGNAL
#endif

#ifdef USE_BROADCAST
#  define PTHREAD_COND_NOTIFY pthread_cond_broadcast
#else
#  define PTHREAD_COND_NOTIFY pthread_cond_signal
#endif

#ifdef USE_PTHREAD

#include <pthread.h>

int pthread_cond_timedwait_timeout(pthread_cond_t *cond, pthread_mutex_t *mutex, int timeout);

#else

#include <windows.h>

#ifndef ETIMEDOUT
#  define ETIMEDOUT WAIT_TIMEOUT
#endif

typedef HANDLE pthread_mutex_t;
typedef HANDLE pthread_cond_t;
typedef void pthread_mutexattr_t;
typedef void pthread_condattr_t;

int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);

int pthread_mutex_lock(pthread_mutex_t *mutex);

int pthread_mutex_unlock(pthread_mutex_t *mutex);

int pthread_mutex_destroy(pthread_mutex_t *mutex);

int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr);

int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);

int pthread_cond_timedwait_timeout(pthread_cond_t *cond, pthread_mutex_t *mutex, DWORD timeout);

#ifdef USE_BROADCAST
/* We can not support both function on same cond variable so we implement
 * only one just in case.
 */

int pthread_cond_broadcast(pthread_cond_t *cond);

#else

int pthread_cond_signal(pthread_cond_t *cond);

#endif

int pthread_cond_destroy(pthread_cond_t *cond);

#endif

#endif


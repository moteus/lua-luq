#include "luq_library_lock.h"
#include <stdlib.h>
#include <memory.h>


static pfn_on_unload luq_global_on_unload = NULL;
static void*         luq_global_on_unload_ctx = NULL;

void luq_library_on_unload(pfn_on_unload fn, void *ctx){
  luq_global_on_unload     = fn;
  luq_global_on_unload_ctx = ctx;
}

#if defined(_WIN32)

#include <windows.h>

static CRITICAL_SECTION luq_global_lock;

BOOL WINAPI DllMain( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved){
  switch(fdwReason){
  case DLL_PROCESS_ATTACH:
    InitializeCriticalSection(&luq_global_lock);
    break;
  case DLL_PROCESS_DETACH:
    DeleteCriticalSection(&luq_global_lock);
    if(luq_global_on_unload){
      luq_global_on_unload(luq_global_on_unload_ctx);
    }
    break;
  }
  return TRUE;
}

void luq_library_lock(){
  EnterCriticalSection(&luq_global_lock);
}

void luq_library_unlock(){
  LeaveCriticalSection(&luq_global_lock);
}

#else

#include <pthread.h>

static pthread_mutex_t luq_global_lock = PTHREAD_MUTEX_INITIALIZER;

void luq_library_lock(){
  pthread_mutex_lock(&luq_global_lock);
}

void luq_library_unlock(){
  pthread_mutex_unlock(&luq_global_lock);
}

// @todo implement calling `luq_global_on_unload` when library unload

#endif

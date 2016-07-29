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

#if 0 && defined(__GNUC__)

/* I do not use DllMain because not sure should I call `DisableThreadLibraryCalls`
 * so I hope default DllMain do right job
 */
static void __attribute__ ((constructor)) luq_on_load(void){
  InitializeCriticalSection(&luq_global_lock);
}

/* There no way to determine reason of unload. And it may be not
 * safe call `luq_global_on_unload` if process in termination stage.
 */
static void __attribute__ ((destructor)) luq_on_unload(void){
  DeleteCriticalSection(&luq_global_lock);
  if(luq_global_on_unload){
    luq_global_on_unload(luq_global_on_unload_ctx);
  }
}

#else

BOOL WINAPI DllMain( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved){
  switch(fdwReason){
  case DLL_PROCESS_ATTACH:
  #if defined(_MSC_VER) && defined(_MT) && defined(_DLL)
    // Do not call "DisableThreadLibraryCalls" in a DLL which is statically linked to the CRT
    // https://support.microsoft.com/en-us/kb/555563
    DisableThreadLibraryCalls(hinstDLL);
  #endif

  // I think this should works but not tested.
  #if defined(__GNUC__) && defined(_DLL)
    DisableThreadLibraryCalls(hinstDLL);
  #endif

    InitializeCriticalSection(&luq_global_lock);
    break;
  case DLL_PROCESS_DETACH:
    if(lpvReserved == NULL){ // Unload because of FreeLibrary
      DeleteCriticalSection(&luq_global_lock);
      if(luq_global_on_unload){
        luq_global_on_unload(luq_global_on_unload_ctx);
      }
    }
    break;
  }
  return TRUE;
}

#endif

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

#ifdef __GNUC__

static void __attribute__ ((destructor)) luq_on_unload(void){
  if(luq_global_on_unload){
    luq_global_on_unload(luq_global_on_unload_ctx);
  }
}

#else

// @todo implement calling `luq_global_on_unload` when library unload

#endif

#endif

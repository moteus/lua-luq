#ifndef _LUQ_LIBRARY_LOCK_B4920AC5_EA40_4156_BDD8_3996694A9E6F_
#define _LUQ_LIBRARY_LOCK_B4920AC5_EA40_4156_BDD8_3996694A9E6F_

void luq_library_lock();

void luq_library_unlock();

typedef void(*pfn_on_unload)(void*);

void luq_library_on_unload(pfn_on_unload fn, void *ctx);

#endif
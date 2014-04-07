#ifndef _LUQ_MAP_DF1EB176_4685_45A0_85C0_4D933BD13E3E_
#define _LUQ_MAP_DF1EB176_4685_45A0_85C0_4D933BD13E3E_

typedef struct luq_map_tag{
  void *impl;
}luq_map_t;

typedef int(*pfn_map_iterator)(const char*, void*, void*);

int luq_map_init(luq_map_t *map);

int luq_map_set(luq_map_t *map, const char *name, void *data, void **prev);

void* luq_map_get(luq_map_t *map, const char *name);

void luq_map_each(luq_map_t *map, pfn_map_iterator iter, void *ctx);

void luq_map_remove(luq_map_t *map, void *data);

void luq_map_destroy(luq_map_t *map);

void luq_map_self_test();

#endif

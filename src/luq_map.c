#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "luq_map.h"

//{ Named List

typedef struct named_list_element_tag{
  struct named_list_element_tag *next;
  struct named_list_element_tag *prev;
  void         *data;
  char         name[1];
} named_list_element_t;

typedef struct named_list_tag{
  named_list_element_t *head;
  named_list_element_t *tail;
} named_list_t;

static named_list_element_t* nl_create_element(const char *name){
  size_t len = strlen(name);
  size_t size = sizeof(named_list_element_t) + len;
  named_list_element_t *self = (named_list_element_t*)malloc(size);
  if(!self) return 0;
  memset(self, 0, size);
  memcpy(self->name, name, len + 1);
  return self;
}

static void nl_destroy_element(named_list_element_t *self){
  if(self){
    free(self);
  }
}

static void nl_push_back_element(named_list_t *list, named_list_element_t *elem){
  elem->prev = list->tail;
  elem->next = NULL;
  if(list->tail) list->tail->next = elem;
  list->tail = elem;
  if(!list->head) list->head = elem;
}

static void nl_remove_element(named_list_t *list, named_list_element_t *elem){
  if(!list->head) return;

  if(elem->prev) elem->prev->next = elem->next;
  if(elem->next) elem->next->prev = elem->prev;

  if(elem == list->head){
    list->head = list->head->next;
  }

  if(elem == list->tail){
    list->tail = list->tail->prev;
  }

  elem->next = elem->prev = NULL;
}

static named_list_element_t* nl_find_by_name(named_list_t *list, const char *name){
  named_list_element_t *cur = list->head;
  for(;cur;cur=cur->next){
    if(strcmp(cur->name, name) == 0){
      return cur;
    }
  }
  return 0;
}

static named_list_element_t* nl_find_by_value(named_list_t *list, void *value){
  named_list_element_t *cur = list->head;
  for(;cur;cur=cur->next){
    if(value == cur->data){
      return cur;
    }
  }
  return 0;
}

static named_list_element_t* nl_ensure_name(named_list_t *list, const char *name){
  named_list_element_t *self = nl_find_by_name(list, name);
  if(self){
    return self;
  }
  self = nl_create_element(name);
  if(!self){
    return 0;
  }
  nl_push_back_element(list, self);
  return self;
}

static void nl_clear(named_list_t *list){
  named_list_element_t *cur;
  while((cur = list->head)){
    nl_remove_element(list, cur);
    nl_destroy_element(cur);
  }
}

//}

#define assert_s_equal(A,B) assert(0 == strcmp((A),(B)))

void luq_map_self_test(){
  named_list_t list_ = {0,0};
  named_list_t *list = &list_;
  named_list_element_t *elem;

  int i;char str[255];
  for(i=1;i<=10;++i){
    sprintf(str, "elem#%d", i);
    elem = nl_find_by_name(list, str);
    assert(0 == elem);

    elem = nl_ensure_name(list, str);
    assert_s_equal(str, elem->name);
    assert(0 == elem->data);
    elem->data = (void*)i;
  }

  for(i=1;i<=10;++i){
    named_list_element_t *elem2;

    sprintf(str, "elem#%d", i);
    elem2 = nl_find_by_name(list, str);
    assert(0 != elem2);

    elem = nl_ensure_name(list, str);
    assert(elem2 == elem);
    assert((void*)i == elem->data);
  }

  nl_clear(list);
  assert(list->head == list->tail);
  assert(0 == list->head);

  elem = nl_ensure_name(list, "1");
  assert(list->head == list->tail);
  assert(list->head == elem);

  nl_remove_element(list, elem);
  assert(list->head == list->tail);
  assert(0 == list->head);

  nl_ensure_name(list, "1");
  elem = nl_ensure_name(list, "2");
  assert(list->head != list->tail);
  assert(list->tail == elem);

  nl_remove_element(list, elem);
  assert(list->head == list->tail);

  elem = nl_ensure_name(list, "2");
  elem = list->head;

  nl_remove_element(list, elem);
  assert(list->head == list->tail);

  elem = nl_ensure_name(list, "3");
  assert(list->head != list->tail);
  assert(list->tail == elem);

  nl_ensure_name(list, "4");
  assert(list->head->next == elem);

  nl_remove_element(list, elem);
  assert(list->head->next == list->tail);
  assert(list->tail->prev == list->head);

  nl_clear(list);
}

int luq_map_init(luq_map_t *map){
  named_list_t *list = (named_list_t *)malloc(sizeof(named_list_t));
  if(!list) return -1;
  map->impl = list;
  list->head = list->tail = NULL;
  return 0;
}

int luq_map_set(luq_map_t *map, const char *name, void *data, void **prev){
  named_list_t *list = (named_list_t *)map->impl;
  named_list_element_t* elem = nl_ensure_name(list, name);
  if(!elem) return -1;
  if(prev) *prev = elem->data;
  if(!data){
    nl_remove_element(list, elem);
    nl_destroy_element(elem);
  }
  else{
    elem->data = data;
  }
  return 0;
}

void* luq_map_get(luq_map_t *map, const char *name){
  named_list_t *list = (named_list_t *)map->impl;
  named_list_element_t* elem = nl_find_by_name(list, name);
  if(!elem) return NULL;
  return elem->data;
}

void luq_map_remove(luq_map_t *map, void *data){
  named_list_t *list = (named_list_t *)map->impl;
  named_list_element_t* elem = nl_find_by_value(list, data);
  if(elem){
    nl_remove_element(list, elem);
    nl_destroy_element(elem);
  }
}

void luq_map_each(luq_map_t *map, pfn_map_iterator iter, void *ctx){
  named_list_t *list = (named_list_t *)map->impl;
  named_list_element_t *cur = list->head;
  for(;cur;cur=cur->next){
    if(0 == iter(cur->name, cur->data, ctx)){
      break;
    }
  }
}


void luq_map_destroy(luq_map_t *map){
  named_list_t *list = (named_list_t *)map->impl;
  nl_clear(list);
  free(list);
  map->impl = NULL;
}

#include "l52util.h"
#include "luq_map.h"
#include "luq_qvoid.h"
#include "luq_library_lock.h"
#include <stdlib.h>
#include <memory.h>

#if defined(_MSC_VER)
#  define LLUQ_EXPORT __declspec(dllexport)
#else
#  define LLUQ_EXPORT
#endif

static luq_map_t luq_global_queue  = {0};
static luq_map_t luq_global_shared = {0};
volatile static int LUQ_INIT = 0;

#define LLUQ_FLAG_TYPE unsigned char
#define LLUQ_FLAG_OPEN (LLUQ_FLAG_TYPE)1 << 0

//{ Lua interface

#define LLUQ_QVOID_NAME "LUQ Queue"

static const char *LLUQ_QVOID_TYPE = LLUQ_QVOID_NAME;

typedef struct lluq_qvoid_tag{
  LLUQ_FLAG_TYPE flags;
  qvoid_t *q;
} lluq_qvoid_t;

static int lluq_qvoid_new(lua_State *L, qvoid_t *q){
  lluq_qvoid_t *queue = lutil_newudatap(L, lluq_qvoid_t, LLUQ_QVOID_TYPE);
  queue->flags = LLUQ_FLAG_OPEN;
  queue->q = q;
  return 1;
}

static lluq_qvoid_t* lluq_qvoid_at(lua_State *L, int i){
  lluq_qvoid_t *queue = (lluq_qvoid_t *)lutil_checkudatap (L, i, LLUQ_QVOID_TYPE);
  luaL_argcheck (L, queue != NULL, 1, LLUQ_QVOID_NAME" expected");
  return queue;
}

static void* lluq_ffi_to_lightud(lua_State *L, int idx){
  void **data;
  luaL_argcheck(L, lua_type(L, idx) > LUA_TTHREAD, idx, "ffi type expected");
  data = (void**)lua_topointer(L, idx);
  lua_pushlightuserdata(L, *data);
  lua_replace(L, idx);
  return *data;
}

static void* lluq_str_to_lightud(lua_State *L, int idx){
  size_t len; void **data = (void **)luaL_checklstring(L, idx, &len);
  luaL_argcheck(L, len == sizeof(void*), idx, "invalid string length");

  lua_pushlightuserdata(L, *data);
  lua_replace(L, idx);
  return *data;
}

static void* ensure_lud(lua_State *L, int idx){
  int t = lua_type(L, idx);
  
  if(LUA_TLIGHTUSERDATA == t){
    return lua_touserdata(L, idx);
  }

  if(LUA_TSTRING == t){
    return lluq_str_to_lightud(L, idx);
  }

  if(LUA_TTHREAD < t){
    return lluq_ffi_to_lightud(L, idx);
  }

  luaL_argcheck(L, 0, idx, "lightuserdata/ffi cdata/string expected");
  return 0;
}

static int lluq_qvoid_put(lua_State *L){
  qvoid_t *q = lluq_qvoid_at(L, 1)->q;
  void *data = ensure_lud(L, 2);
  lua_pushnumber(L, qvoid_put(q, data));
  return 1;
}

static int lluq_qvoid_put_nolock(lua_State *L){
  qvoid_t *q = lluq_qvoid_at(L, 1)->q;
  void *data = ensure_lud(L, 2);
  lua_pushnumber(L, qvoid_put_nolock(q, data));
  return 1;
}

static int lluq_qvoid_put_timeout(lua_State *L){
  qvoid_t *q = lluq_qvoid_at(L, 1)->q;
  void *data = ensure_lud(L, 2);
  int ms     = luaL_optint(L, 3, 0);
  int ret    = qvoid_put_timeout(q, data, ms);
  if(ret == ETIMEDOUT) lua_pushliteral(L, "timeout");
  else lua_pushnumber(L, ret);
  return 1;
}

static int lluq_qvoid_get(lua_State *L){
  qvoid_t *q = lluq_qvoid_at(L, 1)->q;
  void *data;
  int rc = qvoid_get(q, &data);
  if(rc == 0)lua_pushlightuserdata(L, data);
  else lua_pushnumber(L, rc);
  return 1;
}

static int lluq_qvoid_get_nolock(lua_State *L){
  qvoid_t *q = lluq_qvoid_at(L, 1)->q;
  void *data; int rc = qvoid_get_nolock(q, &data);
  if(rc == 0)lua_pushlightuserdata(L, data); else lua_pushnil(L);
  return 1;
}

static int lluq_qvoid_get_timeout(lua_State *L){
  qvoid_t *q = lluq_qvoid_at(L, 1)->q;
  void *data;
  int ms = luaL_optint(L, 2, 0);
  int rc = qvoid_get_timeout(q, &data, ms);
  if(rc == 0)lua_pushlightuserdata(L, data);
  else if(rc == ETIMEDOUT)
    lua_pushliteral(L, "timeout");
  else
    lua_pushnumber(L, rc);
  return 1;
}

static int lluq_qvoid_capacity(lua_State *L){
  qvoid_t *q = lluq_qvoid_at(L, 1)->q;
  lua_pushnumber(L, qvoid_capacity(q));
  return 1;
}

static int lluq_qvoid_size(lua_State *L){
  qvoid_t *q = lluq_qvoid_at(L, 1)->q;
  lua_pushnumber(L, qvoid_size(q));
  return 1;
}

static int lluq_qvoid_clear(lua_State *L){
  qvoid_t *q = lluq_qvoid_at(L, 1)->q;
  lua_pushnumber(L, qvoid_clear(q));
  return 1;
}

static int lluq_qvoid_lock(lua_State *L){
  qvoid_t *q = lluq_qvoid_at(L, 1)->q;
  lua_pushnumber(L, qvoid_lock(q));
  return 1;
}

static int lluq_qvoid_unlock(lua_State *L){
  qvoid_t *q = lluq_qvoid_at(L, 1)->q;
  if(lua_toboolean(L,2)) qvoid_notify(q);
  lua_pushnumber(L, qvoid_unlock(q));
  return 1;
}

static int lluq_qvoid_close(lua_State *L){

  lua_pushnumber(L, 0);
  return 1;
}

static const struct luaL_Reg lluq_qvoid_meth[] = {
  { "put",           lluq_qvoid_put           },
  { "put_nolock",    lluq_qvoid_put_nolock    },
  { "put_timeout",   lluq_qvoid_put_timeout   },
  { "get",           lluq_qvoid_get           },
  { "get_nolock",    lluq_qvoid_get_nolock    },
  { "get_timeout",   lluq_qvoid_get_timeout   },
  { "lock",          lluq_qvoid_lock          },
  { "unlock",        lluq_qvoid_unlock        },
  { "capacity",      lluq_qvoid_capacity      },
  { "size",          lluq_qvoid_size          },
  { "clear",         lluq_qvoid_clear         },
  { "close",         lluq_qvoid_close         },
  {NULL, NULL}
};

//}

static int lluq_queue(lua_State *L){
  const char *name = luaL_checkstring(L, 1);
  qvoid_t *q; int exists = 1;

  luq_library_lock();
  q = luq_map_get(&luq_global_queue, name);
  if(!q){
    exists = 0;
    q = (qvoid_t*)malloc(sizeof(qvoid_t));
    if(q){
      if(0 != qvoid_init(q)){
        free(q);
        q = NULL;
      }
      else{
        luq_map_set(&luq_global_queue, name, q, NULL);
      }
    }
  }
  luq_library_unlock();

  lluq_qvoid_new(L, q);
  lua_pushboolean(L, exists);
  return 2;
}

static int lluq_close(lua_State *L){
  lluq_qvoid_t *q = lluq_qvoid_at(L, 1);
  luq_library_lock();
  luq_map_remove(&luq_global_queue, q->q);
  luq_library_unlock();

  qvoid_destroy(q->q);
  free(q->q);

  lua_pushboolean(L,1);
  return 1;
}

static int lluq_pointer_size(lua_State *L){
  lua_pushnumber(L, sizeof((void*)0));
  return 1;
}

static const struct luaL_Reg lluq_lib[] = {
  { "queue",         lluq_queue         },
  { "pointer_size",  lluq_pointer_size  },
  { "close",         lluq_close         },
  {NULL, NULL}
};

static int lluq_destroy_qvoid(const char *name, void *q, void *ctx){
  qvoid_destroy((qvoid_t*)q);
  return 1;
}

static void lluq_cleanup(void*ctx){
  luq_map_each(&luq_global_queue, lluq_destroy_qvoid, NULL);
  luq_map_destroy(&luq_global_queue);
  luq_map_destroy(&luq_global_shared);
}

LLUQ_EXPORT int luaopen_luq(lua_State *L){
  if(!LUQ_INIT){
    luq_library_lock();
    if(!LUQ_INIT){
      if(0 == luq_map_init(&luq_global_queue)){
        if(0 == luq_map_init(&luq_global_shared)){
          LUQ_INIT = 1;
          luq_library_on_unload(lluq_cleanup, NULL);
        }
        else{
          luq_map_destroy(&luq_global_queue);
        }
      }
    }
    luq_library_unlock();
  }
  if(!LUQ_INIT){
    lua_pushstring(L, "Can not init LUQ library");
    return lua_error(L);
  }

  lutil_createmetap(L, LLUQ_QVOID_TYPE, lluq_qvoid_meth, 0);

  lua_newtable(L);
  luaL_setfuncs(L, lluq_lib, 0);
  return 1;
}

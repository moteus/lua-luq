package = "luq"
version = "scm-0"

source = {
  url = "https://github.com/moteus/lua-luq/archive/master.zip",
  dir = "lua-luq-master",
}

description = {
  summary = "Lua light userdata queue",
  homepage = "https://github.com/moteus/lua-luq",
  license = "MIT/X11",
}

dependencies = {
  "lua >= 5.1, < 5.4",
}

build = {
  copy_directories = {"test"},

  type = "builtin",

  platforms = {
    linux    = { modules = {
      ["luq"] = {
        libraries = {"pthread", "rt"},
      }
    }},

    unix     = { modules = {
      ["luq"] = {
        libraries = {"pthread"},
      }
    }},

    -- mingw32  = { modules = {
    --   ["lzmq.pool.core"] = {
    --     libraries = {"pthread"},
    --     defines   = {"USE_PTHREAD"},
    --   }
    -- }},
  },

  modules = {
    ["luq" ] = {
      sources = {
        'src/l52util.c', 'src/luq.c', 'src/luq_library_lock.c',
        'src/luq_map.c', 'src/luq_pthread.c', 'src/luq_qvoid.c',
      },
    },
  },
}



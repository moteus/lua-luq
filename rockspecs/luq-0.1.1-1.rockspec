package = "luq"
version = "0.1.1-1"

source = {
  url = "https://github.com/moteus/lua-luq/archive/v0.1.1.zip",
  dir = "lua-luq-0.1.1",
}

description = {
  summary = "Lua light userdata queue",
  homepage = "https://github.com/moteus/lua-luq",
  license = "MIT/X11",
}

dependencies = {
  "lua >= 5.1, < 5.3",
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



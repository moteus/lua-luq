local HAS_RUNNER = not not lunit 

local lunit      = require "lunit"
local TEST_CASE  = assert(lunit.TEST_CASE)
local skip       = lunit.skip or function() end
local IS_LUA52   = _VERSION >= 'Lua 5.2'

local path = require "path"

-----------------------------------------------------------
local exec do

local lua_version_t
local function lua_version()
  if not lua_version_t then 
    local version = rawget(_G,"_VERSION")
    local maj,min = version:match("^Lua (%d+)%.(%d+)$")
    if maj then                         lua_version_t = {tonumber(maj),tonumber(min)}
    elseif not math.mod then            lua_version_t = {5,2}
    elseif table.pack and not pack then lua_version_t = {5,2}
    else                                lua_version_t = {5,2} end
  end
  return lua_version_t[1], lua_version_t[2]
end

local LUA_MAJOR, LUA_MINOR = lua_version()
local LUA_VERSION = LUA_MAJOR * 100 + LUA_MINOR
local LUA_52 = 502
local IS_WINDOWS = package.config:sub(1,1) == '\\'

local function read_file(n)
  local f, e = io.open(n, "r")
  if not f then return nil, e end
  local d, e = f:read("*all")
  f:close()
  return d, e
end

exec = function(cwd, cmd, ...)
  local tmpfile = path.tmpname()

  cmd = path.quote(cmd)
  if ... then
    cmd = cmd .. ' ' .. string.format(...) .. ' '
    if IS_WINDOWS then cmd = path.quote(cmd) end
  end
  cmd = cmd .. ' >' .. path.quote(tmpfile) .. ' 2>&1'

  local p
  if cwd and (cwd ~= "") and (cwd ~= ".") then
    p = path.currentdir()
    path.chdir(cwd)
  end

  local res1,res2,res2 = os.execute(cmd)
  if p then path.chdir(p) end

  local data = read_file(tmpfile)
  path.remove(tmpfile)

  if LUA_VERSION < LUA_52 then
    return res1==0, res1, data
  end

  return res1, res2, data
end

end
-----------------------------------------------------------

-----------------------------------------------------------
local function lua_args(args)
  local a, i = {}, -1
  while args[i] do
    table.insert(a, 1, args[i])
    i = i - 1
  end

  local lua = table.remove(a, 1)
  return lua, table.concat(a, ' ')
end
-----------------------------------------------------------

-----------------------------------------------------------
local exec_lua do

local function write_file(dest, data)
  local f, e = io.open(dest, 'w+b')
  if not f then return nil, e end
  f:write(data)
  f:close()
  return true
end

exec_lua = function(lua, src)
  local tmpfile = path.tmpname()
  local ok, err = write_file(tmpfile, src)
  if not ok then return nil, err end
  local status, code, result = exec('.', lua, tmpfile)
  path.remove(tmpfile)
  return status, code, result
end

end
-----------------------------------------------------------

local lua, args = lua_args(arg)

local _ENV = TEST_CASE'final test'           if true  then

function test_library_unload()
  local ok, code, msg = exec_lua(lua, [[
    require "luq"
  ]])
  assert_true(ok, msg)
end

function test_process_unload()
  local ok, code, msg = exec_lua(lua, [[
    require "luq"
    os.exit()
  ]])
  assert_true(ok, msg)
end

end

if not HAS_RUNNER then lunit.run() end


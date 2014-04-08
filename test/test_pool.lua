local HAS_RUNNER = not not lunit 

local lunit      = require "lunit"
local TEST_CASE  = assert(lunit.TEST_CASE)
local skip       = lunit.skip or function() end
local IS_LUA52   = _VERSION >= 'Lua 5.2'


local luq    = require "luq"
local ztimer = require "lzmq.timer"

local QUEUE_NAME   = "lzmq/pool/"
local POINTER_SIZE = luq.pointer_size()
local POINTER      = ("\0"):rep(POINTER_SIZE)

local function return_count(...)
  return select('#', ...), ...
end

print("------------------------------------")
print("Lua version: " .. (_G.jit and _G.jit.version or _G._VERSION))
print("------------------------------------")
print("")

local _ENV = TEST_CASE'core queue'           if true  then

local timeout, epselon = 1500, 490
local q

function setup()
  q = luq.queue(QUEUE_NAME)
  q:clear()
end

function teardown()
  q:clear()
  luq.close(q)
  q = nil
end

function test_interface()
  assert_function(q.put           )
  assert_function(q.put_nolock    )
  assert_function(q.put_timeout   )
  assert_function(q.get           )
  assert_function(q.get_nolock    )
  assert_function(q.get_timeout   )
  assert_function(q.lock          )
  assert_function(q.unlock        )
  assert_function(q.capacity      )
  assert_function(q.size          )
  assert_function(q.clear         )
  assert_function(q.close         )
end

function test_timeout()
  assert_equal(0, q:size(0))

  timer = ztimer.monotonic():start()
  assert_equal("timeout", q:get_timeout(timeout))
  local elapsed = timer:stop()
  assert(elapsed > (timeout-epselon), "Expeted " .. timeout .. "(+/-" .. epselon .. ") got: " .. elapsed)
  assert(elapsed < (timeout+epselon), "Expeted " .. timeout .. "(+/-" .. epselon .. ") got: " .. elapsed)

  timer:start()
  assert_equal("timeout", q:get_timeout(timeout))
  local elapsed = timer:stop()
  assert(elapsed > (timeout-epselon), "Expeted " .. timeout .. "(+/-" .. epselon .. ") got: " .. elapsed)
  assert(elapsed < (timeout+epselon), "Expeted " .. timeout .. "(+/-" .. epselon .. ") got: " .. elapsed)

  assert_equal(0, q:put(POINTER))
  assert_userdata(q:get_timeout(timeout))
end

function test_clear()
  assert_equal(0, q:size())

  assert_equal(0, q:put(POINTER))
  assert_equal(0, q:put(POINTER))

  assert_equal(2, q:size())
  assert_equal(0, q:clear())
  assert_equal(0, q:size())
end

function test_wrong_pointer_size()
  assert_error(function() q:put(("\0"):rep(POINTER_SIZE+1)) end)
  assert_error(function() q:put(("\0"):rep(POINTER_SIZE-1)) end)
  assert_error(function() q:put(("\0"):rep(POINTER_SIZE*2)) end)
end

end

if not HAS_RUNNER then lunit.run() end

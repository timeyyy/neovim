-- local ffi = require('ffi')
local helpers = require('test.unit.helpers')(after_each)
local itp = helpers.gen_itp(it) -- runs in seperate process ?

local to_cstr = helpers.to_cstr

local eq = helpers.eq

local globals = helpers.cimport("./src/nvim/globals.h")
local buffer = helpers.cimport("./src/nvim/buffer.h")
local extmarks = helpers.cimport("./src/nvim/mark_extended.h")


describe('extmark_col_adjust works', function()
  -- before_each(function()
  -- end)

  -- TODO These are defined in buffer_spec.lua, turn into helpers?
  local buflist_new = function(file, flags)
    local c_file = to_cstr(file)
    return buffer.buflist_new(c_file, c_file, 1, flags)
  end

  local close_buffer = function(win, buf, action, abort_if_last)
    return buffer.close_buffer(win, buf, action, abort_if_last)
  end

  local path1 = 'test_file_path'
  local path2 = 'file_path_test'
  local path3 = 'path_test_file'

  before_each(function()
    -- create the files
    io.open(path1, 'w').close()
    io.open(path2, 'w').close()
    io.open(path3, 'w').close()
  end)

  after_each(function()
    os.remove(path1)
    os.remove(path2)
    os.remove(path3)
  end)


  -- local col_adjust = function(lnum, mincol, lnum_amount, col_amount)
  -- end

  itp('nothing happens on no marks #myunit', function()
    local buf = buflist_new(path1, buffer.BLN_LISTED)
    -- extmarks.extmark_col_adjust(buf, )
    -- print(globals)
    eq(1, 1)
  end)
end)

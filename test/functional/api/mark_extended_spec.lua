
local helpers = require('test.functional.helpers')(after_each)
local Screen = require('test.functional.ui.screen')

local eq = helpers.eq
local neq = helpers.neq
local buffer = helpers.buffer
local insert = helpers.insert
local feed = helpers.feed
local wait = helpers.wait

describe('creating a new mark', function()
  local screen
  helpers.clear()
  helpers.nvim('set_option', 'shell', helpers.nvim_dir .. '/shell-test')
  helpers.nvim('set_option', 'shellcmdflag', 'EXE')
  screen = Screen.new(20, 4)
  screen:attach(false)

  it('works', function()
    local buf = helpers.nvim('get_current_buffer')
    local string_mark = "test"
    local row = 0
    local col = 2

    local init_text = "12345"
    insert(init_text)
    -- mark the "2"
    local rv = buffer('mark_set', buf, string_mark, row, col)
    wait()
    eq(1, rv)
    rv = buffer('mark_set', buf, 'abc', 0, 3)
    wait()
    eq(1, rv)
  end)
    -- local pos = buffer('mark_index', buf, string_mark)
    -- neq(pos, nil)
    -- eq(row, pos[1])
    -- eq(col, pos[2])

    -- add some more marks and test we can get the names
    -- marks = {'a', 'b', 'c'}
    -- positions = {{0, 1,}, {0, 3}}
    -- for i, m in ipairs(marks) do
        -- rv = buffer('mark_set', buf, m, positions[i][1], positions[i][2])
        -- eq(1, rv)
    -- end

    -- rv = buffer('mark_set', buf, 'abc', 0, 3)
    -- wait()
    -- eq(1, rv)
    -- local names = buffer('mark_names', buf)
    -- for k in marks do
        -- neq(names[k], nil)
    -- end
    --
    -- marks move with text inserts?
    -- added_text = "999"
    -- feed('0') -- move cursor to start of line
    -- insert(added_text)
    -- wait()
    -- screen:expect([[
      -- 99^9123              |
      -- ~                   |
      -- ~                   |
                          -- |
    -- ]])
    -- pos = buffer('mark_index', buf, string_mark)
    -- neq(pos, nil)
    -- eq(row, pos[1])
    -- eq(col + string.len(added_text), pos[2])

    -- Update an existing mark
    -- row = 0
    -- col = 5
    -- rv = buffer('mark_set', buf, string_mark, row, col)
    -- wait()
    -- eq(2, rv)
    -- pos = buffer('mark_index', buf, string_mark)
    -- eq(row, pos[1])
    -- eq(col, pos[2])

    -- remove a mark
    -- rv = buffer('mark_unset', buf, string_mark)
    -- eq(1, rv)
    -- TODO catch the error..
    -- pos = buffer('mark_index', buf, string_mark)
  -- end)
end)


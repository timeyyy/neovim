
local helpers = require('test.functional.helpers')(after_each)
local Screen = require('test.functional.ui.screen')

local request = helpers.request
local eq = helpers.eq
local neq = helpers.neq
local buffer = helpers.buffer
local insert = helpers.insert
local feed = helpers.feed
local wait = helpers.wait

describe('Extmarks buffer api', function()
  local screen
  local curbuf
  local marks, positions, ns_string2, ns_string, init_text, row, col
  local ns

  before_each(function()
    marks = {1, 2, 3}
    positions = {{0, 1,}, {0, 3}, {0, 4}}

    ns_string = "my-fancy-plugin"
    ns_string2 = "my-fancy-plugin2"
    init_text = "12345"
    row = 0
    col = 2

    ns = 0

    helpers.clear()
    helpers.nvim('set_option', 'shell', helpers.nvim_dir .. '/shell-test')
    helpers.nvim('set_option', 'shellcmdflag', 'EXE')
    screen = Screen.new(20, 10)
    screen:attach(false)

    insert(init_text)
    buf = request('vim_get_current_buffer')
  end)

  -- Implemented as Globals, that is why we don't have to init before other tests
  it('add and query namespaces', function()
    -- TODO test calling functions fails before this is scalled
    rv = buffer('mark_ns_init', ns_string)
    eq(1, rv)
    -- TODO Test error
    -- rv = buffer('mark_ns_init', ns_string)
    -- eq(0, rv)
    rv = buffer('mark_ns_init', ns_string2)
    eq(2, rv)

    rv = buffer('mark_ns_ids')
    eq({ns_string, 1}, rv[1])
    eq({ns_string2, 2}, rv[2])
  end)

  it('adds, updates  and deletes marks', function()
    rv = buffer('mark_set', buf, ns, marks[1], positions[1][1], positions[1][2])
    eq(1, rv)
    rv = buffer('mark_index', buf, ns, marks[1])
    eq({marks[1], positions[1][1], positions[1][2]}, rv)
    -- Test adding a second mark works
    rv = buffer('mark_set', buf, ns, marks[2], positions[2][1], positions[2][2])
    wait()
    eq(1, rv)
    -- Test an update, (same pos)
    rv = buffer('mark_set', buf, ns, marks[1], positions[1][1], positions[1][2])
    eq(2, rv)
    rv = buffer('mark_index', buf, ns, marks[2])
    eq({marks[2], positions[2][1], positions[2][2]}, rv)
    -- Test an update, (new pos)
    row = positions[1][1]
    col = positions[1][2] + 1
    rv = buffer('mark_set', buf, ns, marks[1], row, col)
    eq(2, rv)
    rv = buffer('mark_index', buf, ns, marks[1])
    eq({marks[1], row, col}, rv)

    -- remove the test marks
    rv = buffer('mark_unset', buf, ns, marks[1])
    eq(1, rv)
    rv = buffer('mark_unset', buf, ns, marks[2])
    eq(1, rv)
  end)

  it('querying for information and ranges', function()
    -- add some more marks
    for i, m in ipairs(marks) do
        rv = buffer('mark_set', buf, ns, m, positions[i][1], positions[i][2])
        eq(1, rv)
    end

    rv = buffer('mark_ids', buf, ns)
    for i, m in ipairs(marks) do
        eq({m, positions[i][1], positions[i][2]}, rv[i])
    end

    -- Using next
    rv = buffer('mark_next', buf, ns, marks[1])
    eq({marks[2], positions[2][1], positions[2][2]}, rv)
    rv = buffer('mark_next', buf, ns, marks[2])
    eq({marks[3], positions[3][1], positions[3][2]}, rv)
    rv = buffer('mark_next', buf, ns, marks[3])
    eq({-1, -1, -1}, rv)
    -- Using nextrange
    rv = buffer('mark_nextrange', buf, ns, marks[1], marks[3])
    eq({marks[1], positions[1][1], positions[1][2]}, rv[1])
    eq({marks[2], positions[2][1], positions[2][2]}, rv[2])
    eq({marks[3], positions[3][1], positions[3][2]}, rv[3])

    -- Using prev
    -- rv = buffer('mark_prev', buf, ns, marks[3])
    -- eq({marks[2], positions[2][1], positions[2][2]}, rv)
    -- rv = buffer('mark_prev', buf, ns, marks[2])
    -- eq({marks[1], positions[1][1], positions[1][2]}, rv)
    -- eq(positions[1], rv[2])
    -- rv = buffer('mark_prev', buf, ns, marks[1])
    -- eq({-1, -1, -1}, rv)
    -- prev range
    -- rv = buffer('mark_prevrange', buf, ns, marks[1], marks[3])
    -- eq({marks[3], positions[3][1], positions[3][2]}, rv[1])
    -- eq({marks[2], positions[2][1], positions[2][2]}, rv[2])
    -- eq({marks[1], positions[1][1], positions[1][2]}, rv[3])
    -- end)

  -- returns ranges correctly when using positional indexes
    rv = buffer('mark_index', buf, ns, positions[1])
    eq({marks[1], positions[1][1], positions[1][2]}, rv)

    rv = buffer('mark_next', buf, ns, positions[1])
    eq({marks[2], positions[2][1], positions[2][2]}, rv)

    rv = buffer('mark_nextrange', buf, ns, positions[1], positions[3])
    eq({marks[1], positions[1][1], positions[1][2]}, rv[1])
    eq({marks[2], positions[2][1], positions[2][2]}, rv[2])
    eq({marks[3], positions[3][1], positions[3][2]}, rv[3])

    -- rv = buffer('mark_prev', buf, ns, positions[3])
    -- eq({marks[2], positions[2][1], positions[2][2]}, rv)

    -- rv = buffer('mark_prevrange', buf, ns, positions[1], positions[3])
    -- eq({marks[3], positions[3][1], positions[3][2]}, rv[1])
    -- eq({marks[2], positions[2][1], positions[2][2]}, rv[2])
    -- eq({marks[1], positions[1][1], positions[1][2]}, rv[3])
  end)

  it('marks move with line insertations #good', function()
    buffer('mark_set', buf, ns, marks[1], 1, 1)
    feed("yyP")
    rv = buffer('mark_index', buf, ns, marks[1])
    eq(2, rv[2])
    eq(1, rv[3])
  end)

  it('marks move with multiline insertations #good', function()
    feed("a<cr>22<cr>33<esc>")
    buffer('mark_set', buf, ns, marks[1], 2, 2)
    feed('ggVGyP')
    rv = buffer('mark_index', buf, ns, marks[1])
    eq(5, rv[2])
    eq(2, rv[3])
  end)

  it('marks move with line join #good', function()
    feed("a<cr>222<esc>")
    buffer('mark_set', buf, ns, marks[1], 2, 1)
    feed('ggJ')
    rv = buffer('mark_index', buf, ns, marks[1])
    eq(1, rv[2])
    eq(7, rv[3])
  end)

  it('marks move with multiline join #good', function()
    feed("a<cr>222<cr>333<cr>444<esc>")
    buffer('mark_set', buf, ns, marks[1], 4, 1)
    feed('2GVGJ')
    rv = buffer('mark_index', buf, ns, marks[1])
    eq(2, rv[2])
    eq(9, rv[3])
  end)

  -- TODO throw err if lnum = 0
  it('marks move with line splits #good', function()
    -- open_line in misc1.c
    buffer('mark_set', buf, ns, marks[1], 1, 3)
    feed('0a<cr><esc>')
    rv = buffer('mark_index', buf, ns, marks[1])
    eq(2, rv[2])
    eq(2, rv[3])
  end)

  it('marks move with line deletes #good', function()
    feed("a<cr>222<cr>333<cr>444<esc>")
    buffer('mark_set', buf, ns, marks[1], 3, 2)
    feed('ggdd')
    rv = buffer('mark_index', buf, ns, marks[1])
    eq(2, rv[2])
    eq(2, rv[3])
  end)

  it('marks move with multiline deletes #good', function()
    feed("a<cr>222<cr>333<cr>444<esc>")
    buffer('mark_set', buf, ns, marks[1], 4, 1)
    feed('gg2dd')
    rv = buffer('mark_index', buf, ns, marks[1])
    eq(2, rv[2])
    eq(1, rv[3])
  end)


  it('marks move with char inserts #gdbgood', function()
    -- ins_char_bytes in misc1.c
    -- extmark_col_adjust(curbuf, lnum, curwin->w_cursor.col, 0, charlen);
    screen:snapshot_util()
    buffer('mark_set', buf, ns, marks[1], 1, 1)
    feed('0iab')
    screen:snapshot_util()
    rv = buffer('mark_index', buf, ns, marks[1])
    eq(1, rv[2])
    eq(3, rv[3])
  end)

  it('marks move with single line char deletes #good', function()
    buffer('mark_set', buf, ns, marks[1], 1, 4)
    feed('02dl')
    rv = buffer('mark_index', buf, ns, marks[1])
    eq(1, rv[2])
    eq(2, rv[3])
    end)

  it('marks move with multi line char deletes #gdbgood', function()
    feed('a<cr>12345<esc>h<c-v>hhkd')
    screen:snapshot_util()
    buffer('mark_set', buf, ns, marks[1], 2, 5)
    rv = buffer('mark_index', buf, ns, marks[1])
    eq(2, rv[2])
    eq(2, rv[3])
    end)

end)

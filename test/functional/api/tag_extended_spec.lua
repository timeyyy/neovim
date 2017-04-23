
local helpers = require('test.functional.helpers')(after_each)
local Screen = require('test.functional.ui.screen')

local request = helpers.request
local eq = helpers.eq
local neq = helpers.neq
local buffer = helpers.buffer
local nvim = helpers.nvim
local insert = helpers.insert
local feed = helpers.feed

describe('Extmarks buffer api', function()
  local screen
  local curbuf
  local tags, positions, ns_string2, ns_string, init_text, row, col
  local ns

  before_each(function()
    tags = {1, 2, 3}
    positions = {{1, 1}, {1, 3},
                 {1, 4}, {1, 6},
                 {1, 8}, {1, 9}}

    ns_string = "my-fancy-plugin"
    ns_string2 = "my-fancy-plugin2"

    helpers.clear()
    helpers.nvim('set_option', 'shell', helpers.nvim_dir .. '/shell-test')
    helpers.nvim('set_option', 'shellcmdflag', 'EXE')
    screen = Screen.new(10, 10)
    screen:attach()

    insert(init_text)
    buf = request('vim_get_current_buffer')

    ns = 0
    -- 'add and query namespaces', these are required for tags to be created
    if ns == 0 then
      ns = request('nvim_init_tag_ns', ns_string)
      eq(1, ns)
      rv = request('nvim_init_tag_ns', ns_string2)
      eq(2, rv)
      rv = request('nvim_get_tag_ns_ids')
      eq({1, ns_string}, rv[1])
      eq({2, ns_string2}, rv[2])
    end
  end)

  it('adds, updates and deletes tags #tbad', function()
    -- TODO test get_tags works when nothing has been set
    feed('A<cr>12345<esc>')
    rv = buffer('set_tag', buf, ns, tags[1], positions[1], positions[2])
    eq(1, rv)
    rv = buffer('get_tags', buf, ns, tags[1], positions[1], positions[2], 1, 0)
    eq({{positions[1], positions[2]}}, rv)
    -- Test adding a second tag to the tag group works
    -- rv = buffer('set_tag', buf, ns, tags[1], positions[1], positions[2])
    -- eq(1, rv)
    -- rv = buffer('get_tags', buf, ns, tags[1], positions[2], positions[3], 1, 0)
    -- eq({positions[1], positions[2], positions[2], positions[3]}, rv)
    -- Test adding a second tag group
    rv = buffer('set_tag', buf, ns, tags[2], positions[1], positions[2])
    eq(1, rv)
    rv = buffer('get_tags', buf, ns, tags[2], positions[1], positions[3], 1, 0)
    eq({{positions[1], positions[2]}}, rv)

    -- Test an update, (same pos)
    -- TODO either silently do nothing or raise an error??

    -- Test deleting
    rv = buffer('unset_tag', buf, ns, tags[3], positions[3], positions[4])
    -- Invalid Tag
    eq(2, rv)
    rv = buffer('unset_tag', buf, ns, tags[1], positions[1], positions[2])
    -- Success
    eq(1, rv)
    rv = buffer('unset_tag', buf, ns, tags[1], positions[3], positions[4])
    -- Nothing deleted
    eq(0, rv)
    rv = buffer('unset_tag', buf, ns, tags[2], positions[1], positions[2])
    eq(1, rv)
  end)

  it('querying for information and ranges #tagood', function()
    -- TODO test get_tags works when nothing has been set
    feed('A<cr>12345<esc>')
    -- Add the tags out of order
    buffer('set_tag', buf, ns, tags[1], positions[1], positions[2])
    buffer('set_tag', buf, ns, tags[1], positions[5], positions[6])
    buffer('set_tag', buf, ns, tags[1], positions[3], positions[4])

    -- Test the ranges come back in correct order
    rv = buffer('get_tag_ranges', buf, ns, tags[1])
    eq({{positions[1], positions[2]},
        {positions[3], positions[4]},
        {positions[5], positions[6]}}, rv)
  end)
end)

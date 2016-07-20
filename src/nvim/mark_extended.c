/* A normal mark that you would find in other text editors
 * The marks exists seperatley to vim marks, there are no
 * special marks for insert cursort etc
 *
 * Marks are stored in a btree for fast searching
 * A map of pointers to the marks is used for fast lookup
 *
 * TODO Mark gravity to left or right.
 */

#include "nvim/vim.h"
#include "nvim/mark_extended.h"
#include "nvim/memory.h"
#include "nvim/globals.h"      // FOR_ALL_BUFFERS
#include "nvim/mark.h"         // SET_FMARK
#include "nvim/map.h"          // pmap ...
#include "nvim/lib/kbtree.h"   // kbitr ...

#ifdef INCLUDE_GENERATED_DECLARATIONS
# include "mark_extended.c.generated.h"
#endif

/* Create or update an extmark */
int extmark_set(buf_T *buf, char *ns, char *name, linenr_T row, colnr_T col)
{
  ExtendedMark *extmark = extmark_get(buf, ns, name);
  if (!extmark){
    return extmark_create(buf, ns, name, row, col);
  }
  else {
    extmark_update(extmark, buf, ns, name, row,  col);
    return 2;
  }
}

/* returns 0 on missing name */
int extmark_unset(buf_T *buf, char *ns, char *name)
{
  ExtendedMark *extmark = extmark_get(buf, ns, name);
  if (!extmark){
    return FAIL;
  }
  return extmark_delete(extmark, buf, ns, name);
}

/* Get all mark names ordered by position*/
NamesArray *extmark_names(buf_T *buf, char *ns)
{
  NamesArray extmark_names = KV_INITIAL_VALUE;
  NamesArray *array = &extmark_names;
  char *name;
  FOR_EXTMARKS_IN_NS(buf, ns)
    name = pmap_get(cstr_t)(extmark->names, ns);
    kv_push(extmark_names, name);
  END_FOR_EXTMARKS_IN_NS
  return array;
}

/* Returns the postion of the given mark  */
pos_T *extmark_index(buf_T *buf, char *ns, char *name) {
  ExtendedMark *extmark = extmark_get(buf, ns, name);
  if (!extmark){
    return NULL;
  }
  return &(extmark->fmark.mark);
}

/* Given a text position, finds the next mark */
pos_T *extmark_next(buf_T *buf, char *ns, linenr_T row, colnr_T col)
{
  return get_pos(buf, ns, row, col, 1);
}

/* Given a text position, finds the previous mark */
pos_T *extmark_prev(buf_T *buf, char *ns, linenr_T row, colnr_T col)
{
  return get_pos(buf, ns, row, col, 0);
}

/* Returns the position of marks between a range, */
/* marks found at the start or end index will be included, */
/* if ucol (upper col) and urow  are negative the buffer */
/* will be searched to the end, */
/* Reversed can be set to control the order of the array */
static PosArray *get_pos_range(buf_T *buf, char *ns,
                           linenr_T lrow, colnr_T lcol,
                           linenr_T urow, colnr_T ucol,
                           bool reversed)
{
  pos_T lower, upper;
  lower.lnum = lrow;
  lower.col = lcol;
  upper.lnum = urow;
  upper.col = ucol;

  PosArray positions_found = KV_INITIAL_VALUE;
  PosArray  *array = &positions_found;

  int cmp;
  int finish = 1;

  FOR_EXTMARKS_IN_NS(buf, ns)
    if (pos_cmp(extmark->fmark.mark, upper) == 1) {
      break;
    }
    cmp = pos_cmp(lower, extmark->fmark.mark);
    if (cmp != finish) {
      kv_push(positions_found, extmark->fmark.mark);
    }
    else if (cmp == finish) {
      break;
    }
  END_FOR_EXTMARKS_IN_NS

  /* Swap the elements in the array */
  /* if (reversed) { */
      /* pos_T holder; */
      /* size_t n = sizeof(array); */
      /* int end = n-1; */
      /* for (int i = 0; i < n/2; i++) { */
          /* holder = array[i]; */
          /* array[i] = array[end]; */
          /* array[end] = holder; */
          /* end --; */
      /* } */
  /* } */
  return array;
}

/* Returns a single position after the given position */
static pos_T *get_pos(buf_T *buf, char *ns, linenr_T row, colnr_T col, bool go_forward)
{

  int found = -1;
  if (go_forward == 1){
    found = 1;
  }
  int cmp;
  pos_T pos;
  pos.lnum = row;
  pos.col = col;
  FOR_EXTMARKS_IN_NS(buf, ns)
    cmp = pos_cmp(pos, extmark->fmark.mark);
    if (cmp == found) {
      return &prev->fmark.mark;
    }
    else if (cmp == 0) {
      return &extmark->fmark.mark;
    }
  END_FOR_EXTMARKS_IN_NS
  return NULL;
}

static bool extmark_create(buf_T *buf, char* ns, char *name, linenr_T row, colnr_T col)
{
  /* Initialize as first mark on this buffer*/
  if (buf->b_extmarks == NULL) {
    buf->b_extmarks = map_new(cstr_t, StringMap)();
    buf->b_extmarks_tree = kb_init(extmarks, KB_DEFAULT_SIZE);
  }

  StringMap ns_map;
  ns_map = map_get(cstr_t, StringMap)(buf->b_extmarks, (cstr_t)ns);

  /* Initialize a new namespace */
  /* if (!ns_map) { */
    /* ns_map = (StringMap *) xmalloc(sizeof(StringMap)); */
    map_put(cstr_t, StringMap)(buf->b_extmarks, (cstr_t)ns, ns_map);
  /* } */

  ExtendedMark *extmark = (ExtendedMark *) xmalloc(sizeof(ExtendedMark));
  extmark->fmark.mark.lnum = row;
  extmark->fmark.mark.col = col;
  extmark->names = pmap_new(cstr_t)();
  pmap_put(cstr_t)(extmark->names, (cstr_t)ns, name);

  kb_put(extmarks, buf->b_extmarks_tree,  *extmark);
  pmap_put(cstr_t)(&ns_map, (cstr_t)name, extmark);

  SET_FMARK(&extmark->fmark, extmark->fmark.mark, buf->b_fnum);
  // TODO do we need the timestamp and additional_data ??, also pos_t has 3 fields
  return OK;
}

static void extmark_update(ExtendedMark *extmark, buf_T *buf, char *ns, char *name, linenr_T row, colnr_T col)
{
  extmark->fmark.mark.lnum = row;
  extmark->fmark.mark.col = col;
}

static int extmark_delete(ExtendedMark *extmark, buf_T *buf, char *ns, char *name)
{
  /* Remove our key from the extmark */
  pmap_del(cstr_t)(extmark->names, (cstr_t)ns);

  /* Remove our key from the namespace */
  StringMap ns_map;
  ns_map = map_get(cstr_t, StringMap)(buf->b_extmarks, (cstr_t)ns);
  pmap_del(cstr_t)(&ns_map, (cstr_t)name);

  /* Delete the mark if there are no more namespaces using it */
  if (!kh_size(extmark->names->table)) {
    kb_del(extmarks, buf->b_extmarks_tree, *extmark);
  }

  /* Delete the namespace if empty */
  if (!kh_size(ns_map.table)) {
    pmap_del(cstr_t)(&ns_map, name);
  }
  return OK;
}

ExtendedMark *extmark_get(buf_T *buf, char *ns, char *name)
{
  if (!buf->b_extmarks) {
    return NULL;
  }
  StringMap ns_map;
  ns_map = map_get(cstr_t, StringMap)(buf->b_extmarks, (cstr_t)ns);
  if (!kh_size(ns_map.table)) {
    return NULL;
  }
  return pmap_get(cstr_t)(&ns_map, (cstr_t)name);
}

int pos_cmp(pos_T a, pos_T b)
{
  if (a.lnum < b.lnum) {
    return -1;
  }
  else if (a.lnum == b.lnum) {
    if (a.col < b.col) {
      return -1;
    }
    else if (a.col == b.col) {
      return 0;
    }
  }
  return 1;
}

void extmark_free_all(buf_T *buf)
{
  if (!buf->b_extmarks) {
    return;
  }
  map_free(cstr_t, StringMap)(buf->b_extmarks);
  kb_destroy(extmarks, buf->b_extmarks_tree);
}

//TODO  use from mark.c
#define _col_adjust(pp) \
  { \
    pos_T *posp = pp; \
    if (posp->lnum == lnum && posp->col >= mincol) \
    { \
      posp->lnum += lnum_amount; \
      assert(col_amount > INT_MIN && col_amount <= INT_MAX); \
      if (col_amount < 0 && posp->col <= (colnr_T)-col_amount) \
        posp->col = 0; \
      else \
        posp->col += (colnr_T)col_amount; \
    } \
  }

/* Adjust exmarks when changes to columns happen */
/* This is called from mark_col_adjust as well as */
/* from mark_adjust, and from wherever text edits happen */
void extmark_col_adjust(buf_T *buf, linenr_T lnum, colnr_T mincol, long lnum_amount, long col_amount)
{
  FOR_ALL_EXTMARKS(buf)
    _col_adjust(&(extmark->fmark.mark))
  END_FOR_ALL_EXTMARKS
}

 /* Adjust extmark row for inserted/deleted rows. */
void extmark_adjust(buf_T* buf, linenr_T line1, linenr_T line2, long amount, long amount_after)
{
  FOR_ALL_EXTMARKS(buf)
    if (extmark->fmark.mark.lnum >= line1
        && extmark->fmark.mark.lnum <= line2) {
          if (amount == MAXLNUM) {
            extmark->fmark.mark.lnum = line1;
          }
          else {
            extmark->fmark.mark.lnum += amount;
          }
    }
    else if (extmark->fmark.mark.lnum > line2)
        extmark->fmark.mark.lnum += amount_after;
  END_FOR_ALL_EXTMARKS
}


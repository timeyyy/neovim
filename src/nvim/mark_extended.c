/* A normal mark that you would find in other text editors
 * The marks exists seperatley to vim marks, there are no
 * special marks for insert cursort etc
 *
 * Marks are stored in a btree for fast searching
 * A map of pointers to the marks is used for fast lookup
 *
 * For moving marks see : http://blog.atom.io/2015/06/16/optimizing-an-important-atom-primitive.html
 *
 */

#include "nvim/vim.h"
#include "nvim/mark_extended.h"
#include "nvim/memory.h"
#include "nvim/globals.h"      // FOR_ALL_BUFFERS
#include "nvim/map.h"          // pmap ...
#include "nvim/lib/kbtree.h"   // kbitr ...

#ifdef INCLUDE_GENERATED_DECLARATIONS
# include "mark_extended.c.generated.h"
#endif

static uint64_t NAMESPACE_COUNTER = 0;

/* required before calling any other functions */
uint64_t extmark_ns_create(char *ns)
{
  if (!NAMESPACE_COUNTER) {
    NAMESPACES = map_new(cstr_t, uint64_t)();
  }
  else if(map_has(cstr_t, uint64_t)(NAMESPACES, ns)) {
    return FAIL;
  }
  NAMESPACE_COUNTER++;
  uint64_t id = NAMESPACE_COUNTER;
  map_put(cstr_t, uint64_t)(NAMESPACES, xstrdup(ns), id);
  return NAMESPACE_COUNTER;
}

bool ns_initialized(char *ns) {
  if (!NAMESPACES) {
    return 0;
  }
  return map_has(cstr_t, uint64_t)(NAMESPACES, ns);
}


/* Create or update an extmark */
/* returns 1 on new mark created */
/* returns 2 on succesful update */
int extmark_set(buf_T *buf, uint64_t ns, uint64_t id, linenr_T lnum, colnr_T col)
{
  ExtendedMark *extmark = extmark_from_id(buf, ns, id);
  if (!extmark){
    return extmark_create(buf, ns, id, lnum, col);
  }
  else {
    extmark_update(extmark, buf, ns, id, lnum,  col);
   return 2;
  }
}

/* returns 0 on missing id */
int extmark_unset(buf_T *buf, uint64_t ns, uint64_t id)
{
  ExtendedMark *extmark = extmark_from_id(buf, ns, id);
  if (!extmark){
    return FAIL;
  }
  return extmark_delete(extmark, buf, ns, id);
}

//TODO probably just use this directly in api
//TODO ORDER THE COLUMNS ...
/* Get all mark ids ordered by position*/
ExtmarkArray *extmark_ids(buf_T *buf, uint64_t ns)
{
  ExtmarkArray *array = xmalloc(sizeof(ExtmarkArray));
  kv_init(*array);
  FOR_ALL_EXTMARKS(buf, 0, -1)
    kv_push(*array, extmark);
  END_FOR_ALL_EXTMARKS
  return array;
}

/* Given a mark, finds the next mark */
ExtendedMark *extmark_next(buf_T *buf, uint64_t ns, linenr_T lnum, colnr_T col, bool match)
{
  return extmark_neighbour(buf, ns, lnum, col, 1, match);
}

/* Given a id finds the previous mark */
ExtendedMark *extmark_prev(buf_T *buf, uint64_t ns, linenr_T lnum, colnr_T col, bool match)
{
  return extmark_neighbour(buf, ns, lnum, col, 0, match);
}

ExtmarkArray *extmark_nextrange(buf_T *buf, uint64_t ns, linenr_T l_lnum,
                                colnr_T l_col, linenr_T u_lnum, colnr_T u_col)
{
  return extmark_neighbour_range(buf, ns, l_lnum, l_col, u_lnum, u_col, 1);
}

ExtmarkArray *extmark_prevrange(buf_T *buf, uint64_t ns, linenr_T l_lnum,
                                colnr_T l_col, linenr_T u_lnum, colnr_T u_col)
{
  return extmark_neighbour_range(buf, ns, l_lnum, l_col, u_lnum, u_col, 0);
}

/* Returns the position of marks between a range, */
/* marks found at the start or end index will be included, */
/* if upper_lnum or upper_col are negative the buffer */
/* will be searched to the start, or end */
/* go_forward can be set to control the order of the array */
static ExtmarkArray *extmark_neighbour_range(buf_T *buf, uint64_t ns,
                                             linenr_T l_lnum, colnr_T l_col,
                                             linenr_T u_lnum, colnr_T u_col,
                                             bool go_forward)
{
  ExtmarkArray *array = xmalloc(sizeof(ExtmarkArray));
  kv_init(*array);
  FOR_ALL_EXTMARKS(buf, l_lnum, u_lnum)
    if (extmark->ns_id == ns && extmark->col <= u_col) {
      kv_push(*array, extmark);
    }
  END_FOR_ALL_EXTMARKS
  /* Swap the elements in the array */
  /* if (!go_forward) { */
      /* ExtendedMark holder; */
      /* int n = (int)sizeof(array); */
      /* int end = n-1; */
      /* for (int i = 0; i < n/2; i++) { */
          /* holder = kv_A(*array, i); */
          /* array[i] = array[end]; */
          /* array[end] = holder; */
          /* end --; */
      /* } */
  /* } */
  return array;
}

/// Returns the mark, nearest to or including the passed in mark
/// @param input the desired position to be checked
/// @param go_forward flag to control next or prev
/// @param match a flag to include a mark that is at the postiion being queried
static ExtendedMark *extmark_neighbour(buf_T *buf, uint64_t ns, linenr_T lnum,
                                       colnr_T col, bool go_forward, bool match)
{
  if (go_forward) {
    FOR_ALL_EXTMARKS(buf, lnum, lnum)
      if (extmark->ns_id == ns) {
        /* return extmark; */
        if (extmark->col > col) {
          return extmark;
        } else if (match && extmark->col == col) {
          return extmark;
        }
      }
    END_FOR_ALL_EXTMARKS
  }
  /* else { */
  /* prev */
  /* } */
  return NULL;
}

static bool extmark_create(buf_T *buf, uint64_t ns, uint64_t id, linenr_T lnum, colnr_T col)
{
  if (!buf->b_extlines) {
    buf->b_extlines = kb_init(extlines);
    buf->b_extmark_ns = pmap_new(uint64_t)();
  }
  ExtmarkNs *ns_obj = NULL;
  ns_obj = pmap_get(uint64_t)(buf->b_extmark_ns, ns);
  /* Initialize a new namespace for this buffer*/
  if (!ns_obj) {
    ns_obj = xmalloc(sizeof(ExtmarkNs));
    ns_obj->map = pmap_new(uint64_t)();
    pmap_put(uint64_t)(buf->b_extmark_ns, ns, ns_obj);
  }

  ExtMarkLine *extline = extline_ref(buf->b_extlines, lnum, true);
	extline->lnum = lnum;
  ExtendedMark *extmark = kv_pushp(extline->items);
	extmark->line = extline;
  extmark->ns_id = ns;
  extmark->mark_id = id;
  extmark->col = col;

  pmap_put(uint64_t)(ns_obj->map, id, extmark);

  return OK;
}

static void extmark_update(ExtendedMark *extmark, buf_T *buf, uint64_t ns, uint64_t id, linenr_T lnum, colnr_T col)
{
  extmark->col = col;
  extmark->line->lnum = lnum;
}

static int extmark_delete(ExtendedMark *input, buf_T *buf, uint64_t ns, uint64_t id)
{
  /* Remove our key from the namespace */
  ExtmarkNs *ns_obj = pmap_get(uint64_t)(buf->b_extmark_ns, ns);
  pmap_del(uint64_t)(ns_obj->map, id);

  int i=0;
	FOR_ALL_EXTMARKS(buf, input->line->lnum, input->line->lnum)
    if (extmark->ns_id == ns) {
      /* delete mark from list */
      kv_A(extline->items, i) = kv_pop(extline->items);
    }
    i++;
  END_FOR_ALL_EXTMARKS
  /* kb_del(extlines, buf->b_extlines, extmark->line); */
  /* xfree(extmark); // wasn't required before change to storing pointers */
  return OK;
}

ExtendedMark *extmark_from_id(buf_T *buf, uint64_t ns, uint64_t id)
{
  if (!buf->b_extlines) {
    return NULL;
  }
  ExtmarkNs *ns_obj = pmap_get(uint64_t)(buf->b_extmark_ns, ns);
  if (!ns_obj || !kh_size(ns_obj->map->table)) {
    return NULL;
  }
  return pmap_get(uint64_t)(ns_obj->map, id);
}

ExtendedMark *extmark_from_pos(buf_T *buf, uint64_t ns, linenr_T lnum, colnr_T col)
{
  if (!buf->b_extlines) {
    return NULL;
  }
  FOR_ALL_EXTMARKS(buf, lnum, lnum)
    if (extmark->ns_id == ns) {
      if (extmark->col == col) {
        return extmark;
      }
    }
  END_FOR_ALL_EXTMARKS
  return NULL;
}

void extmark_free_all(buf_T *buf)
{
  if (!buf->b_extlines) {
    return;
  }
  pmap_free(uint64_t)(buf->b_extmark_ns);
  /* FOR_ALL_EXTMARKS(buf) // wasn't required beforestoring pointers in kbtree */
    /* kb_del(extlines, extmark); */
  /* END_FOR_ALL_EXTMARKS */
  kb_destroy(extlines, buf->b_extlines);
}
#define _one_adjust(add) \
  { \
    lp = add; \
    if (*lp >= line1 && *lp <= line2) \
    { \
      if (amount == MAXLNUM) \
        *lp = 0; \
      else \
        *lp += amount; \
    } \
    else if (amount_after && *lp > line2) \
      *lp += amount_after; \
  }

#define _one_adjust_nodel(add) \
  { \
    lp = add; \
    if (*lp >= line1 && *lp <= line2) \
    { \
      if (amount == MAXLNUM) \
        *lp = line1; \
      else \
        *lp += amount; \
    } \
    else if (amount_after && *lp > line2) \
      *lp += amount_after; \
  }


#define _col_adjust(m_lnum, m_col) \
  { \
    lp = m_lnum; \
    cp = m_col; \
    if (*lp == lnum && *cp >= mincol) \
    { \
      *lp += lnum_amount; \
      assert(col_amount > INT_MIN && col_amount <= INT_MAX); \
      if (col_amount < 0 && *cp <= (colnr_T)-col_amount) \
        *cp = 0; \
      else \
        *cp += (colnr_T)col_amount; \
    } \
  }

/* extmark_adjust will invalidate marks, When this happens */
/* the function may need to do some repair and call col adjust
/* Adjust exmarks when changes to columns happen */
/* We only need to adjust marks on the same line */
/* This is called from mark_col_adjust as well as */
/* from mark_adjust, and from wherever text edits happen */
void extmark_col_adjust(buf_T *buf, linenr_T lnum, colnr_T mincol, long lnum_amount, long col_amount)
{
  linenr_T start;
  linenr_T end;
  if ((lnum + lnum_amount) < lnum) {
    start = lnum +lnum_amount;
    end = lnum;
  }
  else {
    start = lnum;
    end = lnum +lnum_amount;
  }

  linenr_T *lp;
  colnr_T *cp;
  FOR_ALL_EXTMARKS(buf, start, end)
    /* _col_adjust(&(extmark->line->lnum), &(extmark->col)); */
    lp = &(extmark->line->lnum);
    cp = &(extmark->col);
    if (*lp == lnum && *cp >= mincol)
    {
      *lp += lnum_amount;
      assert(col_amount > INT_MIN && col_amount <= INT_MAX);
      if (col_amount < 0 && *cp <= (colnr_T)-col_amount)
        *cp = 0;
      else
        *cp += (colnr_T)col_amount;
    }
  END_FOR_ALL_EXTMARKS
}

/*
 * Adjust marks between line1 and line2 (inclusive) to move 'amount' lines.
 * Must be called before changed_*(), appended_lines() or deleted_lines().
 * May be called before or after changing the text.
 * When deleting lines line1 to line2, use an 'amount' of MAXLNUM: The marks
 * within this range are made invalid.
 * If 'amount_after' is non-zero adjust marks after line2.
 * Example: Delete lines 34 and 35: mark_adjust(34, 35, MAXLNUM, -2);
 * Example: Insert two lines below 55: mark_adjust(56, MAXLNUM, 2, 0);
 *				   or: mark_adjust(56, 55, MAXLNUM, 2);
 */
 /* Adjust extmark row for inserted/deleted rows. */
void extmark_adjust(buf_T* buf, linenr_T line1, linenr_T line2, long amount, long amount_after)
{
  linenr_T *lp;
  FOR_ALL_EXTMARKLINES(buf)
    /* _one_adjust(&(extline->lnum)); */
    lp = &(extline->lnum);
    if (*lp >= line1 && *lp <= line2)
    {
      if (amount == MAXLNUM)
        *lp = 0;
      else
        *lp += amount;
    }
    else if (amount_after && *lp > line2)
      *lp += amount_after;

  END_FOR_ALL_EXTMARKLINES
}

ExtendedMark *extline_ref(kbtree_t(extlines) *b, linenr_T lnum, bool put)
{
  ExtMarkLine t, *p , **pp;
  t.lnum = lnum;
  pp = kb_get(extlines, b, &t);
  // IMPORTANT: put() only works if key is absent
  if (pp) {
      return *pp;
  } else if (!put) {
      return NULL;
  }
  p = xcalloc(sizeof(*p),1);
  p->lnum = lnum;
  // p->items zero initialized
  kb_put(extlines, b, p);
  return p;
}

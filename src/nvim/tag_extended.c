//
// tag_extended.c --
//
//	This module implements the "tag" subcommand of the widget command for
//	text widgets, plus most of the other high-level functions related to
//	tags.
//

// TODO stack order tag_raise, tag_lower
// TODO namespaces
// TODO nextrange
// TODO prevrange

#include "nvim/vim.h"
#include "nvim/tag_extended.h"
#include "nvim/mark_extended.h"
#include "nvim/memory.h"
#include "nvim/globals.h"      // FOR_ALL_BUFFERS
#include "nvim/map.h"          // pmap ...
#include "nvim/lib/kbtree.h"   // kbitr ...

#ifdef INCLUDE_GENERATED_DECLARATIONS
# include "tag_extended.c.generated.h"
#endif


static uint64_t namespace_counter = 0;


// Required before calling any other functions
uint64_t exttag_ns_create(char *ns)
{
  if (!namespace_counter) {
    EXTTAG_NAMESPACES = map_new(uint64_t, cstr_t)();
  }
  // Ensure the namespace is unique
  cstr_t value;
  map_foreach_value(EXTTAG_NAMESPACES, value, {
    if (STRCMP(ns, value) == 0) {
      return FAIL;
    }
  });
  namespace_counter++;
  uint64_t id = namespace_counter;
  map_put(uint64_t, cstr_t)(EXTTAG_NAMESPACES, id, xstrdup(ns));
  return namespace_counter;
}


bool exttag_ns_initialized(uint64_t ns) {
  if (!EXTTAG_NAMESPACES) {
    return 0;
  }
  return map_has(uint64_t, cstr_t)(EXTTAG_NAMESPACES, ns);
}


// Create or update an exttag
//
// Returns 1 on success
// Returns 0 on fail
int HITS = 0;
int exttag_set(buf_T *buf,
               uint64_t ns,
               uint64_t id,
               linenr_T l_lnum,
               colnr_T l_col,
               linenr_T u_lnum,
               colnr_T u_col,
               ExtmarkReverse undo)
{
  HITS++;
  int rv = exttag_create(buf, ns, id, l_lnum, l_col, u_lnum, u_col, undo);
  return rv;
  /* return 1; */
}


static int exttag_create(buf_T *buf,
                          uint64_t ns,
                          uint64_t id,
                          linenr_T l_lnum,
                          colnr_T l_col,
                          linenr_T u_lnum,
                          colnr_T u_col,
                          ExtmarkReverse undo)
{
  if (!buf->b_exttag_ns) {
    buf->b_exttag_ns = pmap_new(uint64_t)();
  }
  ExtTagNs *ns_obj = NULL;
  ns_obj = pmap_get(uint64_t)(buf->b_exttag_ns, ns);
  // Initialie a new namespace for this buffer
  if (!ns_obj) {
    ns_obj = xmalloc(sizeof(ExtTagNs));
    ns_obj->tag_groups = pmap_new(uint64_t)();
    pmap_put(uint64_t)(buf->b_exttag_ns, ns, ns_obj);
  }

  ExtTagGroup *exttag_group = NULL;
  exttag_group = pmap_get(uint64_t)(ns_obj->tag_groups, id);
  // Initialize a new exttag_group
  if (!exttag_group) {
    exttag_group = xcalloc(sizeof(ExtTagGroup), 1);
    // exttag_group->extlines Zero initialized
    exttag_group->id = id;
    exttag_group->extmark_ns = init_extmark_ns(ns, id);
    pmap_put(uint64_t)(ns_obj->tag_groups, id, exttag_group);
  }

  // Create the marks
  uint64_t extmark_ns = exttag_group->extmark_ns;
  uint64_t lower = extmark_next_id_get(buf, extmark_ns);
  extmark_create(buf, extmark_ns, lower, l_lnum, l_col, undo);
  uint64_t upper = extmark_next_id_get(buf, extmark_ns);
  extmark_create(buf, extmark_ns, upper, u_lnum, u_col, undo);

  // create or get a line
    /* return exttag_bad(&exttag_group->extlines, l_lnum); */
  /* } */
  ExtTagLine *extline = exttag_ref(&exttag_group->extlines, l_lnum);
  // put the tag on the line
  exttag_put(&extline->items, extline, lower, upper, &exttag_group->extmark_ns);
  return 1;
  /* return true; */
}


// Returns 0 on empty tag group or no tag in range
// Returns 1 on successful delete
// Returns 2 on invalid id
int exttag_unset(buf_T *buf,
                 uint64_t ns,
                 uint64_t id,
                 linenr_T l_lnum,
                 colnr_T l_col,
                 linenr_T u_lnum,
                 colnr_T u_col)
{
  ExtTagGroup *exttag_group = exttag_group_from_id(buf, ns, id);
  if (!exttag_group) {
    return 2;
  }

  if (exttag_delete(buf, exttag_group, l_lnum, l_col, u_lnum, u_col)) {
    return 1;
  } else {
    return 0;
  }
}

// Returns the position of tags between a range,
// tags found at the start or end index will be included,
// if tag_name is -1 return all tags from all tag groups
// if upper_lnum or upper_col are negative the buffer
// will be searched to the start, or end
// dir can be set to control the order of the array
// amount = amount of tags to find or -1 for all
ExtmarkArray exttag_get(buf_T *buf,
                       uint64_t ns,
                       uint64_t tag_name,
                       linenr_T l_lnum,
                       colnr_T l_col,
                       linenr_T u_lnum,
                       colnr_T u_col,
                       int64_t amount,
                       int dir)
{
  ExtmarkArray array = KV_INITIAL_VALUE;
  ExtendedMark *l_mark;
  ExtendedMark *u_mark;
  int n = 0;
  // get only Tags in the tag_name group
    ExtTagGroup *exttag_group = exttag_group_from_id(buf, ns, tag_name);
    /* FOR_ALL_TAGS_IN_TAGGROUP(exttag_group, l_lnum, l_col, u_lnum, u_col, { */
      /* l_mark = extmark_from_id(buf, exttag_group->extmark_ns, exttag.l_id); */
      /* u_mark = extmark_from_id(buf, exttag_group->extmark_ns, exttag.u_id); */
      /* n++; */
      /* kv_push(array, l_mark); */
      /* kv_push(array, u_mark); */

      /* if (n == amount) { */
        /* return array; */
      /* } */
    /* }) */

      ExtendedMark *_l_mark;
      ExtendedMark *_u_mark;

      kbitr_t(tagitems) mitr;
      ExtTag mt;
      mt.line = NULL;

        kbitr_t(taglines) itr;
        ExtTagLine t;
        t.lnum = l_lnum;
        if(!kb_itr_get(taglines, &(exttag_group->extlines), &t, &itr)) {
            kb_itr_next(taglines, &(exttag_group->extlines), &itr);
        }
        ExtTagLine *extline;
        for (; kb_itr_valid(&itr); kb_itr_next(taglines, &(exttag_group->extlines), &itr)) { 
          extline = kb_itr_key(&itr);
          if (extline->lnum > u_lnum && u_lnum != -1){ 
            break;
          } 

          if(!kb_itr_get(tagitems, &extline->items, mt, &mitr)) { 
              kb_itr_next(tagitems, &extline->items, &mitr);
          }
          ExtTag exttag;
          for (; kb_itr_valid(&mitr); kb_itr_next(tagitems, &extline->items, &mitr)) {
            exttag = kb_itr_key(&mitr);
                // INNTER CODE
                l_mark = extmark_from_id(buf, exttag_group->extmark_ns, exttag.l_id);
                u_mark = extmark_from_id(buf, exttag_group->extmark_ns, exttag.u_id);
                n++;
                kv_push(array, l_mark);
                kv_push(array, u_mark);

                if (n == amount) {
                  return array;
                }
                // INNERT COE
          }
        }

  return array;
}


static bool exttag_delete(buf_T *buf,
                          ExtTagGroup *exttag_group,
                          linenr_T l_lnum,
                          colnr_T l_col,
                          linenr_T u_lnum,
                          colnr_T u_col)
{
  bool deleted = false;
  FOR_ALL_TAGS_IN_TAGGROUP(exttag_group,
                                 l_lnum,
                                 l_col,
                                 u_lnum,
                                 u_col,
                                 {
    deleted = true;

    // delete tag from from the line
    kb_del(tagitems, &extline->items, exttag);

    // Remove the line if there are no more tags in the line
    if (kb_size(&extline->items) == 0) {
      kb_del(taglines, &exttag_group->extlines, extline);
    }
  })

  if (deleted) {
    return true;
  }
  return false;
}



ExtTagGroup *exttag_group_from_id(buf_T *buf, uint64_t ns, uint64_t id)
{
  if (!buf->b_exttag_ns) {
    return NULL;
  }
  ExtTagNs *ns_obj = pmap_get(uint64_t)(buf->b_exttag_ns, ns);
  if (!ns_obj || !kh_size(ns_obj->tag_groups->table)) {
    return NULL;
  }
  ExtTagGroup *exttag_group = pmap_get(uint64_t)(ns_obj->tag_groups, id);
  if (!exttag_group) {
    return NULL;
  }

  return pmap_get(uint64_t)(ns_obj->tag_groups, id);
}


static char exttag_prefix[] = "__extTags__\0";
static char seperator[] = "-\0";
static uint64_t init_extmark_ns(uint64_t tag_ns, uint64_t id)
{
  // make a ns string to be used by the exttag group
  char str_tag_ns[128];
  char str_id[128];
  /* sprintf(str_tag_ns, "%lu", tag_ns); */
  /* sprintf(str_id, "%lu", id); */
  /* char *ns_str = STRCAT(exttag_prefix, seperator); */
  // TODO(timeyyy): this doesn't produce desired result..
  /* sprintf(ns_str, "%c", "\0"); */
  /* ns_str = STRCAT(ns_str, str_tag_ns); */
  /* ns_str = STRCAT(ns_str, seperator); */
  /* ns_str = STRCAT(ns_str, str_id); */
  /* ns_str = STRCAT(ns_str, "\0"); */

  // get the integer representation of the namespace
  /* return extmark_ns_create(ns_str); */
  return extmark_ns_create(exttag_prefix);
}

void exttag_free_all(buf_T *buf)
{
  if (!buf->b_exttag_ns) {
    return;
  }

  ExtTagNs *ns_obj;
  IntMap *tag_groups;
  ExtTagGroup *tag_group;
  map_foreach_value(buf->b_exttag_ns, ns_obj, {
    map_foreach_value(ns_obj->tag_groups, tag_groups, {
      map_foreach_value(tag_groups, tag_group, {
        xfree(tag_group->extmark_ns);
        FOR_ALL_EXTTAGLINES(tag_group, 0, -1, {
          xfree(extline);
        })
        xfree(tag_group);
      })
      pmap_free(uint64_t)(tag_groups);
    })
    pmap_free(uint64_t)(ns_obj->tag_groups);
  })
  pmap_free(uint64_t)(buf->b_exttag_ns);
  // TODO, free mark namespaces create for the tag groups?
}

ExtTagLine *exttag_ref(kbtree_t(taglines) *b, linenr_T lnum)
{
  ExtTagLine t, *p, **pp;
  t.lnum = lnum;

  pp = kb_get(taglines, b, &t);
  if (!pp) {
    p = xcalloc(sizeof(*p), 1);
    p->lnum = lnum;
    // p->items zero initialized
    kb_put(taglines, b, p);
    return p;
  }
  // Return existing
  return *pp;
}

/* int exttag_bad(kbtree_t(taglines) *b, linenr_T lnum) */
/* { */
  /* ExtTagLine t, *p , **pp; */
  /* t.lnum = lnum; */
  /* pp = kb_get(taglines, b, &t); */
  /* if (!pp) { */
    /* p = xcalloc(sizeof(*p), 1); */
    /* p->lnum = lnum; */
    // p->items zero initialized
    /* kb_put(taglines, b, p); */
    /* return p; */
  /* } */
  // Return existing
  /* return *pp; */
/* } */

void exttag_put(kbtree_t(tagitems) *b,
                ExtTagLine *extline,
                uint64_t l_id,
                uint64_t u_id,
                uint64_t *extmark_ns)
{
  ExtTag t, *p;
  t.l_id = l_id;
  t.u_id = u_id;
  t.extmark_ns = extmark_ns;
  t.line = extline;

  p = kb_get(tagitems, b, t);
  assert(!p);

  kb_put(tagitems, b, t);
}

// Used by the btree for insterting tags
int tag_cmp(ExtTag a, ExtTag b) {
  // Test that the tag has been initialized
  // TODO try !a->line, values should be 0 initialized
  if (!a.line) {
    return -1;
  } else if (!b.line) {
    return 1;
  }

  // TODO(timeyyy): is this always correct??
  buf_T *buf = curbuf;
  ExtendedMark *a_l_mark = extmark_from_id(buf, *a.extmark_ns, a.l_id);
  ExtendedMark *a_u_mark = extmark_from_id(buf, *b.extmark_ns, a.u_id);
  ExtendedMark *b_l_mark = extmark_from_id(buf, *b.extmark_ns, b.l_id);
  ExtendedMark *b_u_mark = extmark_from_id(buf, *b.extmark_ns, b.u_id);

  // We only need to compare columns as as lines live in a different trees
  int lower_cmp = kb_generic_cmp(a_l_mark->col, b_l_mark->col);
  // The mark id is unique so compare on that
  if (lower_cmp != 0) {
    return lower_cmp;
  }
  int upper_cmp = kb_generic_cmp(a_u_mark->col, b_u_mark->col);
  if (upper_cmp != 0) {
    return upper_cmp;
  }

  // Compare on the mark id's are they are guranteed unique
  return kb_generic_cmp(a_l_mark->mark_id, b_l_mark->mark_id);
}

int col_cmp(colnr_T a, colnr_T b)
{
  if (a < b) {
    return -1;
  }
  else if (a == b) {
    return 0;
  }
  return 1;
}

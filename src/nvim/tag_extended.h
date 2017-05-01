#ifndef NVIM_EXTTAG_H
#define NVIM_EXTTAG_H

/* #include "nvim/tag_extended_defs.h" */
#include "nvim/lib/kbtree.h"
#include "nvim/lib/kvec.h"
#include "nvim/map.h"
#include "nvim/mark_extended.h"

#define FOR_ALL_EXTTAGLINES(exttag_group, start, end, code)\
  kbitr_t(taglines) itr;\
  ExtTagLine t;\
  t.lnum = start;\
  if (!kb_itr_get(taglines, &(exttag_group->extlines), &t, &itr)) { \
      kb_itr_next(taglines, &(exttag_group->extlines), &itr);\
  } \
  ExtTagLine *extline;\
  for (; kb_itr_valid(&itr); kb_itr_next(taglines, &(exttag_group->extlines), &itr)) { \
    extline = kb_itr_key(&itr);\
    if (extline->lnum > end && end != -1){ \
      break;\
    } \
    code;\
  }

// Defines exttag variable and runs the codeblock
#define _FOR_ALL_TAGS_IN_TAGGROUP(exttag_group, start, end,  code)\
  kbitr_t(tagitems) mitr;\
  ExtTag mt;\
  mt.line = NULL;\
  FOR_ALL_EXTTAGLINES(exttag_group, start, end, { \
    if (!kb_itr_get(tagitems, &extline->items, mt, &mitr)) { \
        kb_itr_next(tagitems, &extline->items, &mitr);\
    } \
    ExtTag exttag;\
    for (; kb_itr_valid(&mitr); kb_itr_next(tagitems, &extline->items, &mitr)) { \
      exttag = kb_itr_key(&mitr);\
      code;\
    }\
  })


// Make more efficent TODO using kbtree builtins
#define FOR_ALL_TAGS_IN_TAGGROUP(exttag_group, \
                                 l_lnum,\
                                 l_col,\
                                 u_lnum,\
                                 u_col,\
                                 code)\
  ExtendedMark *_l_mark;\
  ExtendedMark *_u_mark;\
  _FOR_ALL_TAGS_IN_TAGGROUP(exttag_group, l_lnum, u_lnum, {\
    _l_mark = extmark_from_id(buf, exttag_group->extmark_ns, exttag.l_id);\
    _u_mark = extmark_from_id(buf, exttag_group->extmark_ns, exttag.u_id);\
    if (col_cmp(l_col, _l_mark->col) != -1\
        && col_cmp(_u_mark->col, u_col) != -1) {\
       code;\
    }\
  });


#define FOR_ALL_TAGS_AT_INDEX(ns, lnum, col, code)\
  ExtTagNs *ns_obj = pmap_get(uint64_t)(buf->b_exttag_ns, ns);\
  ExtTagGroup *exttag_group;\
  /* For All TagGroups */\
  map_foreach_value(ns_obj->tag_groups, exttag_group, {\
    /* For All Tags in Group */ \
    FOR_ALL_TAGS_IN_TAGGROUP(exttag_group,\
                             lnum,\
                             col,\
                             lnum,\
                             col,\
                             {\
      code;\
    });\
  });

#define FOR_ALL_TAGS_IN_RANGE(ns, l_lnum, l_col, u_lnum, u_col, code)\
  ExtTagNs *ns_obj = pmap_get(uint64_t)(buf->b_exttag_ns, ns);\
  ExtTagGroup *exttag_group;\
  map_foreach_value(ns_obj->tag_groups, exttag_group, {\
    FOR_ALL_TAGS_IN_TAGGROUP(exttag_group,\
                             l_lnum,\
                             l_col,\
                             u_lnum,\
                             u_col,\
                             {\
      code;\
    });\
  });


struct ExtTagLine;

typedef struct ExtTag
{
  uint64_t l_id;
  uint64_t u_id;
  uint64_t *extmark_ns;
  struct ExtTagLine *line;
} ExtTag;


int tag_cmp(ExtTag a, ExtTag b);

#define tagitems_cmp(a, b) (tag_cmp((a), (b)))
KBTREE_INIT(tagitems, ExtTag, tagitems_cmp, 10)

typedef struct ExtTagLine
{
  linenr_T lnum;
  kbtree_t(tagitems) items;
} ExtTagLine;


KBTREE_INIT(taglines, ExtTagLine *, extline_cmp, 10)


typedef struct ExtTagGroup
{
  uint64_t id;
  uint64_t extmark_ns; // Used to do range lookups in the group
  kbtree_t(taglines) extlines;
} ExtTagGroup;


_NS *EXTTAG_NAMESPACES;


typedef struct ExtTagNs {
  IntMap *tag_groups;
} ExtTagNs;


#ifdef INCLUDE_GENERATED_DECLARATIONS
# include "tag_extended.h.generated.h"
#endif

#endif  // NVIM_EXTTAG_H

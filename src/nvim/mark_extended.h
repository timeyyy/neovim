#ifndef NVIM_EXTMARK_H
#define NVIM_EXTMARK_H

#include "nvim/mark_extended_defs.h"
#include "nvim/lib/kbtree.h"
#include "nvim/lib/kvec.h"
#include "nvim/map.h"

#define extmark_pos_cmp(a, b) (pos_cmp((a)->mark, (b)->mark))

#define FOR_ALL_EXTMARKS(buf) \
  if (buf->b_extmarks) { \
      kbitr_t(extmarks) itr; \
      ExtendedMark t = {1}; \
      if(!kb_itr_get(extmarks, buf->b_extmarks, &t, &itr)) { \
        kb_itr_next(extmarks, buf->b_extmarks, &itr); \
      } \
      ExtendedMark *extmark; \
        for (;kb_itr_valid(&itr); kb_itr_next(extmarks, buf->b_extmarks, &itr)) { \
                 extmark = kb_itr_key(ExtendedMark*, &itr); \

#define END_FOR_ALL_EXTMARKS }}

typedef struct ExtendedMark {
  uint64_t id;
  /* kvec_t(uint64_t) id; */
  pos_T mark;
} ExtendedMark, *ExtendedMarkPtr;

typedef kvec_t(ExtendedMark*) ExtmarkArray;
typedef PMap(uint64_t) IntMap;
KBTREE_INIT(extmarks, ExtendedMark*, extmark_pos_cmp, 10)
/* KBTREE_INIT(extmark, ExtMarkLine*, extline_cmp) */

/* All namesspace that exist in nvim */
typedef Map(cstr_t, uint64_t) _NS;
_NS *NAMESPACES;

typedef struct ExtmarkNs {
  IntMap *map;
  kbtree_t(extmarks) *tree;
} ExtmarkNs;

#ifdef INCLUDE_GENERATED_DECLARATIONS
# include "mark_extended.h.generated.h"
#endif

#endif  // NVIM_EXTMARK_H

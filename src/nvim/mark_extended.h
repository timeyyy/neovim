#ifndef NVIM_EXTMARK_H
#define NVIM_EXTMARK_H

#include "nvim/lib/kbtree.h"
#include "nvim/lib/kvec.h"
#include "nvim/map.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
/* #pragma GCC diagnostic ignored "-Wconversion" */
/* #pragma GCC diagnostic ignored "-Woverflow" */
/* #pragma GCC diagnostic ignored "-Wsign-conversion" */
/* #pragma GCC diagnostic ignored "-Wunused-function" */
#pragma GCC diagnostic ignored "-Wstrict-prototypes"
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#define extline_cmp(a, b) (((a)->lnum - (b)->lnum))

#define FOR_ALL_EXTMARKLINES(buf)\
  if (buf->b_extlines) {\
      kbitr_t(extlines) itr;\
      ExtMarkLine t = {1};\
      if(!kb_itr_get(extlines, buf->b_extlines, &t, &itr)) {\
        kb_itr_next(extlines, buf->b_extlines, &itr);\
      }\
      ExtMarkLine *extline; \
      for (;kb_itr_valid(&itr); kb_itr_next(extlines, buf->b_extlines, &itr)){\
        extline = kb_itr_key(ExtMarkLine*, &itr);\

#define END_FOR_ALL_EXTMARKLINES }}


#define FOR_ALL_EXTMARKS(buf, start, end)\
  if (buf->b_extlines) {\
      kbitr_t(extlines) itr;\
      ExtMarkLine t = {start};\
      if(!kb_itr_get(extlines, buf->b_extlines, &t, &itr)) {\
        kb_itr_next(extlines, buf->b_extlines, &itr);\
      }\
      ExtendedMark *extmark;\
      ExtMarkLine *extline;\
      for (;kb_itr_valid(&itr); kb_itr_next(extlines, buf->b_extlines, &itr)){\
        extline = kb_itr_key(ExtMarkLine*, &itr);\
        if (extline->lnum > end && end != -1){\
          break;\
        }\
        size_t marks = kv_size(extline->items);\
        for (size_t _i=0; _i<marks; _i++) {\
          extmark = &kv_A(extline->items, _i);\

#define END_FOR_ALL_EXTMARKS }}}

struct ExtMarkLine;

typedef struct ExtendedMark
{
  uint64_t ns_id;
  uint64_t mark_id;
  struct ExtMarkLine *line;
  colnr_T col;
} ExtendedMark;

typedef kvec_t(ExtendedMark) extmark_vec_t;

typedef struct ExtMarkLine
{
    linenr_T lnum;
    extmark_vec_t items;
}ExtMarkLine;


typedef kvec_t(ExtendedMark*) ExtmarkArray;
typedef PMap(uint64_t) IntMap;
KBTREE_INIT(extlines, ExtMarkLine*, extline_cmp, 10)

/* All namespaces that exist in nvim */
typedef Map(cstr_t, uint64_t) _NS;
_NS *NAMESPACES;

typedef struct ExtmarkNs {
  IntMap *map;
} ExtmarkNs;

#ifdef INCLUDE_GENERATED_DECLARATIONS
# include "mark_extended.h.generated.h"
#endif

#endif  // NVIM_EXTMARK_H

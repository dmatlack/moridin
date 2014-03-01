/**
 * @file list.h
 *
 */

#define list_typedef(elem_type)                                               \
  typedef struct {                                                            \
    elem_type *head;                                                          \
    elem_type *tail;                                                          \
    int size;                                                                 \
  }


#define list_link(elem_type)                                                  \
  struct {                                                                    \
    elem_type *prev;                                                          \
    elem_type *next;                                                          \
  }

#define DECLARE_LIST_LINK                                                     \
  {                                                                           \
    .prev = NULL,                                                             \
    .next = NULL                                                              \
  }


#define list_init(list_ptr)                                                   \
  do {                                                                        \
    (list_ptr)->head = NULL;                                                  \
    (list_ptr)->tail = NULL;                                                  \
    (list_ptr)->size = 0;                                                     \
  } while (0)

#define DECLARE_LIST                                                          \
  {                                                                           \
    .head = NULL,                                                             \
    .tail = NULL,                                                             \
    .size = 0                                                                 \
  }

#define list_swap_ptr(list_to_ptr, list_from_ptr)                             \
  do {                                                                        \
    (list_to_ptr)->head = (list_from_ptr)->head;                              \
    (list_from_ptr)->head = NULL;                                             \
    (list_to_ptr)->tail = (list_from_ptr)->tail;                              \
    (list_from_ptr)->tail = NULL;                                             \
    (list_to_ptr)->size = (list_from_ptr)->size;                              \
    (list_from_ptr)->size = 0;                                                \
  } while (0)

#define list_head(list_ptr)                   ((list_ptr)->head)
#define list_tail(list_ptr)                   ((list_ptr)->tail)
#define list_size(list_ptr)                   ((list_ptr)->size)
#define list_next(elem_ptr, link_name)        ((elem_ptr)->link_name.next)
#define list_prev(elem_ptr, link_name)        ((elem_ptr)->link_name.prev)


#define list_elem_init(elem_ptr, link_name)                                   \
  do {                                                                        \
    list_next(elem_ptr, link_name) = NULL;                                    \
    list_prev(elem_ptr, link_name) = NULL;                                    \
  } while (0)


#define list_insert_head(list_ptr, elem_ptr, link_name)                       \
  do {                                                                        \
    list_next(elem_ptr, link_name) = list_head(list_ptr);                     \
    list_prev(elem_ptr, link_name) = NULL;                                    \
    if (list_head(list_ptr) == NULL) {                                        \
      list_tail(list_ptr) = (elem_ptr);                                       \
    }                                                                         \
    else {                                                                    \
      list_prev(list_head(list_ptr), link_name) = (elem_ptr);                 \
    }                                                                         \
    list_head(list_ptr) = (elem_ptr);                                         \
    list_size(list_ptr)++;                                                    \
  } while (0)


#define list_insert_tail(list_ptr, elem_ptr, link_name)                       \
  do {                                                                        \
    list_prev(elem_ptr, link_name) = list_tail(list_ptr);                     \
    list_next(elem_ptr, link_name) = NULL;                                    \
    if (list_tail(list_ptr) == NULL) {                                        \
      list_head(list_ptr) = (elem_ptr);                                       \
    }                                                                         \
    else {                                                                    \
      list_next(list_tail(list_ptr), link_name) = (elem_ptr);                 \
    }                                                                         \
    list_tail(list_ptr) = (elem_ptr);                                         \
    list_size(list_ptr)++;                                                    \
  } while (0)


#define list_insert_after(list_ptr, elem_inq, elem_new, link_name)            \
  do {                                                                        \
    if (list_next(elem_inq, link_name)!= NULL) {                              \
      list_prev(list_next(elem_inq, link_name), link_name) = (elem_new);      \
    }                                                                         \
    list_next(elem_new, link_name) = list_next(elem_inq, link_name);          \
    list_next(elem_inq, link_name) = (elem_new);                              \
    list_prev(elem_new, link_name) = (elem_inq);                              \
    if ((elem_inq) == list_tail(list_ptr)) {                                  \
      list_tail(list_ptr) = (elem_new);                                       \
    }                                                                         \
    list_size(list_ptr)++;                                                    \
  } while (0)


#define list_insert_before(list_ptr, elem_inq, elem_new, link_name)           \
  do {                                                                        \
    if (list_prev(elem_inq, link_name) != NULL) {                             \
      list_next(list_prev(elem_inq, link_name), link_name) = (elem_new);      \
    }                                                                         \
    list_prev(elem_new, link_name) = list_prev(elem_inq, link_name);          \
    list_prev(elem_inq, link_name) = (elem_new);                              \
    list_next(elem_new, link_name) = (elem_inq);                              \
    if ((elem_inq) == list_head(list_ptr)) {                                  \
      list_head(list_ptr) = (elem_new);                                       \
    }                                                                         \
    list_size(list_ptr)++;                                                    \
  } while (0)


#define list_remove(list_ptr, elem_ptr, link_name)                            \
  do {                                                                        \
    if ((list_prev(elem_ptr, link_name)) != NULL) {                           \
      list_next(list_prev(elem_ptr, link_name), link_name) =                  \
        list_next(elem_ptr, link_name);                                       \
    }                                                                         \
    else {                                                                    \
      list_head(list_ptr) = list_next(elem_ptr, link_name);                   \
      if (list_head(list_ptr) != NULL) {                                      \
        list_prev(list_head(list_ptr), link_name) = NULL;                     \
      }                                                                       \
    }                                                                         \
    if ((list_next(elem_ptr, link_name)) != NULL) {                           \
      list_prev(list_next(elem_ptr, link_name), link_name) =                  \
        list_prev(elem_ptr, link_name);                                       \
    }                                                                         \
    else {                                                                    \
      list_tail(list_ptr) = list_prev(elem_ptr, link_name);                   \
      if (list_tail(list_ptr) != NULL)                                        \
        list_next(list_tail(list_ptr), link_name) = NULL;                     \
    }                                                                         \
    list_size(list_ptr)--;                                                    \
    list_next(elem_ptr, link_name) = NULL;                                    \
    list_prev(elem_ptr, link_name) = NULL;                                    \
  } while (0)


#define list_foreach(elem_ptr, list_ptr, link_name)                           \
  for (elem_ptr = list_head(list_ptr);                                        \
       elem_ptr != NULL;                                                      \
       elem_ptr = list_next(elem_ptr, link_name))

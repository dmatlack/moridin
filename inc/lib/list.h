/**
 * @file list.h
 *
 * @author David Matlack
 */
#include <variable_queue.h>

#define list_typedef(elem_type) \
  Q_NEW_HEAD(elem_type)

#define list_link(elem_type) \
  Q_NEW_LINK(elem_type)

#define list_init(list_ptr) \
  Q_INIT_HEAD(list_ptr)

#define list_swap_ptr(list_to_ptr, list_from_ptr) \
  Q_SWAP_HEAD(list_to_ptr, list_from_ptr)

#define list_head(list_ptr) \
  Q_GET_FRONT(list_ptr)

#define list_tail(list_ptr) \
  Q_GET_TAIL(list_ptr)

#define list_size(list_ptr) \
  Q_GET_SIZE(list_ptr)

#define list_next(elem_ptr, link_name) \
  Q_GET_NEXT(elem_ptr, link_name)

#define list_prev(elem_ptr, link_name) \
  Q_GET_PREV(elem_ptr, link_name)

#define list_elem_init(elem_ptr, link_name) \
  Q_INIT_ELEM(elem_ptr, link_name)

#define list_insert_head(list_ptr, elem_ptr, link_name) \
  Q_INSERT_FRONT(list_ptr, elem_ptr, link_name)

#define list_insert_tail(list_ptr, elem_ptr, link_name) \
  Q_INSERT_TAIL(list_ptr, elem_ptr, link_name)

#define list_insert_after(list_ptr, elem_inq, elem_new, link_name) \
  Q_INSERT_AFTER(list_ptr, elem_inq, elem_new, link_name)

#define list_insert_before(list_ptr, elem_inq, elem_new, link_name) \
  Q_INSERT_BEFORE(list_ptr, elem_inq, elem_new, link_name)

#define list_remove(list_ptr, elem_ptr, link_name) \
  Q_REMOVE(list_ptr, elem_ptr, link_name)

#define list_foreach(elem_ptr, list_ptr, link_name) \
  Q_FOREACH(elem_ptr, list_ptr, link_name)


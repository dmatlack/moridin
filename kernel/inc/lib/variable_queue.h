/** 
 * @file variable_queue.h
 *
 * @brief Generalized queue module for data collection
 *
 * @author Mark Wong Siang Kai
 */

/** @def Q_NEW_HEAD(Q_HEAD_TYPE, Q_ELEM_TYPE) 
 *
 *  @brief Generates a new structure of type Q_HEAD_TYPE representing the head 
 *  of a queue of elements of type Q_ELEM_TYPE. 
 *  
 *  Usage: Q_NEW_HEAD(Q_HEAD_TYPE, Q_ELEM_TYPE); //create the type <br>
 *         Q_HEAD_TYPE headName; //instantiate a head of the given type
 *
 *  @param Q_HEAD_TYPE the type you wish the newly-generated structure to have.
 *         
 *  @param Q_ELEM_TYPE the type of elements stored in the queue.
 *         Q_ELEM_TYPE must be a structure.
 */
#define Q_NEW_HEAD(Q_ELEM_TYPE) \
  typedef struct {                           \
    Q_ELEM_TYPE *head;                \
    Q_ELEM_TYPE *tail;                \
    int size;                                \
  }

/** @def Q_NEW_LINK(Q_ELEM_TYPE)
 *
 *  @brief Instantiates a link within a structure, allowing that structure to be
 *         collected into a queue created with Q_NEW_HEAD. 
 *
 *  Usage: <br>
 *  typedef struct Q_ELEM_TYPE {<br>
 *  Q_NEW_LINK(Q_ELEM_TYPE) LINK_NAME; //instantiate the link <br>
 *  } Q_ELEM_TYPE; <br>
 *
 *  A structure can have more than one link defined within it, as long as they
 *  have different names. This allows the structure to be placed in more than
 *  one queue simultanteously.
 *
 *  @param Q_ELEM_TYPE the type of the structure containing the link
 */
#define Q_NEW_LINK(Q_ELEM_TYPE) \
  struct {                      \
    Q_ELEM_TYPE *prev;   \
    Q_ELEM_TYPE *next;   \
  }
 
 
/** @def Q_INIT_HEAD(Q_HEAD)
 *
 *  @brief Initializes the head of a queue so that the queue head can be used
 *         properly.
 *  @param Q_HEAD Pointer to queue head to initialize
 **/
#define Q_INIT_HEAD(Q_HEAD) \
  do {                      \
    (Q_HEAD)->head = NULL;  \
    (Q_HEAD)->tail = NULL;  \
    (Q_HEAD)->size = 0;     \
  } while (0)

#define Q_SWAP_HEAD(Q_HEAD_TO, Q_HEAD_FM) \
  do {                                      \
    (Q_HEAD_TO)->head = (Q_HEAD_FM)->head;  \
    (Q_HEAD_FM)->head = NULL;               \
    (Q_HEAD_TO)->tail = (Q_HEAD_FM)->tail;  \
    (Q_HEAD_FM)->tail = NULL;               \
    (Q_HEAD_TO)->size = (Q_HEAD_FM)->size;  \
    (Q_HEAD_FM)->size = 0;                  \
  } while (0)

/** @def Q_GET_FRONT(Q_HEAD)
 *  
 *  @brief Returns a pointer to the first element in the queue, or NULL 
 *  (memory address 0) if the queue is empty.
 *
 *  @param Q_HEAD Pointer to the head of the queue
 *  @return Pointer to the first element in the queue, or NULL if the queue
 *          is empty
 **/
#define Q_GET_FRONT(Q_HEAD) \
  (Q_HEAD)->head
 
/** @def Q_GET_TAIL(Q_HEAD)
 *
 *  @brief Returns a pointer to the last element in the queue, or NULL 
 *  (memory address 0) if the queue is empty.
 *
 *  @param Q_HEAD Pointer to the head of the queue
 *  @return Pointer to the last element in the queue, or NULL if the queue
 *          is empty
 **/
#define Q_GET_TAIL(Q_HEAD)  \
  (Q_HEAD)->tail

#define Q_GET_SIZE(Q_HEAD)  \
  (Q_HEAD)->size

/** @def Q_GET_NEXT(Q_ELEM, LINK_NAME)
 * 
 *  @brief Returns a pointer to the next element in the queue, as linked to by 
 *         the link specified with LINK_NAME. 
 *
 *  If Q_ELEM is not in a queue or is the last element in the queue, 
 *  Q_GET_NEXT should return NULL.
 *
 *  @param Q_ELEM Pointer to the queue element before the desired element
 *  @param LINK_NAME Name of the link organizing the queue
 *
 *  @return The element after Q_ELEM, or NULL if there is no next element
 **/
#define Q_GET_NEXT(Q_ELEM, LINK_NAME) \
  (Q_ELEM)->LINK_NAME.next
 
/** @def Q_GET_PREV(Q_ELEM, LINK_NAME)
 * 
 *  @brief Returns a pointer to the previous element in the queue, as linked to 
 *         by the link specified with LINK_NAME. 
 *
 *  If Q_ELEM is not in a queue or is the first element in the queue, 
 *  Q_GET_NEXT should return NULL.
 *
 *  @param Q_ELEM Pointer to the queue element after the desired element
 *  @param LINK_NAME Name of the link organizing the queue
 *
 *  @return The element before Q_ELEM, or NULL if there is no next element
 **/
#define Q_GET_PREV(Q_ELEM, LINK_NAME) \
  (Q_ELEM)->LINK_NAME.prev

/** @def Q_INIT_ELEM(Q_ELEM, LINK_NAME)
 *
 *  @brief Initializes the link named LINK_NAME in an instance of the structure 
 *         Q_ELEM. 
 *  
 *  Once initialized, the link can be used to organized elements in a queue.
 *  
 *  @param Q_ELEM Pointer to the structure instance containing the link
 *  @param LINK_NAME The name of the link to initialize
 **/
#define Q_INIT_ELEM(Q_ELEM, LINK_NAME)    \
  do {                                    \
    Q_GET_NEXT(Q_ELEM, LINK_NAME) = NULL; \
    Q_GET_PREV(Q_ELEM, LINK_NAME) = NULL; \
  } while (0)
 
/** @def Q_INSERT_FRONT(Q_HEAD, Q_ELEM, LINK_NAME)
 *
 *  @brief Inserts the queue element pointed to by Q_ELEM at the front of the 
 *         queue headed by the structure Q_HEAD. 
 *  
 *  The link identified by LINK_NAME will be used to organize the element and
 *  record its location in the queue.
 *
 *  @param Q_HEAD Pointer to the head of the queue into which Q_ELEM will be 
 *         inserted
 *  @param Q_ELEM Pointer to the element to insert into the queue
 *  @param LINK_NAME Name of the link used to organize the queue
 *
 *  @return Void (you may change this if your implementation calls for a 
 *                return value)
 **/
#define Q_INSERT_FRONT(Q_HEAD, Q_ELEM, LINK_NAME)             \
  do {                                                        \
    Q_GET_NEXT(Q_ELEM, LINK_NAME) = Q_GET_FRONT(Q_HEAD);      \
    Q_GET_PREV(Q_ELEM, LINK_NAME) = NULL;                     \
    if (Q_GET_FRONT(Q_HEAD) == NULL)                          \
      Q_GET_TAIL(Q_HEAD) = (Q_ELEM);                          \
    else                                                      \
      Q_GET_PREV(Q_GET_FRONT(Q_HEAD), LINK_NAME) = (Q_ELEM);  \
    Q_GET_FRONT(Q_HEAD) = (Q_ELEM);                           \
    (Q_HEAD)->size++;                                         \
  } while (0)

/** @def Q_INSERT_TAIL(Q_HEAD, Q_ELEM, LINK_NAME) 
 *  @brief Inserts the queue element pointed to by Q_ELEM at the end of the 
 *         queue headed by the structure pointed to by Q_HEAD. 
 *  
 *  The link identified by LINK_NAME will be used to organize the element and
 *  record its location in the queue.
 *
 *  @param Q_HEAD Pointer to the head of the queue into which Q_ELEM will be 
 *         inserted
 *  @param Q_ELEM Pointer to the element to insert into the queue
 *  @param LINK_NAME Name of the link used to organize the queue
 *
 *  @return Void (you may change this if your implementation calls for a 
 *                return value)
 **/
#define Q_INSERT_TAIL(Q_HEAD, Q_ELEM, LINK_NAME)              \
  do {                                                        \
    Q_GET_PREV(Q_ELEM, LINK_NAME) = Q_GET_TAIL(Q_HEAD);       \
    Q_GET_NEXT(Q_ELEM, LINK_NAME) = NULL;                     \
    if (Q_GET_TAIL(Q_HEAD) == NULL) {                         \
      Q_GET_FRONT(Q_HEAD) = (Q_ELEM);                         \
    }                                                         \
    else                                                      \
      Q_GET_NEXT(Q_GET_TAIL(Q_HEAD), LINK_NAME) = (Q_ELEM);   \
    Q_GET_TAIL(Q_HEAD) = (Q_ELEM);                            \
    (Q_HEAD)->size++;                                         \
  } while (0)

/** @def Q_INSERT_AFTER(Q_HEAD, Q_INQ, Q_TOINSERT, LINK_NAME)
 *
 *  @brief Inserts the queue element Q_TOINSERT after the element Q_INQ
 *         in the queue.
 *
 *  Inserts an element into a queue after a given element. If the given
 *  element is the last element, Q_HEAD should be updated appropriately
 *  (so that Q_TOINSERT becomes the tail element)
 *
 *  Assumes Q_HEAD has at least one element (at least Q_INQ)
 *
 *  @param Q_HEAD head of the queue into which Q_TOINSERT will be inserted
 *  @param Q_INQ  Element already in the queue
 *  @param Q_TOINSERT Element to insert into queue
 *  @param LINK_NAME  Name of link field used to organize the queue
 **/

#define Q_INSERT_AFTER(Q_HEAD,Q_INQ,Q_TOINSERT,LINK_NAME) \
  do {  \
    if (Q_GET_NEXT(Q_INQ, LINK_NAME)!= NULL)                              \
      Q_GET_PREV(Q_GET_NEXT(Q_INQ, LINK_NAME), LINK_NAME) = (Q_TOINSERT); \
    Q_GET_NEXT(Q_TOINSERT, LINK_NAME) = Q_GET_NEXT(Q_INQ, LINK_NAME);     \
    Q_GET_NEXT(Q_INQ, LINK_NAME) = (Q_TOINSERT);                          \
    Q_GET_PREV(Q_TOINSERT, LINK_NAME) = (Q_INQ);                          \
    if ((Q_INQ) == Q_GET_TAIL(Q_HEAD))                                    \
      Q_GET_TAIL(Q_HEAD) = (Q_TOINSERT);                                  \
    (Q_HEAD)->size++;                                                     \
  } while (0)

/** @def Q_INSERT_BEFORE(Q_HEAD, Q_INQ, Q_TOINSERT, LINK_NAME)
 *
 *  @brief Inserts the queue element Q_TOINSERT before the element Q_INQ
 *         in the queue.
 *
 *  Inserts an element into a queue before a given element. If the given
 *  element is the first element, Q_HEAD should be updated appropriately
 *  (so that Q_TOINSERT becomes the front element)
 *
 *  @param Q_HEAD head of the queue into which Q_TOINSERT will be inserted
 *  @param Q_INQ  Element already in the queue
 *  @param Q_TOINSERT Element to insert into queue
 *  @param LINK_NAME  Name of link field used to organize the queue
 **/

#define Q_INSERT_BEFORE(Q_HEAD,Q_INQ,Q_TOINSERT,LINK_NAME)                \
  do {                                                                    \
    if (Q_GET_PREV(Q_INQ, LINK_NAME) != NULL)                             \
      Q_GET_NEXT(Q_GET_PREV(Q_INQ, LINK_NAME), LINK_NAME) = (Q_TOINSERT); \
    Q_GET_PREV(Q_TOINSERT, LINK_NAME) = Q_GET_PREV(Q_INQ, LINK_NAME);     \
    Q_GET_PREV(Q_INQ, LINK_NAME) = (Q_TOINSERT);                          \
    Q_GET_NEXT(Q_TOINSERT, LINK_NAME) = (Q_INQ);                          \
    if ((Q_INQ) == Q_GET_FRONT(Q_HEAD))                                   \
      Q_GET_FRONT(Q_HEAD) = (Q_TOINSERT);                                 \
    (Q_HEAD)->size++;                                                     \
  } while (0)

/** @def Q_REMOVE(Q_HEAD,Q_ELEM,LINK_NAME)
 * 
 *  @brief Detaches the element Q_ELEM from the queue organized by LINK_NAME
 *
 *  If Q_HEAD does not use the link named LINK_NAME to organize its elements or 
 *  if Q_ELEM is not a member of Q_HEAD's queue, the behavior of this macro
 *  is undefined.
 *
 *  @param Q_HEAD Pointer to the head of the queue containing Q_ELEM. If 
 *         Q_REMOVE removes the first, last, or only element in the queue, 
 *         Q_HEAD should be updated appropriately.
 *  @param Q_ELEM Pointer to the element to remove from the queue headed by 
 *         Q_HEAD.
 *  @param LINK_NAME The name of the link used to organize Q_HEAD's queue
 * 
 *  @return Void (if you would like to return a value, you may change this
 *                specification)
 **/
#define Q_REMOVE(Q_HEAD,Q_ELEM,LINK_NAME)                     \
  do {                                                        \
    if ((Q_GET_PREV(Q_ELEM, LINK_NAME)) != NULL) {            \
      Q_GET_NEXT(Q_GET_PREV(Q_ELEM, LINK_NAME), LINK_NAME) =  \
        Q_GET_NEXT(Q_ELEM, LINK_NAME);                        \
    }                                                         \
    else {                                                    \
      Q_GET_FRONT(Q_HEAD) = Q_GET_NEXT(Q_ELEM, LINK_NAME);    \
      if (Q_GET_FRONT(Q_HEAD) != NULL)                        \
        Q_GET_PREV(Q_GET_FRONT(Q_HEAD), LINK_NAME) = NULL;    \
    }                                                         \
    if ((Q_GET_NEXT(Q_ELEM, LINK_NAME)) != NULL) {            \
      Q_GET_PREV(Q_GET_NEXT(Q_ELEM, LINK_NAME), LINK_NAME) =  \
        Q_GET_PREV(Q_ELEM, LINK_NAME);                        \
    }                                                         \
    else {                                                    \
      Q_GET_TAIL(Q_HEAD) = Q_GET_PREV(Q_ELEM, LINK_NAME);     \
      if (Q_GET_TAIL(Q_HEAD) != NULL)                         \
        Q_GET_NEXT(Q_GET_TAIL(Q_HEAD), LINK_NAME) = NULL;     \
    }                                                         \
    (Q_HEAD)->size--;                                         \
    Q_GET_NEXT(Q_ELEM, LINK_NAME) = NULL;                     \
    Q_GET_PREV(Q_ELEM, LINK_NAME) = NULL;                     \
  } while (0)

/** @def Q_FOREACH(CURRENT_ELEM,Q_HEAD,LINK_NAME) 
 *
 *  @brief Constructs an iterator block (like a for block) that operates
 *         on each element in Q_HEAD, in order.
 *
 *  Q_FOREACH constructs the head of a block of code that will iterate through
 *  each element in the queue headed by Q_HEAD. Each time through the loop, 
 *  the variable named by CURRENT_ELEM will be set to point to a subsequent
 *  element in the queue.
 *
 *  Usage:<br>
 *  Q_FOREACH(CURRENT_ELEM,Q_HEAD,LINK_NAME)<br>
 *  {<br>
 *  ... operate on the variable CURRENT_ELEM ... <br>
 *  }
 *
 *  If LINK_NAME is not used to organize the queue headed by Q_HEAD, then
 *  the behavior of this macro is undefined.
 *
 *  @param CURRENT_ELEM name of the variable to use for iteration. On each
 *         loop through the Q_FOREACH block, CURRENT_ELEM will point to the
 *         current element in the queue. CURRENT_ELEM should be an already-
 *         defined variable name, and its type should be a pointer to 
 *         the type of data organized by Q_HEAD
 *  @param Q_HEAD Pointer to the head of the queue to iterate through
 *  @param LINK_NAME The name of the link used to organize the queue headed
 *         by Q_HEAD.
 **/

#define Q_FOREACH(CURRENT_ELEM,Q_HEAD,LINK_NAME)            \
  for (CURRENT_ELEM = Q_GET_FRONT(Q_HEAD);                  \
       CURRENT_ELEM != NULL;                                \
       CURRENT_ELEM = Q_GET_NEXT(CURRENT_ELEM, LINK_NAME))

/**
 * @brief create a static circular buffer queue of type ELEM_TYPE 
 * The queue will be of type QUEUE_TYPE
 * @param QUEUE_TYPE the type of this new queue.
 * @param ELEM_TYPE the type of elements in this buffer queue
 * @param MAX_SIZE the maximum size of the buffer
 */
#define Q_NEW_QUEUE(QUEUE_TYPE, ELEM_TYPE, MAX_SIZE)  \
  typedef struct {                                    \
    ELEM_TYPE buffer[MAX_SIZE];                       \
    int head_index;                                   \
    int tail_index;                                   \
    int num_elems;                                    \
    int capacity;                                     \
  } (QUEUE_TYPE);

/**
 * @brief the following are accessor macros for size, capacity, head_index,
 * tail_index and buffer respectively
 */
#define Q_SIZE(Q_QUEUE) ((Q_QUEUE)->num_elems)
#define Q_CAP(Q_QUEUE)  ((Q_QUEUE)->capacity)
#define Q_HEAD(Q_QUEUE) ((Q_QUEUE)->head_index)
#define Q_TAIL(Q_QUEUE) ((Q_QUEUE)->tail_index)
#define Q_BUF(Q_QUEUE)  ((Q_QUEUE)->buffer)

/**
 * @brief initializes the static queue
 * @param Q_QUEUE the static queue to initialize
 * @param MAX_SIZE the capacity of the queue
 */
#define Q_INIT_QUEUE(Q_QUEUE, MAX_SIZE) \
  do {                                  \
    Q_HEAD(Q_QUEUE) = 0;                \
    Q_TAIL(Q_QUEUE) = 0;                \
    Q_SIZE(Q_QUEUE) = 0;                \
    Q_CAP(Q_QUEUE) = MAX_SIZE;          \
  } while (0)

/**
 * @brief returns the element at index INDEX in the static queue Q_QUEUE
 */
#define Q_AT(Q_QUEUE, INDEX) (Q_BUF(Q_QUEUE)[INDEX])

/**
 * @brief Return the element at the tail of the queue 
 */
#define Q_PEEK_TAIL(Q_QUEUE) (Q_AT(Q_QUEUE, Q_TAIL(Q_QUEUE)))

/**
 * @brief returns the nth element of the static queue Q_QUEUE
 */
#define Q_NTH(Q_QUEUE, N) \
  (Q_AT(Q_QUEUE, (Q_HEAD(Q_QUEUE) + N) % Q_CAP(Q_QUEUE)))

/**
 * @brief adds the element to the tail of the static queue
 * @param Q_QUEUE a pointer to the static queue
 * @param Q_ELEM a pointer to the element to enqueue
 */
#define Q_ENQUEUE(Q_QUEUE, Q_ELEM) \
  do {                                                          \
    if (Q_SIZE(Q_QUEUE) != Q_CAP(Q_QUEUE)) {                    \
      Q_BUF(Q_QUEUE)[Q_TAIL(Q_QUEUE)] = Q_ELEM;                 \
      Q_TAIL(Q_QUEUE) = (Q_TAIL(Q_QUEUE) + 1) % Q_CAP(Q_QUEUE); \
      Q_SIZE(Q_QUEUE)++;                                        \
    }                                                           \
  } while (0)

/**
 * @brief checks if the static queue is empty
 * @param Q_QUEUE a pointer to the static queue
 * @return 0 if the queue is empty, non-zero otherwise
 */
#define Q_EMPTY(Q_QUEUE) (Q_SIZE(Q_QUEUE) == 0)

/**
 * @brief Return non-zero if the queue is full, zero otherwise
 */
#define Q_FULL(Q_QUEUE) (Q_SIZE(Q_QUEUE) == Q_CAP(Q_QUEUE))

/**
 * @brief removes the head of the static queue
 * @param Q_QUEUE a pointer to the static queue
 * @param TEMP_ELEM a pointer to store the head of the queue
 */
#define Q_DEQUEUE(Q_QUEUE, TEMP_ELEM)                           \
  do {                                                          \
    ASSERT(!Q_EMPTY(Q_QUEUE));                                  \
    *(TEMP_ELEM) = Q_BUF(Q_QUEUE)[Q_HEAD(Q_QUEUE)];             \
    Q_HEAD(Q_QUEUE) = (Q_HEAD(Q_QUEUE) + 1) % (Q_CAP(Q_QUEUE)); \
    Q_SIZE(Q_QUEUE)--;                                          \
  } while (0)

/**
 * @brief removes the tail of the static queue
 * @param Q_QUEUE a pointer to the static queue
 * @param TEMP_ELEM a pointer to store the head of the queue
 */
#define Q_POP(Q_QUEUE, TEMP_ELEM)                               \
  do {                                                          \
    ASSERT(!Q_EMPTY(Q_QUEUE));                                  \
    *(TEMP_ELEM) = Q_BUF(Q_QUEUE)[Q_TAIL(Q_QUEUE)];             \
    Q_TAIL(Q_QUEUE) = (Q_TAIL(Q_QUEUE) - 1) % (Q_CAP(Q_QUEUE)); \
    Q_SIZE(Q_QUEUE)--;                                          \
  } while (0)


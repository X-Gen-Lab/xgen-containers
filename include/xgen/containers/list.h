/**
 * \file            list.h
 * \brief           Intrusive non-owning doubly-linked list
 * \author          X-Gen Lab
 */

#ifndef XGCT_LIST_H
#define XGCT_LIST_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct xgct_list_node {
    struct xgct_list_node *next;
    struct xgct_list_node *prev;
} xgct_list_node_t;

typedef struct {
    xgct_list_node_t *head;
    xgct_list_node_t *tail;
    size_t count;
} xgct_list_t;

/* Intrusive, non-owning, externally synchronized. Before insertion a node
 * must be unlinked. Position/removal nodes must belong to this list.
 * Nodes and lists must not move or be copied while linked. */
/**
 * \brief           Initialize an empty list
 * \param[in,out]   list: Caller-owned list
 */
void xgct_list_init(xgct_list_t *list);
/**
 * \brief           Initialize an unlinked list node
 * \param[in,out]   node: Caller-owned intrusive node
 */
void xgct_list_node_init(xgct_list_node_t *node);
/**
 * \brief           Check whether the list contains no nodes
 * \param[in]       list: Caller-owned list
 * \return          true when the condition holds, otherwise false
 */
bool xgct_list_is_empty(const xgct_list_t *list);
/**
 * \brief           Query the number of linked nodes
 * \param[in]       list: Caller-owned list
 * \return          Calculated or queried value
 */
size_t xgct_list_count(const xgct_list_t *list);
/**
 * \brief           Insert an unlinked node at the head
 * \param[in,out]   list: Caller-owned list
 * \param[in,out]   node: Caller-owned intrusive node
 */
void xgct_list_insert_head(xgct_list_t *list, xgct_list_node_t *node);
/**
 * \brief           Insert an unlinked node at the tail
 * \param[in,out]   list: Caller-owned list
 * \param[in,out]   node: Caller-owned intrusive node
 */
void xgct_list_insert_tail(xgct_list_t *list, xgct_list_node_t *node);
/**
 * \brief           Insert an unlinked node after a member of this list
 * \param[in,out]   list: Caller-owned list
 * \param[in,out]   pos: Position node that belongs to this list
 * \param[in,out]   node: Caller-owned intrusive node
 */
void xgct_list_insert_after(xgct_list_t *list, xgct_list_node_t *pos,
                           xgct_list_node_t *node);
/**
 * \brief           Insert an unlinked node before a member of this list
 * \param[in,out]   list: Caller-owned list
 * \param[in,out]   pos: Position node that belongs to this list
 * \param[in,out]   node: Caller-owned intrusive node
 */
void xgct_list_insert_before(xgct_list_t *list, xgct_list_node_t *pos,
                            xgct_list_node_t *node);
/**
 * \brief           Detach a node that belongs to this list
 * \param[in,out]   list: Caller-owned list
 * \param[in,out]   node: Caller-owned intrusive node
 */
void xgct_list_remove(xgct_list_t *list, xgct_list_node_t *node);
/**
 * \brief           Detach the head node
 * \param[in,out]   list: Caller-owned list
 * \return          Matching object, or NULL when absent or unavailable
 */
xgct_list_node_t *xgct_list_remove_head(xgct_list_t *list);
/**
 * \brief           Detach the tail node
 * \param[in,out]   list: Caller-owned list
 * \return          Matching object, or NULL when absent or unavailable
 */
xgct_list_node_t *xgct_list_remove_tail(xgct_list_t *list);
/**
 * \brief           Observe the head node without removing it
 * \param[in]       list: Caller-owned list
 * \return          Matching object, or NULL when absent or unavailable
 */
xgct_list_node_t *xgct_list_peek_head(const xgct_list_t *list);
/**
 * \brief           Observe the tail node without removing it
 * \param[in]       list: Caller-owned list
 * \return          Matching object, or NULL when absent or unavailable
 */
xgct_list_node_t *xgct_list_peek_tail(const xgct_list_t *list);
/**
 * \brief           Observe the next linked node
 * \param[in]       node: Caller-owned intrusive node
 * \return          Matching object, or NULL when absent or unavailable
 */
xgct_list_node_t *xgct_list_next(const xgct_list_node_t *node);
/**
 * \brief           Observe the previous linked node
 * \param[in]       node: Caller-owned intrusive node
 * \return          Matching object, or NULL when absent or unavailable
 */
xgct_list_node_t *xgct_list_prev(const xgct_list_node_t *node);

/**
 * \brief           Detach all members without destroying caller objects.
 * \param[in,out]   list: Valid initialized list; NULL is ignored.
 * \note            Work is O(count); every detached node can be reinserted.
 */
void xgct_list_clear(xgct_list_t* list);
/**
 * \brief           Diagnose links and count with bounded traversal.
 * \param[in]       list: List whose pointer fields refer to live nodes, or NULL.
 * \return          true for consistent structure, false otherwise.
 * \note            This does not validate arbitrary addresses or detect shared
 *                  ownership of singleton nodes. Mutations retain their stated
 *                  membership preconditions; no owner pointer is added.
 */
bool xgct_list_validate(const xgct_list_t* list);

#define XGCT_LIST_ENTRY(ptr, type, member) \
    ((type *) (void *) ((char *) (ptr) - offsetof(type, member)))
#define XGCT_LIST_FOR_EACH(list, node) \
    for ((node) = (list)->head; (node) != NULL; (node) = (node)->next)
#define XGCT_LIST_FOR_EACH_SAFE(list, node, tmp)                         \
    for ((node) = (list)->head, (tmp) = ((node) ? (node)->next : NULL); \
         (node) != NULL;                                                \
         (node) = (tmp), (tmp) = ((node) ? (node)->next : NULL))

#ifdef __cplusplus
}
#endif
#endif

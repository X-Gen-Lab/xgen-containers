/**
 * \file            xgct_list.c
 * \brief           Intrusive doubly-linked list implementation
 * \author          X-Gen Lab
 */

#include <xgen/containers/list.h>

#include <stddef.h>

/*---------------------------------------------------------------------------*/
/* List Initialization                                                       */
/*---------------------------------------------------------------------------*/

/**
 * \brief           Initialize a list
 */
void xgct_list_init(xgct_list_t *list)
{
    if (list == NULL) {
        return;
    }

    list->head = NULL;
    list->tail = NULL;
    list->count = 0;
}

/**
 * \brief           Initialize a list node
 */
void xgct_list_node_init(xgct_list_node_t *node)
{
    if (node == NULL) {
        return;
    }

    node->next = NULL;
    node->prev = NULL;
}

/*---------------------------------------------------------------------------*/
/* List Query Operations                                                     */
/*---------------------------------------------------------------------------*/

/**
 * \brief           Check if list is empty
 */
bool xgct_list_is_empty(const xgct_list_t *list)
{
    if (list == NULL) {
        return true;
    }

    return list->head == NULL;
}

/**
 * \brief           Get number of nodes in list
 */
size_t xgct_list_count(const xgct_list_t *list)
{
    if (list == NULL) {
        return 0;
    }

    return list->count;
}

/*---------------------------------------------------------------------------*/
/* List Insertion Operations                                                 */
/*---------------------------------------------------------------------------*/

/**
 * \brief           Insert node at head of list
 */
void xgct_list_insert_head(xgct_list_t *list, xgct_list_node_t *node)
{
    if (list == NULL || node == NULL) {
        return;
    }

    node->prev = NULL;
    node->next = list->head;

    if (list->head != NULL) {
        list->head->prev = node;
    } else {
        /* List was empty, node is also tail */
        list->tail = node;
    }

    list->head = node;
    list->count++;
}

/**
 * \brief           Insert node at tail of list
 */
void xgct_list_insert_tail(xgct_list_t *list, xgct_list_node_t *node)
{
    if (list == NULL || node == NULL) {
        return;
    }

    node->next = NULL;
    node->prev = list->tail;

    if (list->tail != NULL) {
        list->tail->next = node;
    } else {
        /* List was empty, node is also head */
        list->head = node;
    }

    list->tail = node;
    list->count++;
}

/**
 * \brief           Insert node after specified node
 */
void xgct_list_insert_after(xgct_list_t *list, xgct_list_node_t *pos,
                           xgct_list_node_t *node)
{
    if (list == NULL || pos == NULL || node == NULL) {
        return;
    }

    node->prev = pos;
    node->next = pos->next;

    if (pos->next != NULL) {
        pos->next->prev = node;
    } else {
        /* pos was tail, node is new tail */
        list->tail = node;
    }

    pos->next = node;
    list->count++;
}

/**
 * \brief           Insert node before specified node
 */
void xgct_list_insert_before(xgct_list_t *list, xgct_list_node_t *pos,
                            xgct_list_node_t *node)
{
    if (list == NULL || pos == NULL || node == NULL) {
        return;
    }

    node->next = pos;
    node->prev = pos->prev;

    if (pos->prev != NULL) {
        pos->prev->next = node;
    } else {
        /* pos was head, node is new head */
        list->head = node;
    }

    pos->prev = node;
    list->count++;
}

/*---------------------------------------------------------------------------*/
/* List Removal Operations                                                   */
/*---------------------------------------------------------------------------*/

/**
 * \brief           Remove node from list
 */
void xgct_list_remove(xgct_list_t *list, xgct_list_node_t *node)
{
    if (list == NULL || node == NULL) {
        return;
    }

    /* Update previous node's next pointer */
    if (node->prev != NULL) {
        node->prev->next = node->next;
    } else {
        /* Node was head */
        list->head = node->next;
    }

    /* Update next node's prev pointer */
    if (node->next != NULL) {
        node->next->prev = node->prev;
    } else {
        /* Node was tail */
        list->tail = node->prev;
    }

    /* Clear node pointers */
    node->next = NULL;
    node->prev = NULL;

    list->count--;
}

/**
 * \brief           Remove and return head node
 */
xgct_list_node_t *xgct_list_remove_head(xgct_list_t *list)
{
    if (list == NULL || list->head == NULL) {
        return NULL;
    }

    xgct_list_node_t *node = list->head;
    xgct_list_remove(list, node);
    return node;
}

/**
 * \brief           Remove and return tail node
 */
xgct_list_node_t *xgct_list_remove_tail(xgct_list_t *list)
{
    if (list == NULL || list->tail == NULL) {
        return NULL;
    }

    xgct_list_node_t *node = list->tail;
    xgct_list_remove(list, node);
    return node;
}

/*---------------------------------------------------------------------------*/
/* List Access Operations                                                    */
/*---------------------------------------------------------------------------*/

/**
 * \brief           Get head node without removing
 */
xgct_list_node_t *xgct_list_peek_head(const xgct_list_t *list)
{
    if (list == NULL) {
        return NULL;
    }

    return list->head;
}

/**
 * \brief           Get tail node without removing
 */
xgct_list_node_t *xgct_list_peek_tail(const xgct_list_t *list)
{
    if (list == NULL) {
        return NULL;
    }

    return list->tail;
}

/**
 * \brief           Get next node in list
 */
xgct_list_node_t *xgct_list_next(const xgct_list_node_t *node)
{
    if (node == NULL) {
        return NULL;
    }

    return node->next;
}

/**
 * \brief           Get previous node in list
 */
xgct_list_node_t *xgct_list_prev(const xgct_list_node_t *node)
{
    if (node == NULL) {
        return NULL;
    }

    return node->prev;
}

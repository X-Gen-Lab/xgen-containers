/**
 * \file            hash.h
 * \brief           Intrusive fixed-capacity hash index with caller-owned
 *                  buckets
 * \author          X-Gen Lab
 */

#ifndef XGCT_HASH_H
#define XGCT_HASH_H

#include <xgen/status/status.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \brief           Forward declaration for node ownership. */
typedef struct xgct_hash xgct_hash_t;

/** \brief           Caller-owned index node and borrowed value. */
typedef struct xgct_hash_node {
    uint32_t key; /**< Unsigned integer lookup key. */
    void* value;  /**< Borrowed application value, possibly NULL. */
    struct xgct_hash_node* next; /**< Next collision-chain member. */
    xgct_hash_t* owner; /**< Owning index while linked, otherwise NULL. */
} xgct_hash_node_t;

/** \brief           Fixed buckets and entry limit; no allocation or rehash. */
struct xgct_hash {
    xgct_hash_node_t** buckets; /**< Borrowed bucket array. */
    size_t bucket_count;        /**< Nonzero power of two. */
    size_t capacity;            /**< Maximum entries. */
    size_t count;               /**< Current entries. */
};

/* Caller owns table, power-of-two buckets, initialized nodes, and values.
 * No allocation, resize, or value destruction. Capacity is an entry limit,
 * independent of bucket_count. Nodes/tables must not move while linked.
 * NULL values are permitted: use find()'s node result to distinguish absence.
 * Calls require external serialization. */
/**
 * \brief           Initialize caller buckets and the fixed entry capacity
 * \param[in,out]   table: Caller-owned hash index
 * \param[in,out]   buckets: Caller-owned bucket array
 * \param[in]       bucket_count: Nonzero power-of-two bucket count
 * \param[in]       capacity: Maximum number of indexed entries
 * \return          XGS_OK on success; a status code on validation or capacity
 *                  failure
 */
xgs_status_t xgct_hash_init(xgct_hash_t* table, xgct_hash_node_t** buckets,
                            size_t bucket_count, size_t capacity);

/**
 * \brief           Initialize an unlinked caller-owned node
 * \param[in,out]   node: Caller-owned intrusive node
 */
void xgct_hash_node_init(xgct_hash_node_t* node);

/**
 * \brief           Index an unlinked node without allocating memory
 * \param[in,out]   table: Caller-owned hash index
 * \param[in,out]   node: Caller-owned intrusive node
 * \param[in]       key: Integer lookup key
 * \param[in,out]   value: Non-owned value pointer; NULL is supported
 * \return          XGS_OK on success; a status code on validation or capacity
 *                  failure
 */
xgs_status_t xgct_hash_insert(xgct_hash_t* table, xgct_hash_node_t* node,
                              uint32_t key, void* value);

/**
 * \brief           Find a node by key, including nodes with NULL values
 * \param[in]       table: Caller-owned hash index
 * \param[in]       key: Integer lookup key
 * \return          Matching object, or NULL when absent or unavailable
 */
xgct_hash_node_t* xgct_hash_find(const xgct_hash_t* table, uint32_t key);

/**
 * \brief           Detach the node matching a key without freeing its value
 * \param[in,out]   table: Caller-owned hash index
 * \param[in]       key: Integer lookup key
 * \return          Matching object, or NULL when absent or unavailable
 */
xgct_hash_node_t* xgct_hash_remove(xgct_hash_t* table, uint32_t key);

/* Detaches nodes without freeing nodes or values. Retains initialized buckets.
 */
/**
 * \brief           Detach all nodes while retaining initialized buckets
 * \param[in,out]   table: Caller-owned hash index
 */
void xgct_hash_clear(xgct_hash_t* table);

/**
 * \brief           Query the current number of indexed nodes
 * \param[in]       table: Caller-owned hash index
 * \return          Calculated or queried value
 */
size_t xgct_hash_count(const xgct_hash_t* table);

/**
 * \brief           Diagnose count, membership and bucket placement.
 * \param[in]       table: Index containing live pointer targets, or NULL.
 * \return          true for consistent structure; false for invalid or cyclic
 *                  structure. Does not validate arbitrary memory addresses.
 */
bool xgct_hash_validate(const xgct_hash_t* table);

#ifdef __cplusplus
}
#endif

#endif

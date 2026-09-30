/**
 * \file            hash.c
 * \brief           Intrusive fixed-capacity hash index with caller-owned
 *                  buckets
 * \author          X-Gen Lab
 */

#include <xgen/containers/hash.h>

static size_t bucket_index(const xgct_hash_t *table, uint32_t key)
{
    /* Multiplicative mixing, with high bits folded into the bucket index. */
    key ^= key >> 16U;
    key *= UINT32_C(0x7feb352d);
    key ^= key >> 15U;
    return (size_t) key & (table->bucket_count - 1U);
}

bool xgct_hash_validate(const xgct_hash_t* table) {
    if (table == NULL || table->buckets == NULL || table->bucket_count == 0U ||
        (table->bucket_count & (table->bucket_count - 1U)) != 0U ||
        table->capacity == 0U || table->count > table->capacity) {
        return false;
    }
    size_t count = 0U;
    for (size_t i = 0U; i < table->bucket_count; ++i) {
        const xgct_hash_node_t* node = table->buckets[i];
        while (node != NULL) {
            if (count >= table->count || node->owner != table ||
                bucket_index(table, node->key) != i) {
                return false;
            }
            ++count;
            node = node->next;
        }
    }
    return count == table->count;
}

xgs_status_t xgct_hash_init(xgct_hash_t *table, xgct_hash_node_t **buckets,
                           size_t bucket_count, size_t capacity)
{
    if (table == NULL || buckets == NULL || bucket_count == 0U ||
        (bucket_count & (bucket_count - 1U)) != 0U || capacity == 0U ||
        bucket_count > SIZE_MAX / sizeof(*buckets)) {
        return XGS_INVALID_ARGUMENT;
    }
    for (size_t i = 0U; i < bucket_count; ++i) {
        buckets[i] = NULL;
    }
    *table = (xgct_hash_t) {buckets, bucket_count, capacity, 0U};
    return XGS_OK;
}

void xgct_hash_node_init(xgct_hash_node_t *node)
{
    if (node != NULL) {
        *node = (xgct_hash_node_t) {0};
    }
}

xgct_hash_node_t *xgct_hash_find(const xgct_hash_t *table, uint32_t key)
{
    if (table == NULL || table->buckets == NULL || table->bucket_count == 0U) {
        return NULL;
    }
    xgct_hash_node_t *node = table->buckets[bucket_index(table, key)];
    while (node != NULL) {
        if (node->key == key) {
            return node;
        }
        node = node->next;
    }
    return NULL;
}

xgs_status_t xgct_hash_insert(xgct_hash_t *table, xgct_hash_node_t *node,
                             uint32_t key, void *value)
{
    if (table == NULL || table->buckets == NULL || table->bucket_count == 0U ||
        node == NULL) {
        return XGS_INVALID_ARGUMENT;
    }
    if (node->owner != NULL) {
        return XGS_BUSY;
    }
    if (xgct_hash_find(table, key) != NULL) {
        return XGS_ALREADY_EXISTS;
    }
    if (table->count >= table->capacity) {
        return XGS_CAPACITY;
    }

    size_t index = bucket_index(table, key);
    node->key = key;
    node->value = value;
    node->owner = table;
    node->next = table->buckets[index];
    table->buckets[index] = node;
    ++table->count;
    return XGS_OK;
}

xgct_hash_node_t *xgct_hash_remove(xgct_hash_t *table, uint32_t key)
{
    if (table == NULL || table->buckets == NULL || table->bucket_count == 0U) {
        return NULL;
    }
    xgct_hash_node_t **link = &table->buckets[bucket_index(table, key)];
    while (*link != NULL) {
        xgct_hash_node_t *node = *link;
        if (node->key == key) {
            *link = node->next;
            node->next = NULL;
            node->owner = NULL;
            --table->count;
            return node;
        }
        link = &node->next;
    }
    return NULL;
}

void xgct_hash_clear(xgct_hash_t *table)
{
    if (table == NULL || table->buckets == NULL) {
        return;
    }
    for (size_t i = 0U; i < table->bucket_count; ++i) {
        xgct_hash_node_t *node = table->buckets[i];
        while (node != NULL) {
            xgct_hash_node_t *next = node->next;
            node->next = NULL;
            node->owner = NULL;
            node = next;
        }
        table->buckets[i] = NULL;
    }
    table->count = 0U;
}

size_t xgct_hash_count(const xgct_hash_t *table)
{
    return table == NULL ? 0U : table->count;
}

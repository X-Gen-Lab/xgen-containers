/**
 * \file            bitset.h
 * \brief           Fixed-capacity caller-owned bit storage
 */
#ifndef XGCT_BITSET_H
#define XGCT_BITSET_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <xgen/status/status.h>
#ifdef __cplusplus
extern "C" {
#endif
/**
 * \brief           Borrowed bit storage; least significant bit comes first.
 * \details         The caller serializes all operations, supplies live storage
 *                  of at least storage_size(bit_count), and does not overlap
 *                  state and storage. The descriptor may be a temporary view.
 *                  No memory, locks, threads, or protocol state are owned.
 */
typedef struct {
    uint8_t* storage; /**< Caller-owned bytes. */
    size_t bit_count; /**< Valid bit positions. */
} xgct_bitset_t;
/**
 * \brief           Calculate storage bytes without overflowing.
 * \param[in]       bit_count: Number of bits; zero is accepted.
 * \return          Ceil(bit_count / 8), or zero for zero bits.
 */
size_t xgct_bitset_storage_size(size_t bit_count);
/**
 * \brief           Initialize and clear the required bytes only.
 * \param[out]      set: Separate caller descriptor.
 * \param[in,out]   storage: Borrowed writable bytes.
 * \param[in]       storage_size: Available bytes.
 * \param[in]       bit_count: Nonzero bit capacity.
 * \return          OK, INVALID_ARGUMENT, or CAPACITY. Errors change nothing.
 */
xgs_status_t xgct_bitset_init(xgct_bitset_t* set, void* storage,
                              size_t storage_size, size_t bit_count);
/**
 * \brief           Set a bit without modifying unused tail bits.
 * \param[in,out]   set: Initialized descriptor.
 * \param[in]       index: Zero-based position.
 * \return          OK, or INVALID_ARGUMENT with no modification.
 */
xgs_status_t xgct_bitset_set(xgct_bitset_t* set, size_t index);
/**
 * \brief           Clear one bit.
 * \param[in,out]   set: Initialized descriptor.
 * \param[in]       index: Zero-based position.
 * \return          OK, or INVALID_ARGUMENT with no modification.
 */
xgs_status_t xgct_bitset_clear(xgct_bitset_t* set, size_t index);
/**
 * \brief           Read a bit.
 * \param[in]       set: Initialized descriptor, or NULL.
 * \param[in]       index: Zero-based position.
 * \return          The bit, or false for an invalid descriptor or index.
 */
bool xgct_bitset_test(const xgct_bitset_t* set, size_t index);
/**
 * \brief           Clear all storage bits including unused tail bits.
 * \param[in,out]   set: Initialized descriptor; NULL is ignored.
 */
void xgct_bitset_clear_all(xgct_bitset_t* set);
#ifdef __cplusplus
}
#endif
#endif

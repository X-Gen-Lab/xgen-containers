/**
 * \file            bitset.c
 * \brief           Bounded bit operations without allocation
 */
#include <string.h>
#include <xgen/containers/bitset.h>

size_t xgct_bitset_storage_size(size_t bit_count) {
    return bit_count / 8U + (bit_count % 8U != 0U ? 1U : 0U);
}

/* Checked capacity/unit contract; see docs/standards.md. */
/* NOLINTBEGIN(bugprone-easily-swappable-parameters,clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling)
 */
xgs_status_t xgct_bitset_init(xgct_bitset_t* set, void* storage,
                              size_t storage_size, size_t bit_count) {
    if (set == NULL || storage == NULL || bit_count == 0U) {
        return XGS_INVALID_ARGUMENT;
    }
    size_t required = xgct_bitset_storage_size(bit_count);
    if (storage_size < required) {
        return XGS_CAPACITY;
    }
    memset(storage, 0, required);
    *set = (xgct_bitset_t){storage, bit_count};
    return XGS_OK;
}

/* NOLINTEND(bugprone-easily-swappable-parameters,clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling)
 */

xgs_status_t xgct_bitset_set(xgct_bitset_t* set, size_t index) {
    if (set == NULL || set->storage == NULL || index >= set->bit_count) {
        return XGS_INVALID_ARGUMENT;
    }
    set->storage[index / 8U] |= (uint8_t)(1U << (index % 8U));
    return XGS_OK;
}

xgs_status_t xgct_bitset_clear(xgct_bitset_t* set, size_t index) {
    if (set == NULL || set->storage == NULL || index >= set->bit_count) {
        return XGS_INVALID_ARGUMENT;
    }
    set->storage[index / 8U] &= (uint8_t)~(1U << (index % 8U));
    return XGS_OK;
}

bool xgct_bitset_test(const xgct_bitset_t* set, size_t index) {
    return set != NULL && set->storage != NULL && index < set->bit_count &&
           (set->storage[index / 8U] & (uint8_t)(1U << (index % 8U))) != 0U;
}

/* Checked capacity/unit contract; see docs/standards.md. */
/* NOLINTBEGIN(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling)
 */
void xgct_bitset_clear_all(xgct_bitset_t* set) {
    if (set != NULL && set->storage != NULL) {
        memset(set->storage, 0, xgct_bitset_storage_size(set->bit_count));
    }
}

/* NOLINTEND(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling)
 */

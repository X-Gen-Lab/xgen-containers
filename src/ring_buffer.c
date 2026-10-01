/**
 * \file            ring_buffer.c
 * \brief           Byte ring with bounded non-overlapping DMA reservations
 */
#include <stdbool.h>
#include <string.h>
#include <xgen/containers/ring_buffer.h>

static bool valid(const xgct_ring_buffer_t* ring) {
    return ring != NULL && ring->storage != NULL && ring->capacity != 0U;
}

static size_t minimum(size_t first, size_t second) {
    return first < second ? first : second;
}

/* Avoid adding two capacity-sized values, which could overflow size_t. */
/* Checked capacity/unit contract; see docs/standards.md. */
/* NOLINTBEGIN(bugprone-easily-swappable-parameters) */
static size_t advance(size_t position, size_t amount, size_t capacity) {
    size_t tail = capacity - position;
    return amount >= tail ? amount - tail : position + amount;
}

/* NOLINTEND(bugprone-easily-swappable-parameters) */

xgs_status_t xgct_ring_init(xgct_ring_buffer_t* ring, void* storage,
                            size_t capacity) {
    if (ring == NULL || storage == NULL || capacity == 0U) {
        return XGS_INVALID_ARGUMENT;
    }
    *ring = (xgct_ring_buffer_t){0};
    ring->storage = storage;
    ring->capacity = capacity;
    return XGS_OK;
}

xgs_status_t xgct_ring_reset(xgct_ring_buffer_t* ring) {
    if (!valid(ring)) {
        return XGS_INVALID_ARGUMENT;
    }
    if (ring->read_token != 0U || ring->write_token != 0U) {
        return XGS_BUSY;
    }
    ring->read_pos = 0U;
    ring->write_pos = 0U;
    ring->count = 0U;
    return XGS_OK;
}

xgs_status_t xgct_ring_deinit(xgct_ring_buffer_t* ring) {
    xgs_status_t status = xgct_ring_reset(ring);
    if (status == XGS_OK) {
        *ring = (xgct_ring_buffer_t){0};
    }
    return status;
}

size_t xgct_ring_readable(const xgct_ring_buffer_t* ring) {
    return valid(ring) ? ring->count : 0U;
}

size_t xgct_ring_writable(const xgct_ring_buffer_t* ring) {
    return valid(ring) ? ring->capacity - ring->count - ring->write_reserved
                       : 0U;
}

xgs_status_t xgct_ring_write_claim(xgct_ring_buffer_t* ring, size_t requested,
                                   xgct_ring_write_span_t* span) {
    if (!valid(ring) || requested == 0U || span == NULL) {
        return XGS_INVALID_ARGUMENT;
    }
    if (ring->write_token != 0U) {
        return XGS_BUSY;
    }
    size_t size = minimum(requested, ring->capacity - ring->count);
    size = minimum(size, ring->capacity - ring->write_pos);
    if (size == 0U || ring->sequence == UINT64_MAX) {
        return XGS_CAPACITY;
    }
    ring->write_reserved = size;
    ring->write_token = ++ring->sequence;
    *span = (xgct_ring_write_span_t){ring->storage + ring->write_pos, size,
                                     ring->write_token};
    return XGS_OK;
}

xgs_status_t xgct_ring_write_finish(xgct_ring_buffer_t* ring, uint64_t token,
                                    size_t completed) {
    if (!valid(ring) || token == 0U || token != ring->write_token ||
        completed > ring->write_reserved) {
        return XGS_INVALID_ARGUMENT;
    }
    ring->write_pos = advance(ring->write_pos, completed, ring->capacity);
    ring->count += completed;
    ring->write_token = 0U;
    ring->write_reserved = 0U;
    return XGS_OK;
}

xgs_status_t xgct_ring_write_cancel(xgct_ring_buffer_t* ring, uint64_t token) {
    return xgct_ring_write_finish(ring, token, 0U);
}

xgs_status_t xgct_ring_read_claim(xgct_ring_buffer_t* ring, size_t requested,
                                  xgct_ring_read_span_t* span) {
    if (!valid(ring) || requested == 0U || span == NULL) {
        return XGS_INVALID_ARGUMENT;
    }
    if (ring->read_token != 0U) {
        return XGS_BUSY;
    }
    size_t size = minimum(requested, ring->count);
    size = minimum(size, ring->capacity - ring->read_pos);
    if (size == 0U || ring->sequence == UINT64_MAX) {
        return XGS_CAPACITY;
    }
    ring->read_reserved = size;
    ring->read_token = ++ring->sequence;
    *span = (xgct_ring_read_span_t){ring->storage + ring->read_pos, size,
                                    ring->read_token};
    return XGS_OK;
}

xgs_status_t xgct_ring_read_finish(xgct_ring_buffer_t* ring, uint64_t token,
                                   size_t completed) {
    if (!valid(ring) || token == 0U || token != ring->read_token ||
        completed > ring->read_reserved) {
        return XGS_INVALID_ARGUMENT;
    }
    ring->read_pos = advance(ring->read_pos, completed, ring->capacity);
    ring->count -= completed;
    ring->read_token = 0U;
    ring->read_reserved = 0U;
    return XGS_OK;
}

xgs_status_t xgct_ring_read_cancel(xgct_ring_buffer_t* ring, uint64_t token) {
    return xgct_ring_read_finish(ring, token, 0U);
}

/* Checked capacity/unit contract; see docs/standards.md. */
/* NOLINTBEGIN(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling)
 */
size_t xgct_ring_write(xgct_ring_buffer_t* ring, const void* source,
                       size_t size) {
    if (!valid(ring) || source == NULL || ring->write_token != 0U) {
        return 0U;
    }
    size_t amount = minimum(size, ring->capacity - ring->count);
    if (amount == 0U) {
        return 0U;
    }
    size_t first = minimum(amount, ring->capacity - ring->write_pos);
    memcpy(ring->storage + ring->write_pos, source, first);
    if (first < amount) {
        memcpy(ring->storage, (const uint8_t*)source + first, amount - first);
    }
    ring->write_pos = advance(ring->write_pos, amount, ring->capacity);
    ring->count += amount;
    return amount;
}

/* NOLINTEND(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling)
 */

/* Checked capacity/unit contract; see docs/standards.md. */
/* NOLINTBEGIN(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling)
 */
size_t xgct_ring_read(xgct_ring_buffer_t* ring, void* destination,
                      size_t size) {
    if (!valid(ring) || destination == NULL || ring->read_token != 0U) {
        return 0U;
    }
    size_t amount = minimum(size, ring->count);
    if (amount == 0U) {
        return 0U;
    }
    size_t first = minimum(amount, ring->capacity - ring->read_pos);
    memcpy(destination, ring->storage + ring->read_pos, first);
    if (first < amount) {
        memcpy((uint8_t*)destination + first, ring->storage, amount - first);
    }
    ring->read_pos = advance(ring->read_pos, amount, ring->capacity);
    ring->count -= amount;
    return amount;
}

/* NOLINTEND(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling)
 */

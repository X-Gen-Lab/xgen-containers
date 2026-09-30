/**
 * \file            ring_buffer.h
 * \brief           Bounded byte ring with exclusive zero-copy claims
 */
#ifndef XGCT_RING_BUFFER_H
#define XGCT_RING_BUFFER_H
#include <xgen/status/status.h>
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
/**
 * \brief           Non-owning ring state; fields are private to this API.
 * \details         All calls require external serialization, including ISR
 *                  calls. This is not an atomic or lock-free SPSC queue.
 *                  One reader and one writer claim may coexist on disjoint
 *                  spans; each side permits only one outstanding claim.
 *                  DMA addresses, cache maintenance, barriers and stopping
 *                  hardware belong to the platform. Stop DMA and synchronize
 *                  callbacks BEFORE finish/cancel; never reuse a claimed span.
 *                  State and buffer remain fixed and live until deinit.
 *                  Tokens are unique within one initialized lifetime (including
 *                  reset); reinit requires all old callbacks to be quiescent.
 */
typedef struct {
    uint8_t* storage; /**< Borrowed bytes. */
    size_t capacity; /**< Buffer byte capacity. */
    size_t read_pos; /**< Next committed readable byte. */
    size_t write_pos; /**< Next writable byte. */
    size_t count; /**< Committed readable bytes. */
    size_t read_reserved; /**< Reader's claimed bytes. */
    size_t write_reserved; /**< Writer's claimed bytes. */
    uint64_t sequence; /**< Last claim ID; never wraps. */
    uint64_t read_token; /**< Active reader ID, or zero. */
    uint64_t write_token; /**< Active writer ID, or zero. */
} xgct_ring_buffer_t;
/** \brief           Writable contiguous reservation, invalid after completion. */
typedef struct {
    uint8_t* data; /**< Writable span. */
    size_t size; /**< Reserved byte count. */
    uint64_t token; /**< Identity required by finish/cancel. */
} xgct_ring_write_span_t;
/** \brief           Readable contiguous reservation, invalid after completion. */
typedef struct {
    const uint8_t* data; /**< Read-only span, including during TX DMA. */
    size_t size; /**< Reserved byte count. */
    uint64_t token; /**< Identity required by finish/cancel. */
} xgct_ring_read_span_t;
/**
 * \brief           Initialize an empty ring without clearing storage.
 * \param[out]      ring: Uninitialized state, disjoint from storage.
 * \param[in,out]   storage: Borrowed buffer; no alignment promise beyond bytes.
 * \param[in]       capacity: Nonzero capacity; any size is supported.
 * \return          OK or INVALID_ARGUMENT; failure leaves objects unchanged.
 */
xgs_status_t xgct_ring_init(xgct_ring_buffer_t* ring, void* storage, size_t capacity);
/**
 * \brief           Discard queued bytes, preserving claim identity history.
 * \param[in,out]   ring: Initialized state.
 * \return          OK, INVALID_ARGUMENT, or BUSY if either claim is active.
 */
xgs_status_t xgct_ring_reset(xgct_ring_buffer_t* ring);
/**
 * \brief           Invalidate state without releasing borrowed storage.
 * \param[in,out]   ring: Initialized state.
 * \return          OK, INVALID_ARGUMENT, or BUSY with state unchanged.
 */
xgs_status_t xgct_ring_deinit(xgct_ring_buffer_t* ring);
/**
 * \brief           Query committed bytes, including an active read claim.
 * \param[in]       ring: Initialized state, or NULL.
 * \return          Committed byte count, or zero for invalid state.
 */
size_t xgct_ring_readable(const xgct_ring_buffer_t* ring);
/**
 * \brief           Query unreserved free bytes.
 * \param[in]       ring: Initialized state, or NULL.
 * \return          Free bytes excluding active write reservation, or zero.
 */
size_t xgct_ring_writable(const xgct_ring_buffer_t* ring);
/**
 * \brief           Reserve a contiguous writable span, possibly shorter.
 * \param[in,out]   ring: Initialized state.
 * \param[in]       requested: Nonzero upper bound in bytes.
 * \param[out]      span: Separate output descriptor, unchanged on failure.
 * \return          OK, INVALID_ARGUMENT, BUSY, or CAPACITY (full or ID exhausted).
 * \note            RX DMA writes remain unpublished until write_finish.
 */
xgs_status_t xgct_ring_write_claim(xgct_ring_buffer_t* ring, size_t requested,
                                  xgct_ring_write_span_t* span);
/**
 * \brief           Publish a prefix and release the entire reservation.
 * \param[in,out]   ring: Initialized state.
 * \param[in]       token: Current writer claim identity.
 * \param[in]       completed: Prefix byte count, from zero through span.size.
 * \return          OK or INVALID_ARGUMENT; invalid completion changes nothing.
 * \note            Duplicate/stale IDs within this lifetime are rejected.
 */
xgs_status_t xgct_ring_write_finish(xgct_ring_buffer_t* ring, uint64_t token,
                                   size_t completed);
/**
 * \brief           Release a stopped writer reservation without publishing.
 * \param[in,out]   ring: Initialized state.
 * \param[in]       token: Current writer claim identity.
 * \return          OK or INVALID_ARGUMENT.
 */
xgs_status_t xgct_ring_write_cancel(xgct_ring_buffer_t* ring, uint64_t token);
/**
 * \brief           Reserve a contiguous readable span, possibly shorter.
 * \param[in,out]   ring: Initialized state.
 * \param[in]       requested: Nonzero upper bound in bytes.
 * \param[out]      span: Separate output descriptor, unchanged on failure.
 * \return          OK, INVALID_ARGUMENT, BUSY, or CAPACITY (empty or ID exhausted).
 * \note            TX DMA may read these bytes until read_finish/cancel.
 */
xgs_status_t xgct_ring_read_claim(xgct_ring_buffer_t* ring, size_t requested,
                                 xgct_ring_read_span_t* span);
/**
 * \brief           Consume a prefix; unconsumed bytes remain queued.
 * \param[in,out]   ring: Initialized state.
 * \param[in]       token: Current reader claim identity.
 * \param[in]       completed: Prefix byte count, from zero through span.size.
 * \return          OK or INVALID_ARGUMENT; failure changes nothing.
 */
xgs_status_t xgct_ring_read_finish(xgct_ring_buffer_t* ring, uint64_t token,
                                  size_t completed);
/**
 * \brief           Release a stopped reader reservation without consuming.
 * \param[in,out]   ring: Initialized state.
 * \param[in]       token: Current reader claim identity.
 * \return          OK or INVALID_ARGUMENT.
 */
xgs_status_t xgct_ring_read_cancel(xgct_ring_buffer_t* ring, uint64_t token);
/**
 * \brief           Copy up to the free capacity, including across wrap.
 * \param[in,out]   ring: Initialized state with no writer claim.
 * \param[in]       source: Readable non-overlapping bytes; NULL for zero only.
 * \param[in]       size: Requested bytes.
 * \return          Copied count; zero on invalid input, writer busy, or full.
 */
size_t xgct_ring_write(xgct_ring_buffer_t* ring, const void* source, size_t size);
/**
 * \brief           Copy and consume up to the committed byte count.
 * \param[in,out]   ring: Initialized state with no reader claim.
 * \param[out]      destination: Non-overlapping bytes; NULL for zero only.
 * \param[in]       size: Requested bytes.
 * \return          Copied count; zero on invalid input, reader busy, or empty.
 */
size_t xgct_ring_read(xgct_ring_buffer_t* ring, void* destination, size_t size);
#ifdef __cplusplus
}
#endif
#endif

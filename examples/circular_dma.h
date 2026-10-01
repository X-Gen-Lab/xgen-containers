/**
 * \file            circular_dma.h
 * \brief           Explicit circular-DMA event accounting integration example
 */

#ifndef XGCT_EXAMPLE_CIRCULAR_DMA_H
#define XGCT_EXAMPLE_CIRCULAR_DMA_H

#include <stdbool.h>
#include <xgen/containers/ring_buffer.h>

/** \brief           Board-owned DMA snapshot accounting; no hardware control.
 */
typedef struct {
    const uint8_t* storage; /**< Stable DMA snapshot bytes. */
    size_t capacity;        /**< Circular DMA byte capacity. */
    uint64_t consumed;      /**< Absolute copied byte count, never wraps. */
    bool overrun; /**< Latched overwrite requiring explicit board recovery. */
} dma_cycle_t;

/**
 * \brief           Copy a stable hardware snapshot into a bounded software
 * ring.
 * \param[in,out]   cycle: Initialized with storage/capacity and zero counters.
 * \param[in,out]   destination: Distinct software ring, externally serialized.
 * \param[in]       produced: Monotonic absolute hardware production count.
 * \return          OK when all pending bytes were copied, BUSY for active
 *                  software writer, CAPACITY for backpressure/overrun, or
 *                  INVALID_ARGUMENT for invalid or regressing counters.
 * \note            This example deliberately copies circular DMA snapshots;
 *                  one-shot ring claims are the separate zero-copy path.
 *                  The board must provide coherent, non-overwritten bytes for
 *                  this entire call (stop DMA or prove a sufficient guard).
 *                  Produced counts require lossless wrap/half-transfer events.
 *                  A modulo DMA position alone cannot detect multiple wraps.
 *                  Stop hardware, account lost data, reset counters and rings
 *                  explicitly to recover a latched overrun.
 */
xgs_status_t dma_cycle_drain(dma_cycle_t* cycle,
                             xgct_ring_buffer_t* destination,
                             uint64_t produced);

#endif

/**
 * \file            circular_dma.c
 * \brief           Drain coherent circular DMA snapshots with explicit counts
 */
#include "circular_dma.h"

xgs_status_t dma_cycle_drain(dma_cycle_t* cycle,
                             xgct_ring_buffer_t* destination,
                             uint64_t produced) {
    if (cycle == NULL || cycle->storage == NULL || cycle->capacity == 0U ||
        destination == NULL || destination->storage == NULL ||
        destination->capacity == 0U || produced < cycle->consumed) {
        return XGS_INVALID_ARGUMENT;
    }
    if (cycle->overrun || produced - cycle->consumed > cycle->capacity) {
        cycle->overrun = true;
        return XGS_CAPACITY;
    }
    if (destination->write_token != 0U) {
        return XGS_BUSY;
    }
    while (cycle->consumed != produced) {
        size_t offset = (size_t)(cycle->consumed % cycle->capacity);
        size_t amount = (size_t)(produced - cycle->consumed);
        if (amount > cycle->capacity - offset) {
            amount = cycle->capacity - offset;
        }
        size_t copied =
            xgct_ring_write(destination, cycle->storage + offset, amount);
        cycle->consumed += copied;
        if (copied != amount) {
            return XGS_CAPACITY;
        }
    }
    return XGS_OK;
}

/**
 * \file            test_dma_cycle.cpp
 * \brief           Container behavioral contract tests
 */
extern "C" {
#include "../../examples/circular_dma.h"
}
#include <cstring>
#include <gtest/gtest.h>
TEST(CircularDma,
     CountsWrapEventsAndBackpressureWithoutGuessingModuloPosition) {
    uint8_t hardware[4] = {'a', 'b', 'c', 'd'};
    uint8_t software[3]{};
    uint8_t output[4]{};
    xgct_ring_buffer_t ring{};
    ASSERT_EQ(xgct_ring_init(&ring, software, 3), XGS_OK);
    dma_cycle_t cycle{hardware, 4, 0, false};
    EXPECT_EQ(dma_cycle_drain(&cycle, &ring, 4), XGS_CAPACITY);
    EXPECT_EQ(cycle.consumed, 3U);
    EXPECT_FALSE(cycle.overrun);
    EXPECT_EQ(xgct_ring_read(&ring, output, 3), 3U);
    EXPECT_EQ(std::memcmp(output, "abc", 3), 0);
    hardware[0] = 'e';
    hardware[1] = 'f';
    EXPECT_EQ(dma_cycle_drain(&cycle, &ring, 6), XGS_OK);
    EXPECT_EQ(xgct_ring_read(&ring, output, 3), 3U);
    EXPECT_EQ(std::memcmp(output, "def", 3), 0);
    EXPECT_EQ(dma_cycle_drain(&cycle, &ring, 6), XGS_OK);
    EXPECT_EQ(dma_cycle_drain(&cycle, &ring, 5), XGS_INVALID_ARGUMENT);
    EXPECT_EQ(dma_cycle_drain(&cycle, &ring, 11), XGS_CAPACITY);
    EXPECT_TRUE(cycle.overrun);
    EXPECT_EQ(cycle.consumed, 6U);
    EXPECT_EQ(dma_cycle_drain(&cycle, &ring, 11), XGS_CAPACITY);
}
TEST(CircularDma, RejectsInvalidObjectsAndDoesNotCompeteWithRxReservation) {
    uint8_t hardware[2] = {1, 2};
    uint8_t software[4]{};
    xgct_ring_buffer_t ring{};
    dma_cycle_t cycle{hardware, 2, 0, false};
    EXPECT_EQ(dma_cycle_drain(nullptr, &ring, 0), XGS_INVALID_ARGUMENT);
    EXPECT_EQ(dma_cycle_drain(&cycle, nullptr, 0), XGS_INVALID_ARGUMENT);
    EXPECT_EQ(dma_cycle_drain(&cycle, &ring, 0), XGS_INVALID_ARGUMENT);
    ASSERT_EQ(xgct_ring_init(&ring, software, 4), XGS_OK);
    xgct_ring_write_span_t span{};
    ASSERT_EQ(xgct_ring_write_claim(&ring, 4, &span), XGS_OK);
    EXPECT_EQ(dma_cycle_drain(&cycle, &ring, 1), XGS_BUSY);
    EXPECT_EQ(cycle.consumed, 0U);
    ASSERT_EQ(xgct_ring_write_cancel(&ring, span.token), XGS_OK);
    cycle.storage = nullptr;
    EXPECT_EQ(dma_cycle_drain(&cycle, &ring, 1), XGS_INVALID_ARGUMENT);
    cycle.storage = hardware;
    cycle.capacity = 0;
    EXPECT_EQ(dma_cycle_drain(&cycle, &ring, 1), XGS_INVALID_ARGUMENT);
}

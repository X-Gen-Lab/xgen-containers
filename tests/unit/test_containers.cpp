#include <xgen/containers/bitset.h>
#include <xgen/containers/ring_buffer.h>
#include <xgen/containers/list.h>
#include <gtest/gtest.h>
#include <array>
#include <cstring>
#include <deque>
#include <limits>

TEST(Bitset, BoundsTailAndBorrowedView) {
    uint8_t bytes[4] = {255, 255, 255, 99};
    xgct_bitset_t set{};
    EXPECT_EQ(xgct_bitset_storage_size(0), 0U);
    EXPECT_EQ(xgct_bitset_storage_size(SIZE_MAX), SIZE_MAX / 8 + 1);
    ASSERT_EQ(xgct_bitset_init(&set, bytes, sizeof(bytes), 17), XGS_OK);
    EXPECT_EQ(bytes[3], 99);
    for (size_t i = 0; i < 17; ++i) {
        EXPECT_FALSE(xgct_bitset_test(&set, i));
        EXPECT_EQ(xgct_bitset_set(&set, i), XGS_OK);
        EXPECT_TRUE(xgct_bitset_test(&set, i));
    }
    EXPECT_EQ(bytes[2], 1);
    xgct_bitset_t view{bytes, 17};
    EXPECT_EQ(xgct_bitset_clear(&view, 16), XGS_OK);
    EXPECT_FALSE(xgct_bitset_test(&set, 16));
    EXPECT_EQ(xgct_bitset_set(&set, 17), XGS_INVALID_ARGUMENT);
    EXPECT_EQ(xgct_bitset_clear(&set, SIZE_MAX), XGS_INVALID_ARGUMENT);
    EXPECT_FALSE(xgct_bitset_test(&set, 17));
    xgct_bitset_clear_all(&set);
    EXPECT_EQ(bytes[0], 0);
    EXPECT_EQ(bytes[3], 99);
}
TEST(Bitset, InvalidInitializationAndQueriesDoNotModifyObjects) {
    uint8_t bytes[1] = {77};
    xgct_bitset_t set{bytes, 8};
    EXPECT_EQ(xgct_bitset_init(nullptr, bytes, 1, 1), XGS_INVALID_ARGUMENT);
    EXPECT_EQ(xgct_bitset_init(&set, nullptr, 1, 1), XGS_INVALID_ARGUMENT);
    EXPECT_EQ(xgct_bitset_init(&set, bytes, 1, 0), XGS_INVALID_ARGUMENT);
    EXPECT_EQ(xgct_bitset_init(&set, bytes, 1, 9), XGS_CAPACITY);
    EXPECT_EQ(set.bit_count, 8U);
    EXPECT_EQ(bytes[0], 77);
    EXPECT_EQ(xgct_bitset_set(nullptr, 0), XGS_INVALID_ARGUMENT);
    EXPECT_EQ(xgct_bitset_clear(nullptr, 0), XGS_INVALID_ARGUMENT);
    EXPECT_FALSE(xgct_bitset_test(nullptr, 0));
    xgct_bitset_clear_all(nullptr);
    set.storage = nullptr;
    EXPECT_EQ(xgct_bitset_set(&set, 0), XGS_INVALID_ARGUMENT);
    EXPECT_EQ(xgct_bitset_clear(&set, 0), XGS_INVALID_ARGUMENT);
    EXPECT_FALSE(xgct_bitset_test(&set, 0));
    xgct_bitset_clear_all(&set);
}
TEST(Ring, DmaRxTxPartialCompletionAndCancellation) {
    uint8_t bytes[8]{};
    xgct_ring_buffer_t ring{};
    ASSERT_EQ(xgct_ring_init(&ring, bytes, 8), XGS_OK);
    xgct_ring_write_span_t rx{};
    ASSERT_EQ(xgct_ring_write_claim(&ring, 6, &rx), XGS_OK);
    EXPECT_EQ(rx.size, 6U);
    EXPECT_EQ(xgct_ring_readable(&ring), 0U);
    EXPECT_EQ(xgct_ring_writable(&ring), 2U);
    EXPECT_EQ(xgct_ring_write_claim(&ring, 1, &rx), XGS_BUSY);
    EXPECT_EQ(xgct_ring_reset(&ring), XGS_BUSY);
    EXPECT_EQ(xgct_ring_deinit(&ring), XGS_BUSY);
    EXPECT_EQ(xgct_ring_write(&ring, "x", 1), 0U);
    std::memcpy(rx.data, "abcdef", 6);
    EXPECT_EQ(xgct_ring_write_finish(&ring, rx.token, 7), XGS_INVALID_ARGUMENT);
    EXPECT_EQ(xgct_ring_write_finish(&ring, rx.token + 1, 4), XGS_INVALID_ARGUMENT);
    ASSERT_EQ(xgct_ring_write_finish(&ring, rx.token, 4), XGS_OK);
    EXPECT_EQ(xgct_ring_write_finish(&ring, rx.token, 4), XGS_INVALID_ARGUMENT);
    xgct_ring_read_span_t tx{};
    ASSERT_EQ(xgct_ring_read_claim(&ring, 8, &tx), XGS_OK);
    EXPECT_EQ(tx.size, 4U);
    EXPECT_EQ(std::memcmp(tx.data, "abcd", 4), 0);
    EXPECT_EQ(xgct_ring_read_claim(&ring, 1, &tx), XGS_BUSY);
    EXPECT_EQ(xgct_ring_read(&ring, bytes, 1), 0U);
    EXPECT_EQ(xgct_ring_reset(&ring), XGS_BUSY);
    EXPECT_EQ(xgct_ring_read_finish(&ring, tx.token, 5), XGS_INVALID_ARGUMENT);
    ASSERT_EQ(xgct_ring_write_claim(&ring, 8, &rx), XGS_OK);
    EXPECT_EQ(rx.size, 4U);
    EXPECT_NE(rx.data, tx.data);
    EXPECT_EQ(xgct_ring_write_cancel(&ring, rx.token), XGS_OK);
    EXPECT_EQ(xgct_ring_write_cancel(&ring, rx.token), XGS_INVALID_ARGUMENT);
    EXPECT_EQ(xgct_ring_read_finish(&ring, tx.token, 2), XGS_OK);
    EXPECT_EQ(xgct_ring_read_finish(&ring, tx.token, 2), XGS_INVALID_ARGUMENT);
    ASSERT_EQ(xgct_ring_read_claim(&ring, 8, &tx), XGS_OK);
    EXPECT_EQ(tx.size, 2U);
    EXPECT_EQ(std::memcmp(tx.data, "cd", 2), 0);
    EXPECT_EQ(xgct_ring_read_cancel(&ring, tx.token), XGS_OK);
    EXPECT_EQ(xgct_ring_readable(&ring), 2U);
    EXPECT_EQ(xgct_ring_reset(&ring), XGS_OK);
    EXPECT_EQ(xgct_ring_deinit(&ring), XGS_OK);
}
TEST(Ring, WrapCapacityOneAndCopyReferenceModel) {
    for (size_t capacity : {size_t{1}, size_t{7}, size_t{16}}) {
        std::array<uint8_t, 16> bytes{};
        xgct_ring_buffer_t ring{};
        ASSERT_EQ(xgct_ring_init(&ring, bytes.data(), capacity), XGS_OK);
        std::deque<uint8_t> model;
        uint32_t seed = 12345;
        for (size_t iteration = 0; iteration < 2000; ++iteration) {
            seed = seed * 1664525U + 1013904223U;
            const size_t request = (seed >> 8U) % 20U;
            uint8_t data[20];
            for (size_t j = 0; j < 20; ++j) { data[j] = static_cast<uint8_t>(iteration + j); }
            if ((seed & 4U) != 0) {
                const size_t actual = xgct_ring_write(&ring, data, request);
                ASSERT_EQ(actual, std::min(request, capacity - model.size()));
                for (size_t j = 0; j < actual; ++j) { model.push_back(data[j]); }
            } else {
                const size_t actual = xgct_ring_read(&ring, data, request);
                ASSERT_EQ(actual, std::min(request, model.size()));
                for (size_t j = 0; j < actual; ++j) { EXPECT_EQ(data[j], model.front()); model.pop_front(); }
            }
            EXPECT_EQ(xgct_ring_readable(&ring), model.size());
            EXPECT_EQ(xgct_ring_writable(&ring), capacity - model.size());
        }
    }
}
TEST(Ring, ClaimsAreContiguousAcrossWrapAndStaleIdsAreRejected) {
    uint8_t bytes[7]{};
    uint8_t output[7]{};
    xgct_ring_buffer_t ring{};
    ASSERT_EQ(xgct_ring_init(&ring, bytes, 7), XGS_OK);
    EXPECT_EQ(xgct_ring_write(&ring, "abcde", 5), 5U);
    EXPECT_EQ(xgct_ring_read(&ring, output, 4), 4U);
    xgct_ring_write_span_t span{};
    ASSERT_EQ(xgct_ring_write_claim(&ring, 6, &span), XGS_OK);
    EXPECT_EQ(span.size, 2U);
    std::memcpy(span.data, "fg", 2);
    const uint64_t old = span.token;
    EXPECT_EQ(xgct_ring_write_finish(&ring, span.token, 2), XGS_OK);
    ASSERT_EQ(xgct_ring_write_claim(&ring, 6, &span), XGS_OK);
    EXPECT_EQ(span.size, 4U);
    EXPECT_EQ(xgct_ring_write_finish(&ring, old, 1), XGS_INVALID_ARGUMENT);
    std::memcpy(span.data, "hijk", 4);
    EXPECT_EQ(xgct_ring_write_finish(&ring, span.token, 4), XGS_OK);
    EXPECT_EQ(xgct_ring_write_claim(&ring, 1, &span), XGS_CAPACITY);
    EXPECT_EQ(xgct_ring_read(&ring, output, 7), 7U);
    EXPECT_EQ(std::memcmp(output, "efghijk", 7), 0);
    EXPECT_EQ(xgct_ring_reset(&ring), XGS_OK);
    ASSERT_EQ(xgct_ring_write_claim(&ring, 1, &span), XGS_OK);
    EXPECT_GT(span.token, old);
    EXPECT_EQ(xgct_ring_write_cancel(&ring, span.token), XGS_OK);
}
TEST(Ring, InvalidArgumentsLeaveClaimsAndOutputsUnchanged) {
    xgct_ring_buffer_t ring{};
    uint8_t byte{};
    xgct_ring_write_span_t write{&byte, 17, 19};
    xgct_ring_read_span_t read{&byte, 17, 19};
    EXPECT_EQ(xgct_ring_init(nullptr, &byte, 1), XGS_INVALID_ARGUMENT);
    EXPECT_EQ(xgct_ring_init(&ring, nullptr, 1), XGS_INVALID_ARGUMENT);
    EXPECT_EQ(xgct_ring_init(&ring, &byte, 0), XGS_INVALID_ARGUMENT);
    EXPECT_EQ(xgct_ring_readable(nullptr), 0U);
    EXPECT_EQ(xgct_ring_writable(nullptr), 0U);
    EXPECT_EQ(xgct_ring_reset(nullptr), XGS_INVALID_ARGUMENT);
    EXPECT_EQ(xgct_ring_deinit(&ring), XGS_INVALID_ARGUMENT);
    EXPECT_EQ(xgct_ring_write_claim(nullptr, 1, &write), XGS_INVALID_ARGUMENT);
    EXPECT_EQ(xgct_ring_read_claim(&ring, 1, &read), XGS_INVALID_ARGUMENT);
    EXPECT_EQ(xgct_ring_write_finish(nullptr, 1, 0), XGS_INVALID_ARGUMENT);
    EXPECT_EQ(xgct_ring_read_finish(nullptr, 1, 0), XGS_INVALID_ARGUMENT);
    EXPECT_EQ(xgct_ring_write(nullptr, &byte, 1), 0U);
    EXPECT_EQ(xgct_ring_read(nullptr, &byte, 1), 0U);
    ASSERT_EQ(xgct_ring_init(&ring, &byte, 1), XGS_OK);
    EXPECT_EQ(xgct_ring_write_claim(&ring, 0, &write), XGS_INVALID_ARGUMENT);
    EXPECT_EQ(xgct_ring_write_claim(&ring, 1, nullptr), XGS_INVALID_ARGUMENT);
    EXPECT_EQ(xgct_ring_read_claim(&ring, 0, &read), XGS_INVALID_ARGUMENT);
    EXPECT_EQ(xgct_ring_read_claim(&ring, 1, nullptr), XGS_INVALID_ARGUMENT);
    EXPECT_EQ(xgct_ring_read_claim(&ring, 1, &read), XGS_CAPACITY);
    EXPECT_EQ(write.token, 19U);
    EXPECT_EQ(read.token, 19U);
    EXPECT_EQ(xgct_ring_write(&ring, nullptr, 1), 0U);
    EXPECT_EQ(xgct_ring_read(&ring, nullptr, 1), 0U);
    EXPECT_EQ(xgct_ring_write_cancel(&ring, 0), XGS_INVALID_ARGUMENT);
    EXPECT_EQ(xgct_ring_read_cancel(&ring, 0), XGS_INVALID_ARGUMENT);
    ring.sequence = UINT64_MAX;
    EXPECT_EQ(xgct_ring_write_claim(&ring, 1, &write), XGS_CAPACITY);
    EXPECT_EQ(xgct_ring_write(&ring, "x", 1), 1U);
    EXPECT_EQ(xgct_ring_read_claim(&ring, 1, &read), XGS_CAPACITY);
}

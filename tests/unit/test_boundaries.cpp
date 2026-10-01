/**
 * \file            test_boundaries.cpp
 * \brief           Intrusive ownership, invalid argument and collision tests
 */
#include <gtest/gtest.h>
#include <xgen/containers/hash.h>
#include <xgen/containers/list.h>

TEST(List, NullQueriesAndMutationsAreDefined) {
    xgct_list_t list{};
    xgct_list_node_t a{}, b{}, c{};
    xgct_list_init(nullptr);
    xgct_list_node_init(nullptr);
    EXPECT_TRUE(xgct_list_is_empty(nullptr));
    EXPECT_EQ(xgct_list_count(nullptr), 0U);
    EXPECT_EQ(xgct_list_peek_head(nullptr), nullptr);
    EXPECT_EQ(xgct_list_peek_tail(nullptr), nullptr);
    EXPECT_EQ(xgct_list_next(nullptr), nullptr);
    EXPECT_EQ(xgct_list_prev(nullptr), nullptr);
    EXPECT_EQ(xgct_list_remove_head(nullptr), nullptr);
    EXPECT_EQ(xgct_list_remove_tail(nullptr), nullptr);
    EXPECT_EQ(xgct_list_remove_head(&list), nullptr);
    EXPECT_EQ(xgct_list_remove_tail(&list), nullptr);
    xgct_list_insert_head(nullptr, &a);
    xgct_list_insert_head(&list, nullptr);
    xgct_list_insert_tail(nullptr, &a);
    xgct_list_insert_tail(&list, nullptr);
    xgct_list_insert_before(nullptr, &a, &b);
    xgct_list_insert_before(&list, nullptr, &b);
    xgct_list_insert_before(&list, &a, nullptr);
    xgct_list_insert_after(nullptr, &a, &b);
    xgct_list_insert_after(&list, nullptr, &b);
    xgct_list_insert_after(&list, &a, nullptr);
    xgct_list_remove(nullptr, &a);
    xgct_list_remove(&list, nullptr);
    EXPECT_TRUE(xgct_list_is_empty(&list));
    xgct_list_insert_tail(&list, &a);
    xgct_list_insert_before(&list, &a, &b);
    xgct_list_insert_after(&list, &a, &c);
    EXPECT_EQ(xgct_list_peek_head(&list), &b);
    EXPECT_EQ(xgct_list_peek_tail(&list), &c);
    xgct_list_remove(&list, &a);
    EXPECT_EQ(xgct_list_next(&b), &c);
    EXPECT_EQ(xgct_list_prev(&c), &b);
    EXPECT_EQ(xgct_list_remove_head(&list), &b);
    EXPECT_EQ(xgct_list_remove_tail(&list), &c);
    EXPECT_TRUE(xgct_list_validate(&list));
}

TEST(List, EntryAndTraversalPreserveContainingObjects) {
    struct Item {
        int value;
        xgct_list_node_t node;
    } items[3]{};

    xgct_list_t list{};
    for (int i = 0; i < 3; ++i) {
        items[i].value = i;
        xgct_list_insert_tail(&list, &items[i].node);
    }
    xgct_list_node_t* node;
    int sum = 0;
    XGCT_LIST_FOR_EACH(&list, node) {
        sum += XGCT_LIST_ENTRY(node, Item, node)->value;
    }
    EXPECT_EQ(sum, 3);
    xgct_list_clear(&list);
    EXPECT_EQ(items[2].value, 2);
}

TEST(Hash, InvalidInputAndAllCollisionChainRemovalPositions) {
    xgct_hash_t table{}, other{}, empty{};
    xgct_hash_node_t* buckets[2]{};
    xgct_hash_node_t* other_buckets[1]{};
    xgct_hash_node_t nodes[4]{};
    xgct_hash_node_init(nullptr);
    xgct_hash_node_init(&nodes[0]);
    EXPECT_EQ(xgct_hash_init(nullptr, buckets, 1, 3), XGS_INVALID_ARGUMENT);
    EXPECT_EQ(xgct_hash_init(&table, nullptr, 1, 3), XGS_INVALID_ARGUMENT);
    EXPECT_EQ(xgct_hash_init(&table, buckets, 0, 3), XGS_INVALID_ARGUMENT);
    EXPECT_EQ(xgct_hash_init(&table, buckets, 3, 3), XGS_INVALID_ARGUMENT);
    EXPECT_EQ(xgct_hash_init(&table, buckets, 1, 0), XGS_INVALID_ARGUMENT);
    const size_t excessive = size_t{1} << (sizeof(size_t) * 8U - 1U);
    EXPECT_EQ(xgct_hash_init(&table, buckets, excessive, 3),
              XGS_INVALID_ARGUMENT);
    EXPECT_EQ(xgct_hash_insert(nullptr, &nodes[0], 0, nullptr),
              XGS_INVALID_ARGUMENT);
    EXPECT_EQ(xgct_hash_insert(&empty, &nodes[0], 0, nullptr),
              XGS_INVALID_ARGUMENT);
    EXPECT_EQ(xgct_hash_find(nullptr, 0), nullptr);
    EXPECT_EQ(xgct_hash_find(&empty, 0), nullptr);
    EXPECT_EQ(xgct_hash_remove(nullptr, 0), nullptr);
    EXPECT_EQ(xgct_hash_remove(&empty, 0), nullptr);
    EXPECT_EQ(xgct_hash_count(nullptr), 0U);
    xgct_hash_clear(nullptr);
    xgct_hash_clear(&empty);
    ASSERT_EQ(xgct_hash_init(&table, buckets, 1, 3), XGS_OK);
    ASSERT_EQ(xgct_hash_init(&other, other_buckets, 1, 1), XGS_OK);
    EXPECT_EQ(xgct_hash_insert(&table, nullptr, 0, nullptr),
              XGS_INVALID_ARGUMENT);
    for (uint32_t i = 0; i < 3; ++i) {
        ASSERT_EQ(xgct_hash_insert(&table, &nodes[i], i, nullptr), XGS_OK);
    }
    EXPECT_EQ(xgct_hash_insert(&other, &nodes[0], 0, nullptr), XGS_BUSY);
    EXPECT_EQ(xgct_hash_insert(&table, &nodes[3], 0, nullptr),
              XGS_ALREADY_EXISTS);
    EXPECT_EQ(xgct_hash_insert(&table, &nodes[3], 3, nullptr), XGS_CAPACITY);
    EXPECT_EQ(xgct_hash_remove(&table, 999), nullptr);
    EXPECT_EQ(xgct_hash_remove(&table, 1), &nodes[1]);
    EXPECT_EQ(xgct_hash_remove(&table, 0), &nodes[0]);
    EXPECT_EQ(xgct_hash_remove(&table, 2), &nodes[2]);
    EXPECT_EQ(xgct_hash_count(&table), 0U);
    EXPECT_TRUE(xgct_hash_validate(&table));
    table.bucket_count = 0;
    EXPECT_EQ(xgct_hash_insert(&table, &nodes[0], 0, nullptr),
              XGS_INVALID_ARGUMENT);
    EXPECT_EQ(xgct_hash_find(&table, 0), nullptr);
    EXPECT_EQ(xgct_hash_remove(&table, 0), nullptr);
    EXPECT_FALSE(xgct_hash_validate(&table));
    table.bucket_count = 3;
    EXPECT_FALSE(xgct_hash_validate(&table));
    table.bucket_count = 1;
    table.capacity = 0;
    EXPECT_FALSE(xgct_hash_validate(&table));
    table.capacity = 1;
    table.count = 2;
    EXPECT_FALSE(xgct_hash_validate(&table));
}

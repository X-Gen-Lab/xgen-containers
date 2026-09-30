#include <xgen/containers/list.h>
#include <xgen/containers/hash.h>
#include <gtest/gtest.h>
TEST(Migration, List)
{
    xgct_list_t list;
    xgct_list_node_t nodes[4];
    xgct_list_init(&list);
    for (size_t i = 0U; i < 4U; ++i) {
        xgct_list_node_init(&nodes[i]);
    }
    EXPECT_TRUE(xgct_list_is_empty(&list));
    xgct_list_insert_head(&list, &nodes[1]);
    xgct_list_insert_before(&list, &nodes[1], &nodes[0]);
    xgct_list_insert_after(&list, &nodes[1], &nodes[2]);
    xgct_list_insert_tail(&list, &nodes[3]);
    EXPECT_TRUE(xgct_list_count(&list) == 4U);
    EXPECT_TRUE(xgct_list_peek_head(&list) == &nodes[0]);
    EXPECT_TRUE(xgct_list_peek_tail(&list) == &nodes[3]);
    for (size_t i = 0U; i < 3U; ++i) {
        EXPECT_TRUE(xgct_list_next(&nodes[i]) == &nodes[i + 1U]);
        EXPECT_TRUE(xgct_list_prev(&nodes[i + 1U]) == &nodes[i]);
    }
    xgct_list_remove(&list, &nodes[1]);
    EXPECT_TRUE(nodes[1].next == NULL && nodes[1].prev == NULL);
    EXPECT_TRUE(xgct_list_remove_tail(&list) == &nodes[3]);
    EXPECT_TRUE(xgct_list_remove_head(&list) == &nodes[0]);
    EXPECT_TRUE(xgct_list_remove_head(&list) == &nodes[2]);
    EXPECT_TRUE(xgct_list_remove_head(&list) == NULL);
    EXPECT_TRUE(xgct_list_count(&list) == 0U && xgct_list_is_empty(&list));
    EXPECT_TRUE(list.head == NULL && list.tail == NULL);
}
TEST(Migration, Hash)
{
    xgct_hash_t table, other;
    xgct_hash_node_t *buckets[1], *other_buckets[2];
    xgct_hash_node_t nodes[4] = {};
    int value = 42;
    EXPECT_TRUE(xgct_hash_init(&table, buckets, 1U, 2U) == XGS_OK);
    EXPECT_TRUE(xgct_hash_init(&other, other_buckets, 2U, 2U) == XGS_OK);
    EXPECT_TRUE(xgct_hash_insert(&table, &nodes[0], 3U, &value) == XGS_OK);
    EXPECT_TRUE(xgct_hash_insert(&table, &nodes[1], 5U, NULL) == XGS_OK);
    EXPECT_TRUE(xgct_hash_find(&table, 3U)->value == &value);
    EXPECT_TRUE(xgct_hash_find(&table, 5U) == &nodes[1]);
    EXPECT_TRUE(xgct_hash_find(&table, 5U)->value == NULL);
    EXPECT_TRUE(xgct_hash_find(&table, 7U) == NULL);
    EXPECT_TRUE(xgct_hash_insert(&table, &nodes[2], 3U, NULL) == XGS_ALREADY_EXISTS);
    EXPECT_TRUE(xgct_hash_insert(&table, &nodes[2], 7U, NULL) == XGS_CAPACITY);
    EXPECT_TRUE(xgct_hash_insert(&other, &nodes[0], 9U, NULL) == XGS_BUSY);
    EXPECT_TRUE(xgct_hash_remove(&table, 3U) == &nodes[0]);
    EXPECT_TRUE(nodes[0].owner == NULL && nodes[0].next == NULL);
    EXPECT_TRUE(xgct_hash_insert(&other, &nodes[0], 9U, &value) == XGS_OK);
    EXPECT_TRUE(xgct_hash_insert(&table, &nodes[2], 7U, NULL) == XGS_OK);
    EXPECT_TRUE(xgct_hash_count(&table) == 2U);
    xgct_hash_clear(&table);
    EXPECT_TRUE(xgct_hash_count(&table) == 0U && buckets[0] == NULL);
    EXPECT_TRUE(nodes[1].owner == NULL && nodes[2].owner == NULL);
    EXPECT_TRUE(xgct_hash_count(&other) == 1U);
    xgct_hash_clear(&other);
    EXPECT_TRUE(value == 42);
}

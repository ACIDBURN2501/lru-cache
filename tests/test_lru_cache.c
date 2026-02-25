/**
 * @file test_lru_cache.c
 * @brief Unit tests for the LRU cache library.
 *
 * Compiled twice by the build system -- once with HASH strategy and once
 * with LINEAR strategy -- so every test exercises both code paths.
 *
 * Each test function returns 0 on pass, 1 on failure.
 * main() returns 0 if all tests pass, 1 otherwise.
 */

#include "lru_cache.h"

#include <stdint.h>
#include <stdio.h>

/* -------------------------------------------------------------------------
 * Minimal test runner (no external dependencies)
 * ------------------------------------------------------------------------- */

static int g_tests_run = 0;
static int g_tests_failed = 0;

#define RUN_TEST(fn)                                                           \
        do {                                                                   \
                g_tests_run++;                                                 \
                if ((fn)() != 0) {                                             \
                        printf("FAIL  %s\n", #fn);                             \
                        g_tests_failed++;                                      \
                } else {                                                       \
                        printf("pass  %s\n", #fn);                             \
                }                                                              \
        } while (0)

/* -------------------------------------------------------------------------
 * Helper: print the active lookup strategy
 * ------------------------------------------------------------------------- */

static void
print_strategy(void)
{
#if (LRU_CACHE_LOOKUP_STRATEGY == LRU_CACHE_LOOKUP_HASH)
        printf("=== Strategy: HASH (LRU_CACHE_LOOKUP_HASH) ===\n");
#else
        printf("=== Strategy: LINEAR (LRU_CACHE_LOOKUP_LINEAR) ===\n");
#endif
}

/* =========================================================================
 * Test 1: After init the cache is empty and get() returns false
 * ========================================================================= */

static int
test_init_empty(void)
{
        lru_cache_t cache;
        uint32_t val = 0xDEADBEEFU;

        lru_cache_init(&cache, 4U);

        if (cache.size != 0U) {
                return 1;
        }
        if (cache.head_idx != -1) {
                return 1;
        }
        if (cache.tail_idx != -1) {
                return 1;
        }
        /* Lookup in an empty cache must return false. */
        if (lru_cache_get(&cache, 1U, &val) != false) {
                return 1;
        }
        return 0;
}

/* =========================================================================
 * Test 2: A stored value can be retrieved by its key
 * ========================================================================= */

static int
test_put_get_single(void)
{
        lru_cache_t cache;
        uint32_t val = 0U;

        lru_cache_init(&cache, 4U);

        if (lru_cache_put(&cache, 42U, 1234U) != true) {
                return 1;
        }
        if (lru_cache_get(&cache, 42U, &val) != true) {
                return 1;
        }
        if (val != 1234U) {
                return 1;
        }
        if (cache.size != 1U) {
                return 1;
        }
        return 0;
}

/* =========================================================================
 * Test 3: Looking up a key that was never inserted returns false
 * ========================================================================= */

static int
test_get_miss(void)
{
        lru_cache_t cache;
        uint32_t val = 0xFFU;

        lru_cache_init(&cache, 4U);
        lru_cache_put(&cache, 10U, 100U);

        /* Key 20 was not inserted. */
        if (lru_cache_get(&cache, 20U, &val) != false) {
                return 1;
        }
        /* val must not have been modified on a miss. */
        if (val != 0xFFU) {
                return 1;
        }
        return 0;
}

/* =========================================================================
 * Test 4: Inserting past capacity evicts the Least Recently Used entry
 * ========================================================================= */

static int
test_eviction_lru(void)
{
        lru_cache_t cache;
        uint32_t val = 0U;

        lru_cache_init(&cache, 3U);

        /* Fill cache: MRU -> 3 -> 2 -> 1 <- LRU */
        lru_cache_put(&cache, 1U, 10U);
        lru_cache_put(&cache, 2U, 20U);
        lru_cache_put(&cache, 3U, 30U);

        if (cache.size != 3U) {
                return 1;
        }

        /* Inserting a 4th item must evict key 1 (LRU). */
        lru_cache_put(&cache, 4U, 40U);

        if (lru_cache_get(&cache, 1U, &val) != false) {
                return 1; /* key 1 should be gone */
        }
        if (lru_cache_get(&cache, 4U, &val) != true || val != 40U) {
                return 1; /* key 4 must be present */
        }
        if (cache.size != 3U) {
                return 1; /* size stays at capacity */
        }
        return 0;
}

/* =========================================================================
 * Test 5: Putting an existing key updates its value without growing the cache
 * ========================================================================= */

static int
test_update_existing_key(void)
{
        lru_cache_t cache;
        uint32_t val = 0U;

        lru_cache_init(&cache, 4U);

        lru_cache_put(&cache, 7U, 100U);
        lru_cache_put(&cache, 7U, 999U); /* update */

        if (lru_cache_get(&cache, 7U, &val) != true) {
                return 1;
        }
        if (val != 999U) {
                return 1;
        }
        if (cache.size != 1U) {
                return 1; /* must not have grown */
        }
        return 0;
}

/* =========================================================================
 * Test 6: get() promotes the accessed entry to MRU, deferring its eviction
 * ========================================================================= */

static int
test_get_promotes_to_mru(void)
{
        lru_cache_t cache;
        uint32_t val = 0U;

        lru_cache_init(&cache, 3U);

        /* Insert order (oldest -> newest): 1, 2, 3 */
        lru_cache_put(&cache, 1U, 10U);
        lru_cache_put(&cache, 2U, 20U);
        lru_cache_put(&cache, 3U, 30U);
        /* MRU -> 3 -> 2 -> 1 <- LRU */

        /* Accessing key 1 makes it MRU; key 2 becomes the new LRU. */
        if (lru_cache_get(&cache, 1U, &val) != true || val != 10U) {
                return 1;
        }
        /* MRU -> 1 -> 3 -> 2 <- LRU */

        /* A new insertion should evict key 2, not key 1. */
        lru_cache_put(&cache, 4U, 40U);

        if (lru_cache_get(&cache, 2U, &val) != false) {
                return 1; /* key 2 should be evicted */
        }
        if (lru_cache_get(&cache, 1U, &val) != true || val != 10U) {
                return 1; /* key 1 must survive */
        }
        if (lru_cache_get(&cache, 3U, &val) != true || val != 30U) {
                return 1; /* key 3 must survive */
        }
        return 0;
}

/* =========================================================================
 * Test 7: Null pointer arguments are handled safely (no crash, returns false)
 * ========================================================================= */

static int
test_null_pointer_safety(void)
{
        lru_cache_t cache;
        uint32_t val = 0U;

        /* init with NULL must not crash */
        lru_cache_init(NULL, 4U);

        lru_cache_init(&cache, 4U);

        if (lru_cache_put(NULL, 1U, 10U) != false) {
                return 1;
        }
        if (lru_cache_get(NULL, 1U, &val) != false) {
                return 1;
        }
        if (lru_cache_get(&cache, 1U, NULL) != false) {
                return 1;
        }
        return 0;
}

/* =========================================================================
 * Test 8: Cache can be filled to capacity and all entries retrieved
 * ========================================================================= */

static int
test_fill_to_capacity(void)
{
        lru_cache_t cache;
        uint32_t val = 0U;
        uint32_t k;

        /*
         * Use keys spaced 100 apart to reduce hash-collision probability
         * across the default 17-slot table.
         */
        lru_cache_init(&cache, 8U);

        for (k = 0U; k < 8U; k++) {
                if (lru_cache_put(&cache, (k + 1U) * 100U, k * 10U) != true) {
                        return 1;
                }
        }

        if (cache.size != 8U) {
                return 1;
        }

        for (k = 0U; k < 8U; k++) {
                if (lru_cache_get(&cache, (k + 1U) * 100U, &val) != true) {
                        return 1;
                }
                if (val != k * 10U) {
                        return 1;
                }
        }
        return 0;
}

/* =========================================================================
 * Test 9: The reserved INVALID_KEY is rejected by put()
 * ========================================================================= */

static int
test_invalid_key_rejected(void)
{
        lru_cache_t cache;
        uint32_t val = 0U;

        lru_cache_init(&cache, 4U);

        if (lru_cache_put(&cache, LRU_CACHE_INVALID_KEY, 42U) != false) {
                return 1;
        }
        if (lru_cache_get(&cache, LRU_CACHE_INVALID_KEY, &val) != false) {
                return 1;
        }
        if (cache.size != 0U) {
                return 1;
        }
        return 0;
}

/* =========================================================================
 * Test 10: Sequential evictions maintain correct LRU ordering
 * ========================================================================= */

static int
test_sequential_evictions(void)
{
        lru_cache_t cache;
        uint32_t val = 0U;

        lru_cache_init(&cache, 2U);

        /* Fill: MRU -> B -> A <- LRU */
        lru_cache_put(&cache, 0xA0U, 0xAAU);
        lru_cache_put(&cache, 0xB0U, 0xBBU);

        /* Evict A (LRU), insert C. */
        lru_cache_put(&cache, 0xC0U, 0xCCU);
        if (lru_cache_get(&cache, 0xA0U, &val) != false) {
                return 1; /* A evicted */
        }

        /* Evict B (now LRU), insert D. */
        lru_cache_put(&cache, 0xD0U, 0xDDU);
        if (lru_cache_get(&cache, 0xB0U, &val) != false) {
                return 1; /* B evicted */
        }

        if (lru_cache_get(&cache, 0xC0U, &val) != true || val != 0xCCU) {
                return 1;
        }
        if (lru_cache_get(&cache, 0xD0U, &val) != true || val != 0xDDU) {
                return 1;
        }
        return 0;
}

/* =========================================================================
 * Entry point
 * ========================================================================= */

int
main(void)
{
        print_strategy();

        RUN_TEST(test_init_empty);
        RUN_TEST(test_put_get_single);
        RUN_TEST(test_get_miss);
        RUN_TEST(test_eviction_lru);
        RUN_TEST(test_update_existing_key);
        RUN_TEST(test_get_promotes_to_mru);
        RUN_TEST(test_null_pointer_safety);
        RUN_TEST(test_fill_to_capacity);
        RUN_TEST(test_invalid_key_rejected);
        RUN_TEST(test_sequential_evictions);

        printf("\n%d/%d tests passed.\n", g_tests_run - g_tests_failed,
               g_tests_run);
        return (g_tests_failed == 0) ? 0 : 1;
}

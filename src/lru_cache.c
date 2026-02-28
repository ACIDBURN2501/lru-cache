/**
 * @file lru_cache.c
 * @brief Implementation of the MISRA C compliant LRU Cache.
 */

#include "lru_cache.h"

#include <string.h>

/* -------------------------------------------------------------------------
 * Internal helpers
 * ------------------------------------------------------------------------- */

/**
 * @brief Map a key to a hash-table slot index.
 *
 * Uses a multiplicative (Fibonacci) hash to spread keys across the table.
 * The result is in [0, LRU_CACHE_HASH_TABLE_SIZE).
 *
 * @param key  The input key.
 * @return     Hash table index in range [0, LRU_CACHE_HASH_TABLE_SIZE - 1].
 */
static uint16_t
compute_hash(uint32_t key)
{
        /* Fibonacci / golden-ratio multiplier -- good avalanche for 32-bit keys
         */
        const uint32_t hash_prime = 0x9E3779B9U;
        /*
         * Use modulo (not bitwise AND) so the distribution is correct for
         * any table size, including non-powers-of-two (e.g. the default
         * prime of 17).  Both operands are unsigned, so this is
         * MISRA-compliant.
         */
        return (uint16_t)((key * hash_prime)
                          % (uint32_t)LRU_CACHE_HASH_TABLE_SIZE);
}

/**
 * @brief Detach a node from the doubly-linked LRU list.
 *
 * @param cache_ptr  Cache instance.
 * @param node_idx   Index of the node to remove.
 */
static void
remove_from_list(lru_cache_t *cache_ptr, int16_t node_idx)
{
        if (node_idx < 0 || (uint16_t)node_idx >= LRU_CACHE_MAX_ENTRIES) {
                return; /* bounds check -- Rule 13.3 */
        }

        int16_t prev = cache_ptr->nodes[node_idx].prev_idx;
        int16_t next = cache_ptr->nodes[node_idx].next_idx;

        if (prev != -1) {
                cache_ptr->nodes[prev].next_idx = next;
        } else {
                cache_ptr->head_idx = next; /* was the head */
        }

        if (next != -1) {
                cache_ptr->nodes[next].prev_idx = prev;
        } else {
                cache_ptr->tail_idx = prev; /* was the tail */
        }
}

/**
 * @brief Insert a node at the front (MRU end) of the doubly-linked list.
 *
 * @param cache_ptr  Cache instance.
 * @param node_idx   Index of the node to promote.
 */
static void
add_to_front(lru_cache_t *cache_ptr, int16_t node_idx)
{
        if (node_idx < 0 || (uint16_t)node_idx >= LRU_CACHE_MAX_ENTRIES) {
                return; /* bounds check -- Rule 13.3 */
        }

        cache_ptr->nodes[node_idx].next_idx = cache_ptr->head_idx;
        cache_ptr->nodes[node_idx].prev_idx = -1;

        if (cache_ptr->head_idx != -1) {
                cache_ptr->nodes[cache_ptr->head_idx].prev_idx = node_idx;
        } else {
                cache_ptr->tail_idx = node_idx; /* list was empty */
        }

        cache_ptr->head_idx = node_idx;
}

/**
 * @brief Locate the node index for a key using the configured strategy.
 *
 * @param cache_ptr  Cache instance (const; LRU list not mutated here).
 * @param key        Key to find.
 * @return           Node index on hit, -1 on miss.
 */
static int16_t
find_node_index(const lru_cache_t *cache_ptr, uint32_t key)
{
#if (LRU_CACHE_LOOKUP_STRATEGY == LRU_CACHE_LOOKUP_HASH)
        uint16_t hash_idx = compute_hash(key);
        uint16_t probe_count = 0U;

        while (probe_count < LRU_CACHE_MAX_PROBES) {
                int16_t stored_idx = cache_ptr->hash_table[hash_idx];

                if (stored_idx == (int16_t)-1) {
                        return -1; /* true empty slot -- key absent */
                }

                if (stored_idx == LRU_CACHE_HASH_TOMBSTONE) {
                        /* tombstone from eviction -- continue probing */
                        hash_idx =
                            (uint16_t)((hash_idx + 1U)
                                       % (uint32_t)LRU_CACHE_HASH_TABLE_SIZE);
                        probe_count++;
                        continue;
                }

                if (cache_ptr->nodes[stored_idx].key == key) {
                        return stored_idx;
                }

                /* Linear probe to next slot (wraps around). */
                hash_idx = (uint16_t)((hash_idx + 1U)
                                      % (uint32_t)LRU_CACHE_HASH_TABLE_SIZE);
                probe_count++;
        }
        return -1; /* probe limit reached */

#else /* LRU_CACHE_LOOKUP_LINEAR */
        uint16_t i = 0U;
        while (i < cache_ptr->capacity) {
                if (cache_ptr->nodes[i].key == key) {
                        return (int16_t)i;
                }
                i++;
        }
        return -1;
#endif
}

/* -------------------------------------------------------------------------
 * Public API
 * ------------------------------------------------------------------------- */

void
lru_cache_init(lru_cache_t *cache_ptr, uint16_t capacity)
{
        if (cache_ptr == NULL || capacity > LRU_CACHE_MAX_ENTRIES) {
                return;
        }

        cache_ptr->capacity = (capacity == 0U) ? 1U : capacity;
        cache_ptr->size = 0U;
        cache_ptr->head_idx = -1;
        cache_ptr->tail_idx = -1;

        /* Initialise all node slots (Rule 9.1: no uninitialised reads). */
        for (uint16_t i = 0U; i < LRU_CACHE_MAX_ENTRIES; i++) {
                cache_ptr->nodes[i].key = LRU_CACHE_INVALID_KEY;
                cache_ptr->nodes[i].value = 0U;
                cache_ptr->nodes[i].prev_idx = -1;
                cache_ptr->nodes[i].next_idx = -1;
        }

        /* Mark every hash-table slot as empty. */
        for (uint16_t i = 0U; i < LRU_CACHE_HASH_TABLE_SIZE; i++) {
                cache_ptr->hash_table[i] = -1;
        }
}

bool
lru_cache_get(lru_cache_t *cache_ptr, uint32_t key, uint32_t *out_value)
{
        if (cache_ptr == NULL || out_value == NULL
            || key == LRU_CACHE_INVALID_KEY) {
                return false;
        }

        int16_t node_idx = find_node_index(cache_ptr, key);
        if (node_idx == -1) {
                return false;
        }

        *out_value = cache_ptr->nodes[node_idx].value;

        remove_from_list(cache_ptr, node_idx);
        add_to_front(cache_ptr, node_idx);

        return true;
}

bool
lru_cache_put(lru_cache_t *cache_ptr, uint32_t key, uint32_t value)
{
        if (cache_ptr == NULL || key == LRU_CACHE_INVALID_KEY) {
                return false;
        }

        /* --- Update path: key already present ----------------------------- */
        int16_t node_idx = find_node_index(cache_ptr, key);
        if (node_idx != -1) {
                cache_ptr->nodes[node_idx].value = value;
                remove_from_list(cache_ptr, node_idx);
                add_to_front(cache_ptr, node_idx);
                return true;
        }

        /* --- Insert path -------------------------------------------------- */

        /* Evict LRU tail when the cache is full. */
        if (cache_ptr->size >= cache_ptr->capacity) {
                int16_t lru_idx = cache_ptr->tail_idx;

                if (lru_idx != -1) {
                        /*
                         * Locate and clear the hash-table slot that points to
                         * this node.  We must probe, because the node may have
                         * been placed at a probed offset from its natural slot.
                         */
                        uint16_t old_hash =
                            compute_hash(cache_ptr->nodes[lru_idx].key);
                        uint16_t evict_probe = 0U;
                        while (evict_probe < LRU_CACHE_MAX_PROBES) {
                                if (cache_ptr->hash_table[old_hash]
                                    == lru_idx) {
                                        cache_ptr->hash_table[old_hash] =
                                            LRU_CACHE_HASH_TOMBSTONE;
                                        break;
                                }
                                old_hash =
                                    (uint16_t)((old_hash + 1U)
                                               % (uint32_t)
                                                   LRU_CACHE_HASH_TABLE_SIZE);
                                evict_probe++;
                        }

                        remove_from_list(cache_ptr, lru_idx);
                        cache_ptr->nodes[lru_idx].key = LRU_CACHE_INVALID_KEY;
                }
                /* size stays the same -- we are replacing one item */
        } else {
                cache_ptr->size++;
        }

        /* Find the first free node slot (INVALID_KEY marker). */
        int16_t free_idx = -1;
        uint16_t i = 0U;
        while (i < LRU_CACHE_MAX_ENTRIES) {
                if (cache_ptr->nodes[i].key == LRU_CACHE_INVALID_KEY) {
                        free_idx = (int16_t)i;
                        break;
                }
                i++;
        }

        if (free_idx == -1) {
                return false; /* should never happen; safety fallback */
        }

        /* Write the new node. */
        cache_ptr->nodes[free_idx].key = key;
        cache_ptr->nodes[free_idx].value = value;

        /*
         * Insert into the hash table with linear probing so that colliding
         * keys are placed in adjacent slots and can be found during lookup.
         */
        uint16_t hash_idx = compute_hash(key);
        uint16_t insert_probe = 0U;

        while (insert_probe < LRU_CACHE_MAX_PROBES) {
                if (cache_ptr->hash_table[hash_idx] == (int16_t)-1
                    || cache_ptr->hash_table[hash_idx]
                           == LRU_CACHE_HASH_TOMBSTONE) {
                        /* found empty or tombstone slot - use it */
                        break;
                }
                hash_idx = (uint16_t)((hash_idx + 1U)
                                      % (uint32_t)LRU_CACHE_HASH_TABLE_SIZE);
                insert_probe++;
        }

        if (insert_probe < LRU_CACHE_MAX_PROBES) {
                cache_ptr->hash_table[hash_idx] = free_idx;
        } else {
                return false; /* probe limit reached, insertion failed */
        }

        add_to_front(cache_ptr, free_idx);
        return true;
}

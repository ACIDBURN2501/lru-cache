# ---------------------------------------------------------------------------
# Test suite
#
# The same test source is compiled TWICE -- once per lookup strategy -- so
# that a single `meson test` run covers both code paths.
# ---------------------------------------------------------------------------

# --- HASH strategy ----------------------------------------------------------

test_hash = executable(
  'test_lru_hash',
  ['test_lru_cache.c'],
  dependencies: [lru_cache_dep],
  include_directories: [public_headers],
  c_args: '-DLRU_CACHE_LOOKUP_STRATEGY=LRU_CACHE_LOOKUP_HASH',
)

test(
  'lru_cache / hash strategy',
  test_hash,
  suite: 'unit',
  timeout: 30,
)

# --- LINEAR strategy --------------------------------------------------------

test_linear = executable(
  'test_lru_linear',
  ['test_lru_cache.c'],
  dependencies: [lru_cache_dep],
  include_directories: [public_headers],
  c_args: '-DLRU_CACHE_LOOKUP_STRATEGY=LRU_CACHE_LOOKUP_LINEAR',
)

test(
  'lru_cache / linear strategy',
  test_linear,
  suite: 'unit',
  timeout: 30,
)

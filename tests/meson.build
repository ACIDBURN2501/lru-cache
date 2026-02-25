# ---------------------------------------------------------------------------
# Test suite
#
# The same test source is compiled TWICE -- once per lookup strategy -- so
# that a single `meson test` run covers both code paths.
# ---------------------------------------------------------------------------

_test_src = files('test_lru_cache.c')
_lib_src  = files('../src/lru_cache.c')
_incs     = [public_inc, generated_inc]

# --- HASH strategy ----------------------------------------------------------

_test_hash = executable(
        'test_lru_hash',
        [_test_src, _lib_src],
        include_directories : _incs,
        c_args              : '-DLRU_CACHE_LOOKUP_STRATEGY=LRU_CACHE_LOOKUP_HASH',
)

test('lru_cache / hash strategy', _test_hash,
        suite   : 'unit',
        timeout : 30,
)

# --- LINEAR strategy --------------------------------------------------------

_test_linear = executable(
        'test_lru_linear',
        [_test_src, _lib_src],
        include_directories : _incs,
        c_args              : '-DLRU_CACHE_LOOKUP_STRATEGY=LRU_CACHE_LOOKUP_LINEAR',
)

test('lru_cache / linear strategy', _test_linear,
        suite   : 'unit',
        timeout : 30,
)

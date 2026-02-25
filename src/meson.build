# ---------------------------------------------------------------------------
# Static library
# ---------------------------------------------------------------------------

lru_cache_lib = static_library(
        'lru_cache',
        'lru_cache.c',
        include_directories : [public_inc, generated_inc],
        c_args              : lru_strategy_arg,
        install             : true,
)

# Dependency object for use by subprojects / wrap consumers.
lru_cache_dep = declare_dependency(
        link_with           : lru_cache_lib,
        include_directories : public_inc,
)

project(
        'lru-cache',
        'c',
        version         : '1.0.0',
        license         : 'MIT',
        default_options : [
                'c_std=c11',
                'warning_level=2',
        ],
        meson_version : '>=0.57.0',
)

# ---------------------------------------------------------------------------
# Version header (generated into the build directory root)
# ---------------------------------------------------------------------------

_ver = meson.project_version().split('.')

_conf = configuration_data()
_conf.set_quoted('LRU_CACHE_VERSION',       meson.project_version())
_conf.set(       'LRU_CACHE_VERSION_MAJOR', _ver[0].to_int())
_conf.set(       'LRU_CACHE_VERSION_MINOR', _ver[1].to_int())
_conf.set(       'LRU_CACHE_VERSION_PATCH', _ver[2].to_int())

configure_file(
        input         : 'include' / 'lru_cache_version.h.in',
        output        : 'lru_cache_version.h',
        configuration : _conf,
        install       : true,
        install_dir   : get_option('includedir') / 'lru_cache',
)

install_headers('include' / 'lru_cache.h', subdir : 'lru_cache')

# ---------------------------------------------------------------------------
# Include directories
#   public_inc    -- the source include/ tree (lru_cache.h)
#   generated_inc -- the build root (lru_cache_version.h lands here)
# ---------------------------------------------------------------------------

public_inc    = include_directories('include')
generated_inc = include_directories('.')

# ---------------------------------------------------------------------------
# Compile-time lookup strategy for the installed library
# ---------------------------------------------------------------------------

if get_option('lookup_strategy') == 'linear'
        lru_strategy_arg = '-DLRU_CACHE_LOOKUP_STRATEGY=LRU_CACHE_LOOKUP_LINEAR'
else
        lru_strategy_arg = '-DLRU_CACHE_LOOKUP_STRATEGY=LRU_CACHE_LOOKUP_HASH'
endif

# ---------------------------------------------------------------------------
# Sub-directories
# ---------------------------------------------------------------------------

subdir('src')

if get_option('build_tests')
        subdir('tests')
endif

# AGENTS.md

---

## 1) Project-specific instructions

**Project:** `lru-cache`
**Primary goal:** A static-memory, deterministic Least Recently Used (LRU)
cache in C for safety-critical embedded systems.

### 1.1 Essential commands

#### Configure and build (library only)

```sh
meson setup build --wipe --buildtype=release -Dbuild_tests=false
meson compile -C build
```

#### Configure, build, and run unit tests

```sh
meson setup build --wipe --buildtype=debug -Dbuild_tests=true
meson compile -C build
meson test -C build --verbose
```

`meson test` runs two executables — one compiled with `HASH` strategy and one
with `LINEAR` — so both code paths are exercised in a single invocation.

#### Select lookup strategy (library build)

```sh
# Hash table (default)
meson setup build -Dlookup_strategy=hash

# Linear scan
meson setup build -Dlookup_strategy=linear
```

#### Notes

- `meson setup` generates `lru_cache_version.h` into the **build directory**
  (not the source tree) via `configure_file`.
- The public header lives at `include/lru_cache.h`; the shim at
  `src/lru_cache.h` was removed. Do not recreate it.
- Header install path: `<prefix>/include/lru_cache/lru_cache.h`.

---

## 2) CI / source of truth

- CI definitions live in `.github/workflows/ci.yml`.
- Prefer running the same commands locally as CI runs (see §1.1 above).
- If `pre-commit` is configured, run `pre-commit run --all-files` before
  committing.

---

## 3) Docs / commit conventions

- Use **Conventional Commits** format when asked to commit.
- Keep commits focused; explain *why* in the message body.

---

## 4) C style expectations

### Build & configuration

- Use the Meson build system. Do not introduce CMake, Make, or other systems.
- Update `src/meson.build` when adding or removing source files.

### Formatting

- `.clang-format` is present and **mandatory**. Run `clang-format -i` on all
  modified `.c` / `.h` files before committing.
- Do not reformat unrelated code.
- Key settings: 8-space indent, `BreakBeforeBraces: Linux`, column limit 80.

### Style & correctness

- Match conventions in the existing files (indentation, braces, naming).
- Validate pointer arguments at every public API boundary.
- No heap allocation (`malloc` / `free` / VLAs).
- Use `uint32_t`, `uint16_t`, `int16_t`, `bool` from `<stdint.h>` /
  `<stdbool.h>` — never plain `int` for fixed-width fields.

### Error handling

- Public functions return `bool` or validate via early `return`.
- No `errno`; no exceptions.

### Testing

- Run `meson test -C build` after every change.
- Add a test case for each bug fix.
- Tests live in `tests/test_lru_cache.c`; both strategy executables must pass.

---

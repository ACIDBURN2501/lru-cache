# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [Unreleased]

### Changed

- Added changelog, contributor guide, security policy, SPDX headers, cleaned ignore rules, sanitiser-enabled CI, and a standard coverage gate.

## [1.1.6] - 2026-03-21

### Fixed

- Guarded the hash lookup test helper with the lookup strategy define to avoid unused-function warnings in non-hash builds.

## [1.0.0] - 2026-02-25

### Added

- Static-memory deterministic LRU cache with pointer-free index lists, configurable hash or linear lookup, fixed capacity, and bounded execution time.
- Workflow status badge to README.

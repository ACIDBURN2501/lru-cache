# Security Policy

## Reporting a Vulnerability

Use GitHub's Security Advisories feature to report security concerns
privately.

Expect a response within 7 days. If the issue is confirmed, a fix
will be released as a patch version.

## Scope

lru-cache is a small cache library with no network stack, no external
dependencies, and no dynamic memory allocation. The primary attack
surface is integer overflow in index/hash calculations and out-of-bounds
access in the entry array.

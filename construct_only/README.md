# ZZT construction-only baseline

This directory is a copy of the old ZZT `CC` construction path trimmed at the
point where the ZigZag array (`indices`) and ZigZag LCP array (`LCP`) have been
completed. It does not construct the CC trie and does not execute queries.

Build:

```bash
make
```

Run:

```bash
/usr/bin/time -v ./zzt_construct_only -f TEXT
```

Reported comparable metrics are `Total construction time`, `Index size`, and
`Index memory`. GNU time's `Maximum resident set size (kbytes)` is the
construction-space measurement because the process exits immediately after
ZZA and ZZ-LCP construction.

This target intentionally omits pattern loading, compact-trie construction,
and all query code. It retains the old implementation's 64-bit `INT` type and
its original ZZA/ZZ-LCP construction algorithm.

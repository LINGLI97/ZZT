ZigZag array / ZZ-LCP construction
==================================
zigzag.cpp     32-bit positions when n < 2^32 (16-byte records), 64-bit otherwise
zigzag64.cpp   dedicated 64-bit build for very large texts
libsais/       libsais 2.10.4 (Ilya Grebnov, Apache-2.0), used by the suffix-array fallback oracle

Build:   make                 (OpenMP build: parallel libsais routines)
         make OMP=            (single-threaded)
Tests:   make tests && ./zigzag64_test units|enum2|enum3|enumX|family|random|cross   (same for ./zigzag_test)
Usage:   ./zigzag64 <textfile> [-o <outfile>] [--bin] [--check] [--quiet] [--raw]
         ZZ_VERBOSE=1 prints per-round statistics and timings.

Reported construction metrics
-----------------------------
Total construction time: wall-clock time measured in the program after input reading,
including alphabet remapping and construction of ZZA and ZZ-LCP.
Index size: logical size of the completed ZZA and ZZ-LCP arrays (not peak memory).
Index memory: the same value under the legacy label used by existing result parsers.
Construction space: run through GNU time and use "Maximum resident set size (kbytes)":
  /usr/bin/time -v ./zigzag <textfile> --quiet
  /usr/bin/time -v ./zigzag64 <textfile> --quiet

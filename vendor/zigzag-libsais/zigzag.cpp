// zigzag.cpp -- construction of the ZigZag array (ZZA) and ZigZag LCP array (ZZ-LCP)
// of a text T by chunked LSD radix sorting of the ZigZag strings.
//
// This version uses 32-bit positions/ranks/counters when n < 2^32 (16-byte records, about
// 45 bytes of peak memory per letter) and 64-bit ones otherwise; see zigzag64.cpp for the
// version dedicated to very large texts.  The two files share the same core.
//
//   Z_i = T[i] T[i+1] T[i-1] T[i+2] T[i-2] ...   (0-based i), terminating as soon as an
//   index falls outside [0, n).   |Z_i| = min(2(n-1-i)+1, 2i+2)  (pairwise distinct).
//
// The t-th chunk of Z_i (2k letters) interleaves two contiguous windows of length k:
//   A = T^R[(n-1-i) + (t-1)k ..)   (odd positions:  T[i], T[i-1], ...)
//   B = T  [ i+1     + (t-1)k ..)   (even positions: T[i+1], T[i+2], ...)
// so a chunk key is two window reads from the spread-packed texts, a shift and an OR.  No sentinel
// symbol is appended anywhere: since all |Z_i| are distinct, a chunk that ends before its 2k
// letters is ordered by its valid length, which is a closed-form function of i.
// Round t sorts the still-unresolved elements by (block id, chunk key) with a stable
// LSD radix sort; truncated strings are ordered by their valid length; ZZ-LCP values
// are induced at the moment two elements are separated.  After `max_rounds` rounds
// the remaining groups (strings sharing a very long prefix) are finished with the
// O(1)-LCE comparator of the paper (LCE via Karp-Rabin fingerprints here).
//
// Build:  see Makefile (libsais/libsais.c and libsais/libsais64.c are compiled as C and linked in), e.g.
//         gcc -O3 -c libsais/libsais.c libsais/libsais64.c && g++ -O3 -march=native -std=c++17 -o zigzag zigzag.cpp libsais.o libsais64.o
//         (add -fopenmp -DLIBSAIS_OPENMP to both commands for the parallel libsais routines)
// Usage:  ./zigzag <textfile> [-o <outfile>] [--bin] [--check] [--quiet] [--raw]
//   Default output: "p ZZA[p] ZZLCP[p]" per line (0-based text positions) to stdout or <outfile>.
//   --bin   writes <outfile>.zza and <outfile>.zzlcp as raw little-endian arrays of uint32
//           (n < 2^32) or uint64 (n entries each; <outfile> defaults to "out").
//   --check verifies against a brute-force construction (small inputs only).
//   --quiet suppresses the output (timing only).
//   --raw   keeps trailing newline bytes of the file as letters (default: strip them).

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <type_traits>
#include <climits>
#include "libsais/libsais.h"
#include "libsais/libsais64.h"
#ifdef _OPENMP
#include <omp.h>
#endif

namespace zz {

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint8_t u8;

struct Params {
    int max_k = 0;        // 0 = automatic (floor(64/(2b)) letters per window); smaller values are for testing
    int max_rounds = 4;   // radix rounds before switching to the comparison-sort fallback
    int fallback_lce = 0; // 0 = automatic (Karp-Rabin unless at least n/32 elements survive),
                          // 1 = Karp-Rabin fingerprints (O(n) build, O(log n) query),
                          // 2 = suffix array + LCP (libsais) + RMQ (O(n) build, O(1) query)
    int digit_bits = 16;  // LSD digit width for small/medium inputs
    size_t msd_threshold = size_t(1) << 20;  // above this many records: MSD split into cache-sized buckets, LSD inside
    // Negative: ordinary finite-string order (a proper prefix sorts first).
    // Non-negative: use this virtual letter after a ZigZag string ends.  This
    // matches the original ZZT implementation, whose out-of-range byte is 255.
    int end_symbol = -1;
};

// Statistics of a construction, filled in by build_zza_zzlcp().
struct Stats {
    size_t n = 0;          // text length (letters)
    size_t sigma = 0;      // number of distinct letters in the text
    int bits = 0;          // bits per letter used internally
    int letters_per_chunk = 0;  // 2k: letters of a ZigZag string consumed per round
    size_t rounds = 0;         // radix rounds performed
    size_t unresolved_after = 0;  // elements still tied after the last radix round (0 if none)
    bool used_fallback = false;   // whether the comparison-sort fallback finished the job
    int fallback_mode = 0;        // 1 = Karp-Rabin fingerprints, 2 = suffix array + LCP + RMQ
};

// ---------------------------------------------------------------------------
// Spread-packed text: letter j of the given sequence occupies the HIGH b bits of the
// j-th 2b-bit field (LSB-first), i.e. the letters are stored "already spread"; the low
// half of every field is free for the letters of the other window (OR-ed in after >> b).
// A forward window of the REVERSED sequence is then a contiguous bit range whose top
// field holds the window's first letter, which is exactly the layout of a chunk key:
//   A (T[i], T[i-1], ...) is read from spread(T),  B (T[i+1], T[i+2], ...) from spread(T^R).
// No sentinels: letters that would lie before position 0 read as 0 by clamping the
// range (they are masked away by the valid length anyway); two slack words allow a
// two-word read at the top end.
// ---------------------------------------------------------------------------
struct SpreadText {
    int b = 0; size_t n = 0;
    std::vector<u64> words;

    void build(const std::vector<u8>& letters, int bits, bool rev) {
        b = bits; n = letters.size();
        words.assign((2 * n * b + 63) / 64 + 2, 0);
        for (size_t j = 0; j < n; ++j) {
            u64 v = rev ? letters[n - 1 - j] : letters[j];
            size_t bit = 2 * j * b + b;
            words[bit >> 6] |= v << (bit & 63);
            if ((bit & 63) + b > 64) words[(bit >> 6) + 1] |= v >> (64 - (bit & 63));
        }
    }
    // bits [bit, bit + nb) of the array, nb <= 64, bit + nb <= 64 * (words.size() - 1)
    inline u64 bits(size_t bit, int nb) const {
        size_t w = bit >> 6, sft = bit & 63;
        u64 v = words[w] >> sft;
        if (sft) v |= words[w + 1] << (64 - sft);
        return nb >= 64 ? v : (v & ((1ULL << nb) - 1));
    }
    // Spread word of the k letters of the reversed sequence starting at reversed position q
    // (q in [0, n)), first letter in the top field.  Letters beyond the end of the reversed
    // sequence (i.e. before position 0 of this one) read as 0.
    inline u64 rev_window(size_t q, int k) const {
        long long s = (long long)n - (long long)q - k;      // first (lowest) letter of this sequence in the range
        if (s >= 0) return bits((size_t)s * 2 * b, 2 * k * b);
        int avail = k + (int)s;                              // letters actually present: [0, avail)
        if (avail <= 0) return 0;
        return bits(0, 2 * avail * b) << (2 * (k - avail) * b);
    }
};

// ---------------------------------------------------------------------------
// LCE by Karp-Rabin fingerprints (mod 2^61-1) with exponential + binary search.
// Only used by the fallback for groups whose strings share a very long prefix.
// A single prefix-fingerprint array of T answers both directions: substring equality
// does not depend on orientation, so the longest common suffix of T[0..i] and T[0..j]
// is searched with the same fingerprints (no reversed copy of the text, no second
// array, one construction pass).
// ---------------------------------------------------------------------------
struct HashLCE {
    static const u64 MOD = (1ULL << 61) - 1;
    std::vector<u64> H, PW;
    static inline u64 mulmod(u64 a, u64 b) {
        unsigned __int128 p = (unsigned __int128)a * b;
        u64 r = (u64)(p & MOD) + (u64)(p >> 61);
        return r >= MOD ? r - MOD : r;
    }
    void build(const std::vector<u8>& s, u64 base) {
        size_t n = s.size();
        H.assign(n + 1, 0); PW.assign(n + 1, 1);
        for (size_t j = 0; j < n; ++j) {
            H[j + 1] = (mulmod(H[j], base) + s[j] + 1) % MOD;
            PW[j + 1] = mulmod(PW[j], base);
        }
    }
    inline u64 sub(size_t i, size_t len) const {                 // fingerprint of T[i..i+len)
        u64 v = H[i + len] + MOD - mulmod(H[i], PW[len]);
        return v >= MOD ? v - MOD : v;
    }
    // Generic search: largest L in [known, maxlen] with eq(L) true, given eq(known) true and eq monotone.
    template <class Eq>
    static inline u64 search(size_t known, size_t maxlen, Eq eq) {
        if (known >= maxlen) return maxlen;
        size_t lo = known, hi = known + 1, step = 1;
        while (hi <= maxlen && eq(hi)) { lo = hi; step <<= 1; hi = lo + step; }
        hi = std::min(hi, maxlen + 1);
        while (hi - lo > 1) { size_t mid = lo + (hi - lo) / 2; if (eq(mid)) lo = mid; else hi = mid; }
        return lo;
    }
    // longest common prefix of the suffixes at i != j, knowing `known` letters agree, capped at `upto`
    u64 lce(size_t i, size_t j, size_t known, size_t upto) const {
        size_t n = H.size() - 1;
        size_t maxlen = std::min(upto, n - std::max(i, j));
        return search(known, maxlen, [&](size_t L) { return sub(i, L) == sub(j, L); });
    }
    // longest common suffix of the prefixes T[0..i] and T[0..j], i != j (an LCE on T^R), same fingerprints
    u64 lcs(size_t i, size_t j, size_t known, size_t upto) const {
        size_t maxlen = std::min(upto, std::min(i, j) + 1);
        return search(known, maxlen, [&](size_t L) { return sub(i + 1 - L, L) == sub(j + 1 - L, L); });
    }
};

// ---------------------------------------------------------------------------
// Range-minimum queries in O(1) time after O(n) preprocessing.
// Level 0: for every position a 32-bit mask of the "candidate minima" stack inside
//          its block of 32 entries (in-block query = one mask-and and one ctz);
// Level 1: the same masks over the array of block minima;
// Level 2: a sparse table over the minima of superblocks (1024 entries).
// Space: 4 bytes per entry plus o(n).  (Same guarantees as the succinct RMQ of
// sdsl-lite; plain arrays instead of a balanced-parentheses sequence.)
// ---------------------------------------------------------------------------
template <typename VT>
struct RMQ {
    const VT* a = nullptr; size_t n = 0;
    std::vector<u32> m0;   // masks over a
    std::vector<VT> b1;    // block minima of a          (nb  = ceil(n/32))
    std::vector<u32> m1;   // masks over b1
    std::vector<VT> b2;    // superblock minima           (ns  = ceil(nb/32))
    std::vector<VT> st;    // sparse table over b2, level-major
    size_t nb = 0, ns = 0, levels = 0;

    static void masks(const VT* v, size_t len, std::vector<u32>& m, std::vector<VT>& bmin) {
        size_t blocks = (len + 31) / 32;
        m.assign(len, 0); bmin.assign(blocks, VT(0));
        for (size_t bb = 0; bb < blocks; ++bb) {
            size_t s0 = bb * 32, e0 = std::min(len, s0 + 32);
            u32 stack = 0;                                    // bit j set: index s0+j is on the stack
            VT mn = v[s0];
            for (size_t i = s0; i < e0; ++i) {
                while (stack) {                               // pop while top value > v[i]
                    int top = 31 - __builtin_clz(stack);
                    if (v[s0 + top] > v[i]) stack &= ~(1u << top); else break;
                }
                stack |= 1u << (i - s0);
                m[i] = stack;
                mn = std::min(mn, v[i]);
            }
            bmin[bb] = mn;
        }
    }
    static inline size_t inblock(const u32* m, size_t l, size_t r) {   // position of the min of v[l..r], same block
        u32 x = m[r] & (~0u << (l & 31));
        return (r & ~size_t(31)) + (size_t)__builtin_ctz(x);
    }
    void build(const VT* values, size_t len) {
        a = values; n = len;
        if (n == 0) return;
        masks(a, n, m0, b1);
        nb = b1.size();
        masks(b1.data(), nb, m1, b2);
        ns = b2.size();
        levels = 1; while ((size_t(1) << levels) <= ns) ++levels;
        st.assign(levels * ns, VT(0));
        for (size_t i = 0; i < ns; ++i) st[i] = b2[i];
        for (size_t l = 1; l < levels; ++l)
            for (size_t i = 0; i + (size_t(1) << l) <= ns; ++i)
                st[l * ns + i] = std::min(st[(l - 1) * ns + i], st[(l - 1) * ns + i + (size_t(1) << (l - 1))]);
    }
    // min of b1[l..r] (inclusive)
    inline VT query1(size_t l, size_t r) const {
        size_t bl = l >> 5, br = r >> 5;
        if (bl == br) return b1[inblock(m1.data(), l, r)];
        VT res = std::min(b1[inblock(m1.data(), l, bl * 32 + 31)], b1[inblock(m1.data(), br * 32, r)]);
        if (bl + 1 <= br - 1) {
            size_t x = bl + 1, y = br - 1, len = y - x + 1, lv = 0;
            while ((size_t(2) << lv) <= len) ++lv;
            res = std::min(res, std::min(st[lv * ns + x], st[lv * ns + y + 1 - (size_t(1) << lv)]));
        }
        return res;
    }
    // min of a[l..r] (inclusive), l <= r
    inline VT query(size_t l, size_t r) const {
        size_t bl = l >> 5, br = r >> 5;
        if (bl == br) return a[inblock(m0.data(), l, r)];
        VT res = std::min(a[inblock(m0.data(), l, bl * 32 + 31)], a[inblock(m0.data(), br * 32, r)]);
        if (bl + 1 <= br - 1) res = std::min(res, query1(bl + 1, br - 1));
        return res;
    }
};

// ---------------------------------------------------------------------------
// LCE by suffix array + LCP array (libsais, https://github.com/IlyaGrebnov/libsais:
// linear time, optionally parallel) and the O(1) RMQ above.  Ranks and LCP values are
// stored in 32 bits whenever n < 2^32.  lce(i, j) = min LCP[isa(i)+1 .. isa(j)].
// ---------------------------------------------------------------------------
template <typename VT>
struct SALCEImpl {
    std::vector<VT> isa, lcp;     // lcp[p] = LCP of the suffixes of rank p-1 and p (lcp[0] = 0)
    RMQ<VT> rmq;
    size_t n = 0;

    template <typename SAT, typename F_SA, typename F_PLCP, typename F_LCP>
    void build_with(const std::vector<u8>& s, F_SA f_sa, F_PLCP f_plcp, F_LCP f_lcp) {
        n = s.size();
        std::vector<SAT> SA(n);
        if (f_sa(s.data(), SA.data(), (SAT)n) != 0) { fprintf(stderr, "libsais failed\n"); abort(); }
        isa.assign(n, 0);
        for (size_t p = 0; p < n; ++p) isa[(size_t)SA[p]] = (VT)p;
        std::vector<SAT> PLCP(n);
        if (f_plcp(s.data(), SA.data(), PLCP.data(), (SAT)n) != 0) { fprintf(stderr, "libsais_plcp failed\n"); abort(); }
        if (f_lcp(PLCP.data(), SA.data(), SA.data(), (SAT)n) != 0) { fprintf(stderr, "libsais_lcp failed\n"); abort(); }   // LCP in place of SA
        std::vector<SAT>().swap(PLCP);
        lcp.assign(n, 0);
        for (size_t p = 0; p < n; ++p) lcp[p] = (VT)SA[p];
        std::vector<SAT>().swap(SA);
        rmq.build(lcp.data(), n);
    }
    void build(const std::vector<u8>& s) {
        if (s.size() <= (size_t)INT32_MAX) {
#if defined(_OPENMP) && defined(LIBSAIS_OPENMP)
            build_with<int32_t>(s,
                [](const uint8_t* T, int32_t* SA, int32_t nn) { return libsais_omp(T, SA, nn, 0, nullptr, 0); },
                [](const uint8_t* T, const int32_t* SA, int32_t* P, int32_t nn) { return libsais_plcp_omp(T, SA, P, nn, 0); },
                [](const int32_t* P, const int32_t* SA, int32_t* L, int32_t nn) { return libsais_lcp_omp(P, SA, L, nn, 0); });
#else
            build_with<int32_t>(s,
                [](const uint8_t* T, int32_t* SA, int32_t nn) { return libsais(T, SA, nn, 0, nullptr); },
                [](const uint8_t* T, const int32_t* SA, int32_t* P, int32_t nn) { return libsais_plcp(T, SA, P, nn); },
                [](const int32_t* P, const int32_t* SA, int32_t* L, int32_t nn) { return libsais_lcp(P, SA, L, nn); });
#endif
        } else {
#if defined(_OPENMP) && defined(LIBSAIS_OPENMP)
            build_with<int64_t>(s,
                [](const uint8_t* T, int64_t* SA, int64_t nn) { return libsais64_omp(T, SA, nn, 0, nullptr, 0); },
                [](const uint8_t* T, const int64_t* SA, int64_t* P, int64_t nn) { return libsais64_plcp_omp(T, SA, P, nn, 0); },
                [](const int64_t* P, const int64_t* SA, int64_t* L, int64_t nn) { return libsais64_lcp_omp(P, SA, L, nn, 0); });
#else
            build_with<int64_t>(s,
                [](const uint8_t* T, int64_t* SA, int64_t nn) { return libsais64(T, SA, nn, 0, nullptr); },
                [](const uint8_t* T, const int64_t* SA, int64_t* P, int64_t nn) { return libsais64_plcp(T, SA, P, nn); },
                [](const int64_t* P, const int64_t* SA, int64_t* L, int64_t nn) { return libsais64_lcp(P, SA, L, nn); });
#endif
        }
    }
    inline u64 lce(size_t i, size_t j) const {                  // i != j
        size_t a = isa[i], c = isa[j];
        if (a > c) std::swap(a, c);
        return rmq.query(a + 1, c);
    }
};

template <typename IDX>
struct SALCE {
    SALCEImpl<u32> i32; SALCEImpl<u64> i64; bool use32 = true;
    void build(const std::vector<u8>& s) {
        use32 = s.size() < (size_t(1) << 32);
        if (use32) i32.build(s); else i64.build(s);
    }
    inline u64 lce(size_t i, size_t j) const { return use32 ? i32.lce(i, j) : i64.lce(i, j); }
};

// ---------------------------------------------------------------------------
// The construction.  IDX must hold n (uint32_t if n < 2^32).
// ---------------------------------------------------------------------------
template <typename IDX>
struct Builder {
    struct Rec { u64 key; IDX id; IDX bid; };
    typedef typename std::conditional<sizeof(IDX) == 4, u32, size_t>::type CNT;   // histogram / offset counters

    const std::vector<u8>& letters;
    const IDX n;
    const int b, k, keybits;
    const Params prm;
    Stats* st = nullptr;    // optional, filled in by run()
    SpreadText ST, SR;      // spread(T) and spread(T^R)

    Builder(const std::vector<u8>& L, int bits, const Params& p)
        : letters(L), n((IDX)L.size()), b(bits),
          k(p.max_k > 0 ? std::min(p.max_k, 64 / (2 * bits)) : 64 / (2 * bits)),
          keybits(2 * k * bits), prm(p) {
        ST.build(letters, b, false);
        SR.build(letters, b, true);
    }

    inline u64 zlen(u64 i) const { return std::min<u64>(2 * ((u64)n - 1 - i) + 1, 2 * i + 2); }
    inline u32 chunk_len(u64 i, u64 t) const {
        u64 consumed = (t - 1) * 2 * (u64)k, zl = zlen(i);
        return zl > consumed ? (u32)std::min<u64>(zl - consumed, 2 * (u64)k) : 0;
    }
    inline u64 chunk_key(u64 i, u64 t, u32 L) const {
        if (L == 0 && prm.end_symbol < 0) return 0;
        // L > 0 implies both window starts are within range (see the formal description)
        size_t off = (size_t)(t - 1) * k;
        u64 K = 0;
        if (L > 0) {
            u64 A = ST.rev_window((size_t)((u64)n - 1 - i) + off, k);   // T^R[n-1-i+off ..): T[i-off], T[i-off-1], ...
            u64 B = (i + 1 + off < (u64)n) ? SR.rev_window((size_t)i + 1 + off, k) : 0;   // T[i+1+off ..)
            K = (A | (B >> b)) << (64 - keybits);    // top-align the 2kb key bits
        }
        int used = (int)L * b;
        K = used == 0 ? 0 : (used < 64 ? (K & (~0ULL << (64 - used))) : K);
        if (prm.end_symbol >= 0) {
            for (u32 q = L; q < 2 * (u32)k; ++q) {
                K |= (u64)prm.end_symbol << (64 - (q + 1) * b);
            }
        }
        return K;
    }

    // Digit j (least significant first) of the composite sort key (bid, key), with D-bit digits:
    // key digits cover the top `keybits` bits of key, bid digits follow.
    struct DigitPlan {
        int D, kd, bd, nd, keybits;
        inline u32 get(const Rec& r, int j) const {
            const u64 DM = (1ULL << D) - 1;
            if (j < kd) return (u32)((r.key >> (64 - keybits + D * j)) & DM);
            return (u32)(((u64)r.bid >> (D * (j - kd))) & DM);
        }
    };
    DigitPlan plan(int D, size_t nblocks) const {
        DigitPlan P; P.D = D; P.keybits = keybits; P.kd = (keybits + D - 1) / D; P.bd = 0;
        if (nblocks > 1) { u64 x = nblocks - 1; while (x) { ++P.bd; x >>= D; } }
        P.nd = P.kd + P.bd;
        return P;
    }

    // Stable LSD radix sort of src[0..m) by digits [0, ndig) of `P`, using dst as scratch;
    // digits that are constant over the range are skipped.  Returns the buffer holding the result.
    Rec* lsd_sort(Rec* src, Rec* dst, size_t m, const DigitPlan& P, int ndig, std::vector<CNT>& hist) const {
        const size_t NB = size_t(1) << P.D;
        hist.assign((size_t)ndig * NB, 0);
        for (size_t p = 0; p < m; ++p)
            for (int j = 0; j < ndig; ++j) ++hist[(size_t)j * NB + P.get(src[p], j)];
        for (int j = 0; j < ndig; ++j) {
            CNT* h = &hist[(size_t)j * NB];
            bool trivial = false;
            for (size_t v = 0; v < NB; ++v) if (h[v] == m) { trivial = true; break; }
            if (trivial) continue;
            CNT sum = 0;
            for (size_t v = 0; v < NB; ++v) { CNT c = h[v]; h[v] = sum; sum += c; }
            for (size_t p = 0; p < m; ++p) { const Rec& r = src[p]; dst[h[P.get(r, j)]++] = r; }
            std::swap(src, dst);
        }
        return src;
    }

    // Sort rec[0..m) by (bid, key).  Small inputs: plain LSD with D-bit digits.  Large inputs:
    // one MSD scatter on the most significant non-constant digit into buckets that fit in cache,
    // then an LSD sort with 8-bit digits inside every bucket (or std::sort for tiny buckets).
    void radix_sort(std::vector<Rec>& rec, std::vector<Rec>& tmp, size_t m, size_t nblocks) {
        const int D = prm.digit_bits;
        std::vector<CNT> hist;
        if (m < prm.msd_threshold) {
            DigitPlan P = plan(D, nblocks);
            Rec* res = lsd_sort(rec.data(), tmp.data(), m, P, P.nd, hist);
            if (res != rec.data()) std::copy(res, res + m, rec.data());
            return;
        }
        // --- MSD split on the top DM bits of the composite key (bid bits, then key bits), DM chosen
        //     so that a bucket holds about 1 MB of records. ---
        int DM = 8;
        while (DM < 16 && (m >> DM) * sizeof(Rec) > (size_t(1) << 20)) ++DM;
        int bidbits = 0;
        if (nblocks > 1) { u64 x = nblocks - 1; while (x) { ++bidbits; x >>= 1; } }
        auto msd_digit = [&](const Rec& r) -> u32 {
            if (bidbits >= DM) return (u32)((u64)r.bid >> (bidbits - DM));
            int from_key = DM - bidbits;                 // top bits of the key complete the digit
            return (u32)(((u64)r.bid << from_key) | (r.key >> (64 - from_key)));
        };
        const size_t NB = size_t(1) << DM;
        std::vector<CNT> start(NB + 1, 0);
        for (size_t p = 0; p < m; ++p) ++start[msd_digit(rec[p]) + 1];
        for (size_t v = 0; v < NB; ++v) start[v + 1] += start[v];
        std::vector<CNT> pos(start.begin(), start.begin() + NB);
        for (size_t p = 0; p < m; ++p) { const Rec& r = rec[p]; tmp[pos[msd_digit(r)]++] = r; }
        // --- inside each bucket: LSD with DS-bit digits over the whole composite key; digits that
        //     are constant within the bucket (in particular those inside the MSD digit) are skipped. ---
        const int DS = 11;
        DigitPlan PS = plan(DS, nblocks);
        auto cmp = [](const Rec& a, const Rec& c) { return a.bid != c.bid ? a.bid < c.bid : a.key < c.key; };
        std::vector<CNT> hist_b;
        for (size_t v = 0; v < NB; ++v) {
            size_t s0 = start[v], e0 = start[v + 1], len = e0 - s0;
            if (len < 2) { if (len == 1) rec[s0] = tmp[s0]; continue; }
            if (len <= 64) {
                std::sort(tmp.begin() + s0, tmp.begin() + e0, cmp);
                std::copy(tmp.begin() + s0, tmp.begin() + e0, rec.begin() + s0);
                continue;
            }
            Rec* res = lsd_sort(tmp.data() + s0, rec.data() + s0, len, PS, PS.nd, hist_b);
            if (res != rec.data() + s0) std::copy(res, res + len, rec.data() + s0);
        }
    }

    // The comparator of the paper: r = common suffix length of T[0..i] and T[0..j], f = LCE(i+1, j+1);
    // Z_i, Z_j first differ at position 2r+1 if r <= f, else at 2f+2.  Only "f >= r?" or the exact
    // f < r is needed, so the forward query is capped at r.
    template <class Oracle>
    void fallback_sort(Oracle& O, std::vector<Rec>& rec, std::vector<IDX>& slots, size_t m, u64 t,
                       std::vector<IDX>& ZZA, std::vector<IDX>& ZZLCP) {
        const u64 known = (t - 1) * (u64)k;        // letters per side already known to agree within a group
        auto mismatch_pos = [&](u64 i, u64 j) -> u64 {   // 1-based position in Z of the first mismatch
            u64 r = O.backward(i, j, known);
            u64 f = (i + 1 < n && j + 1 < n) ? O.forward(i, j, known, r) : 0;
            return (r <= f) ? 2 * r + 1 : 2 * f + 2;
        };
        auto zletter = [&](u64 i, u64 pos) -> int {
            if (pos > zlen(i)) return prm.end_symbol >= 0 ? prm.end_symbol : -1;
            long long idx = (long long)i + ((pos % 2 == 0) ? 1 : -1) * (long long)(pos / 2);
            return letters[(size_t)idx];
        };
        auto less = [&](const Rec& x, const Rec& y) -> bool {
            u64 pos = mismatch_pos(x.id, y.id);
            return zletter(x.id, pos) < zletter(y.id, pos);
        };
        size_t p = 0;
        while (p < m) {
            size_t q = p;
            while (q + 1 < m && rec[q + 1].bid == rec[p].bid) ++q;
            if (prm.end_symbol >= 0)
                std::stable_sort(rec.begin() + p, rec.begin() + q + 1, less);
            else
                std::sort(rec.begin() + p, rec.begin() + q + 1, less);
            for (size_t r = p; r <= q; ++r) ZZA[slots[r]] = rec[r].id;
            for (size_t r = p + 1; r <= q; ++r) ZZLCP[slots[r]] = (IDX)(mismatch_pos(rec[r - 1].id, rec[r].id) - 1);
            p = q + 1;
        }
    }

    // Karp-Rabin oracle: one fingerprint array of T for both directions, built on first use.
    struct HashOracle {
        const std::vector<u8>* letters; size_t n; HashLCE H; bool built = false;
        void ensure() { if (!built) { H.build(*letters, 0x9E3779B97F4A7C15ULL % HashLCE::MOD); built = true; } }
        u64 backward(u64 i, u64 j, u64 known) { ensure(); return H.lcs(i, j, known, n); }
        u64 forward(u64 i, u64 j, u64 known, u64 cap) { ensure(); return H.lce(i + 1, j + 1, known, cap); }
    };
    // Suffix-array oracle: SA-IS + LCP + RMQ on T and on T^R, each built on first use.
    struct SAOracle {
        const std::vector<u8>* letters; size_t n; SALCE<IDX> SF, SR; bool haveF = false, haveR = false;
        u64 backward(u64 i, u64 j, u64) {
            if (!haveR) { std::vector<u8> rev(letters->rbegin(), letters->rend()); SR.build(rev); haveR = true; }
            return SR.lce(n - 1 - i, n - 1 - j);
        }
        u64 forward(u64 i, u64 j, u64, u64 cap) {
            if (!haveF) { SF.build(*letters); haveF = true; }
            return std::min<u64>(SF.lce(i + 1, j + 1), cap);
        }
    };

    void fallback(std::vector<Rec>& rec, std::vector<IDX>& slots, size_t m, u64 t,
                  std::vector<IDX>& ZZA, std::vector<IDX>& ZZLCP) {
        int mode = prm.fallback_lce;
        if (mode == 0) mode = (m >= (size_t)n / 32) ? 2 : 1;   // many elements left: O(1)-LCE structure pays off
        if (mode == 2) {
            SAOracle O; O.letters = &letters; O.n = n;
            fallback_sort(O, rec, slots, m, t, ZZA, ZZLCP);
        } else {
            HashOracle O; O.letters = &letters; O.n = n;
            fallback_sort(O, rec, slots, m, t, ZZA, ZZLCP);
        }
        if (st) st->fallback_mode = mode;
    }

    void run(std::vector<IDX>& ZZA, std::vector<IDX>& ZZLCP) {
        ZZA.assign(n, 0); ZZLCP.assign(n, 0);
        if (n == 0) return;
        auto t_init = std::chrono::steady_clock::now();
        std::vector<Rec> rec(n), tmp(n);
        std::vector<IDX> slots(n);
        for (IDX i = 0; i < n; ++i) { rec[i].key = 0; rec[i].id = i; rec[i].bid = 0; slots[i] = i; }
        if (getenv("ZZ_VERBOSE")) fprintf(stderr, "  init %.3fs\n", std::chrono::duration<double>(std::chrono::steady_clock::now() - t_init).count());
        size_t m = n, nblocks = 1;
        const u32 full = 2 * (u32)k;

        for (u64 t = 1; m > 0; ++t) {
            auto t_round = std::chrono::steady_clock::now();
            if (getenv("ZZ_VERBOSE")) fprintf(stderr, "  round %llu: %zu unresolved, %zu blocks\n", (unsigned long long)t, m, nblocks);
            if ((int)t > prm.max_rounds) {
                if (st) { st->unresolved_after = m; st->used_fallback = true; }
                fallback(rec, slots, m, t, ZZA, ZZLCP);
                break;
            }
            if (st) st->rounds = (size_t)t;

            // --- chunk keys (sequential over the text in round 1; embarrassingly parallel) ---
#ifdef _OPENMP
#pragma omp parallel for schedule(static)
#endif
            for (long long p = 0; p < (long long)m; ++p) {
                Rec& r = rec[p];
                r.key = chunk_key(r.id, t, chunk_len(r.id, t));
            }

            // --- sort by (bid, key) ---
            auto tk = std::chrono::steady_clock::now();
            radix_sort(rec, tmp, m, nblocks);
            auto ts = std::chrono::steady_clock::now();

            // --- runs of equal (bid, key) that contain a truncated chunk: order by valid length ---
            for (size_t s = 0; s < m;) {
                size_t e = s + 1;
                while (e < m && rec[e].bid == rec[s].bid && rec[e].key == rec[s].key) ++e;
                if (e - s >= 2 && prm.end_symbol < 0) {
                    bool trunc = false;
                    for (size_t q = s; q < e && !trunc; ++q) trunc = chunk_len(rec[q].id, t) < full;
                    if (trunc) {                          // stable counting sort of rec[s..e) by chunk length
                        std::vector<CNT> cnt(full + 2, 0);
                        for (size_t q = s; q < e; ++q) ++cnt[chunk_len(rec[q].id, t) + 1];
                        for (u32 v = 1; v <= full + 1; ++v) cnt[v] += cnt[v - 1];
                        for (size_t q = s; q < e; ++q) tmp[s + cnt[chunk_len(rec[q].id, t)]++] = rec[q];
                        std::copy(tmp.begin() + s, tmp.begin() + e, rec.begin() + s);
                    }
                }
                s = e;
            }

            // --- scatter, LCP induction, and compaction of the unresolved groups into tmp ---
            size_t m2 = 0, nb = 0;
            const u64 base = (t - 1) * 2 * (u64)k;
            size_t s = 0;
            u32 Ls = chunk_len(rec[0].id, t);
            ZZA[slots[0]] = rec[0].id;
            for (size_t p = 1; p <= m; ++p) {
                bool boundary = (p == m);
                u32 Lp = 0;
                if (!boundary) {
                    Lp = chunk_len(rec[p].id, t);
                    const Rec &x = rec[p - 1], &y = rec[p];
                    boundary = !(x.bid == y.bid && x.key == y.key &&
                                 (prm.end_symbol >= 0 || chunk_len(x.id, t) == Lp));
                    ZZA[slots[p]] = y.id;
                    if (boundary && x.bid == y.bid) {
                        u32 Lx = chunk_len(x.id, t);
                        u64 ell = (x.key != y.key) ? (u64)(__builtin_clzll(x.key ^ y.key) / b) : (u64)Lx;
                        ell = std::min<u64>(ell, std::min<u32>(Lx, Lp));
                        ZZLCP[slots[p]] = (IDX)(base + ell);
                    }
                }
                if (boundary) {                            // group rec[s..p) is complete
                    if (p - s >= 2) {
                        for (size_t q = s; q < p; ++q) { tmp[m2] = rec[q]; tmp[m2].bid = (IDX)nb; slots[m2] = slots[q]; ++m2; }
                        ++nb;
                    }
                    s = p; Ls = Lp;
                }
            }
            (void)Ls;
            if (getenv("ZZ_VERBOSE")) {
                auto te = std::chrono::steady_clock::now();
                fprintf(stderr, "    keys %.3fs  sort %.3fs  postfix+scan %.3fs\n",
                        std::chrono::duration<double>(tk - t_round).count(),
                        std::chrono::duration<double>(ts - tk).count(),
                        std::chrono::duration<double>(te - ts).count());
            }
            rec.swap(tmp);
            m = m2; nblocks = nb;
        }
    }
};

// Order-preserving remapping of the bytes of `text` to [0, sigma); returns b = bits per letter
// and, if `sigma_out` is given, the alphabet size.
inline int remap_alphabet(const std::string& text, std::vector<u8>& letters, size_t* sigma_out = nullptr) {
    std::vector<int> map(256, -1);
    for (unsigned char ch : text) map[ch] = 0;
    int sigma = 0;
    for (int ch = 0; ch < 256; ++ch) if (map[ch] == 0) map[ch] = sigma++;
    int b = 1;
    while ((1 << b) < sigma) ++b;
    letters.resize(text.size());
    for (size_t j = 0; j < text.size(); ++j) letters[j] = (u8)map[(unsigned char)text[j]];
    if (sigma_out) *sigma_out = (size_t)sigma;
    return b;
}

// Entry point on remapped letters (values in [0, 2^b)).  Lets the caller free the raw text first.
template <typename IDX>
void build_from_letters(const std::vector<u8>& letters, int b, size_t sigma,
                        std::vector<IDX>& ZZA, std::vector<IDX>& ZZLCP,
                        const Params& prm = Params(), Stats* stats = nullptr) {
    Builder<IDX> B(letters, b, prm);
    Stats local;
    Stats& S = stats ? *stats : local;
    S = Stats();
    S.n = letters.size(); S.sigma = sigma; S.bits = b; S.letters_per_chunk = 2 * B.k;
    B.st = &S;
    B.run(ZZA, ZZLCP);
}

// Public entry point on a text.  IDX = uint32_t is enough for n < 2^32.
template <typename IDX>
void build_zza_zzlcp(const std::string& text, std::vector<IDX>& ZZA, std::vector<IDX>& ZZLCP,
                     const Params& prm = Params(), Stats* stats = nullptr) {
    std::vector<u8> letters;
    size_t sigma = 0;
    int b = remap_alphabet(text, letters, &sigma);
    build_from_letters<IDX>(letters, b, sigma, ZZA, ZZLCP, prm, stats);
}

// Brute force reference (explicit ZigZag strings), for testing.
template <typename IDX>
bool brute_check(const std::string& text, const std::vector<IDX>& ZZA, const std::vector<IDX>& ZZLCP,
                 std::string* why = nullptr) {
    size_t n = text.size();
    if (ZZA.size() != n || ZZLCP.size() != n) { if (why) *why = "size"; return false; }
    std::vector<std::string> Z(n);
    for (size_t i = 0; i < n; ++i) {
        std::string z;
        for (u64 kk = 1;; ++kk) {
            long long idx = (long long)i + ((kk % 2 == 0) ? 1 : -1) * (long long)(kk / 2);
            if (idx < 0 || idx >= (long long)n) break;
            z.push_back(text[idx]);
        }
        Z[i] = z;
    }
    std::vector<size_t> ord(n);
    for (size_t i = 0; i < n; ++i) ord[i] = i;
    std::sort(ord.begin(), ord.end(), [&](size_t a, size_t c) { return Z[a] < Z[c]; });
    for (size_t p = 0; p < n; ++p) {
        if (ord[p] != (size_t)ZZA[p]) { if (why) *why = "ZZA at " + std::to_string(p); return false; }
        size_t lcp = 0;
        if (p > 0) {
            const std::string &x = Z[ord[p - 1]], &y = Z[ord[p]];
            while (lcp < x.size() && lcp < y.size() && x[lcp] == y[lcp]) ++lcp;
        }
        if (lcp != (size_t)ZZLCP[p]) { if (why) *why = "ZZLCP at " + std::to_string(p); return false; }
    }
    return true;
}

}  // namespace zz

#ifndef ZZ_NO_MAIN
// Read a whole file in 64 MB blocks (no 2 GB limitations, no byte-wise iterators).
static bool read_file(const char* path, std::string& out) {
    FILE* f = fopen(path, "rb");
    if (!f) return false;
    if (fseeko(f, 0, SEEK_END) != 0) { fclose(f); return false; }
    off_t sz = ftello(f);
    if (sz < 0) { fclose(f); return false; }
    fseeko(f, 0, SEEK_SET);
    out.resize((size_t)sz);
    const size_t BLOCK = size_t(64) << 20;
    size_t done = 0;
    while (done < (size_t)sz) {
        size_t want = std::min(BLOCK, (size_t)sz - done);
        size_t got = fread(&out[done], 1, want, f);
        if (got == 0) break;
        done += got;
    }
    fclose(f);
    if (done != (size_t)sz) { out.resize(done); }
    return true;
}

template <typename IDX>
static bool write_array(const std::string& path, const std::vector<IDX>& v) {
    FILE* f = fopen(path.c_str(), "wb");
    if (!f) return false;
    const size_t BLOCK = size_t(8) << 20;
    for (size_t p = 0; p < v.size(); p += BLOCK) {
        size_t cnt = std::min(BLOCK, v.size() - p);
        if (fwrite(&v[p], sizeof(IDX), cnt, f) != cnt) { fclose(f); return false; }
    }
    return fclose(f) == 0;
}

template <typename IDX>
static int run_main(std::vector<zz::u8>& letters, int b, size_t sigma, const std::string& text_copy,
                    const std::string& outfile, bool check, bool quiet, bool bin,
                    std::chrono::steady_clock::time_point construction_start) {
    const size_t n = letters.size();
    std::vector<IDX> ZZA, ZZLCP;
    zz::Stats st;
    zz::build_from_letters<IDX>(letters, b, sigma, ZZA, ZZLCP, zz::Params(), &st);
    double secs = std::chrono::duration<double>(std::chrono::steady_clock::now() - construction_start).count();
    const size_t index_bytes = ZZA.size() * sizeof(IDX) + ZZLCP.size() * sizeof(IDX);
    const double index_mb = (double)index_bytes / (1024.0 * 1024.0);
    fprintf(stderr, "n = %zu, sigma = %zu, rounds = %zu%s (%d letters of each ZigZag string per round)"
                    ", construction time %.3f s\n",
            st.n, st.sigma, st.rounds,
            st.used_fallback ? " + comparison-sort fallback" : "",
            st.letters_per_chunk, secs);
    fprintf(stderr, "Total construction time: %.6f seconds\n", secs);
    fprintf(stderr, "Index size: %.6f MB (%zu bytes)\n", index_mb, index_bytes);
    fprintf(stderr, "Index memory: %.6fMB\n", index_mb);
    if (st.used_fallback)
        fprintf(stderr, "  %zu elements were still tied after the radix rounds; fallback oracle: %s\n", st.unresolved_after,
                st.fallback_mode == 2 ? "suffix array + LCP + RMQ" : "Karp-Rabin fingerprints");
    if (check) {
        std::string why;
        bool ok = zz::brute_check(text_copy, ZZA, ZZLCP, &why);
        fprintf(stderr, "check: %s %s\n", ok ? "OK" : "FAILED", why.c_str());
        if (!ok) return 2;
    }
    if (quiet) return 0;
    if (bin) {
        std::string base = outfile.empty() ? "out" : outfile;
        if (!write_array(base + ".zza", ZZA) || !write_array(base + ".zzlcp", ZZLCP)) {
            fprintf(stderr, "cannot write %s.zza / %s.zzlcp\n", base.c_str(), base.c_str()); return 1;
        }
        fprintf(stderr, "wrote %s.zza and %s.zzlcp (%zu x uint%zu each)\n", base.c_str(), base.c_str(), n, sizeof(IDX) * 8);
        return 0;
    }
    std::ostream* os = &std::cout;
    std::ofstream fout;
    if (!outfile.empty()) { fout.open(outfile); os = &fout; }
    for (size_t p = 0; p < n; ++p) *os << p << ' ' << ZZA[p] << ' ' << ZZLCP[p] << '\n';
    return 0;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: %s <textfile> [-o <outfile>] [--bin] [--check] [--quiet] [--raw]\n", argv[0]);
        return 1;
    }
    std::string outfile; bool check = false, quiet = false, raw = false, bin = false;
    for (int a = 2; a < argc; ++a) {
        if (!strcmp(argv[a], "-o") && a + 1 < argc) outfile = argv[++a];
        else if (!strcmp(argv[a], "--check")) check = true;
        else if (!strcmp(argv[a], "--quiet")) quiet = true;
        else if (!strcmp(argv[a], "--raw")) raw = true;
        else if (!strcmp(argv[a], "--bin")) bin = true;
    }
    std::string text;
    if (!read_file(argv[1], text)) { fprintf(stderr, "cannot read %s\n", argv[1]); return 1; }
    if (!raw) {
        size_t orig = text.size();
        while (!text.empty() && (text.back() == '\n' || text.back() == '\r')) text.pop_back();
        if (text.size() != orig)
            fprintf(stderr, "note: removed %zu trailing newline byte(s); use --raw to keep them\n", orig - text.size());
    }
    std::string text_copy;
    if (check) text_copy = text;
    auto construction_start = std::chrono::steady_clock::now();
    std::vector<zz::u8> letters;
    size_t sigma = 0;
    int b = zz::remap_alphabet(text, letters, &sigma);
    std::string().swap(text);
    if (letters.size() < 0xFFFFFFFFull)
        return run_main<uint32_t>(letters, b, sigma, text_copy, outfile, check, quiet, bin, construction_start);
    return run_main<uint64_t>(letters, b, sigma, text_copy, outfile, check, quiet, bin, construction_start);
}
#endif

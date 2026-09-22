#ifndef ZZT_ZIGZAG_LIBSAIS_BACKEND_H
#define ZZT_ZIGZAG_LIBSAIS_BACKEND_H

#include <cstddef>
#include <cstdint>
#include <vector>

namespace zzt_libsais {

struct BuildStats {
    std::size_t text_length = 0;
    std::size_t alphabet_size = 0;
    int bits_per_letter = 0;
    int letters_per_chunk = 0;
    std::size_t radix_rounds = 0;
    std::size_t unresolved_after_radix = 0;
    bool used_fallback = false;
    int fallback_mode = 0;
};

// Construct the full ZigZag array and ZigZag LCP directly from a byte text.
// Index must be int32_t or int64_t.  The 32-bit version rejects inputs that do
// not fit in the signed INT type used throughout the original ZZT code.
template <typename Index>
BuildStats build(const unsigned char* text, std::size_t text_length,
                 std::vector<Index>& zza, std::vector<Index>& zzlcp);

extern template BuildStats build<std::int32_t>(
    const unsigned char*, std::size_t, std::vector<std::int32_t>&,
    std::vector<std::int32_t>&);
extern template BuildStats build<std::int64_t>(
    const unsigned char*, std::size_t, std::vector<std::int64_t>&,
    std::vector<std::int64_t>&);

}  // namespace zzt_libsais

#endif

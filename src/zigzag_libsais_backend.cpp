#include "zigzag_libsais_backend.h"

#include <array>
#include <limits>
#include <stdexcept>
#include <type_traits>

// Reuse the optimized constructor without its stand-alone command-line main.
#define ZZ_NO_MAIN
#include "../vendor/zigzag-libsais/zigzag.cpp"

namespace zzt_libsais {

template <typename Index>
BuildStats build(const unsigned char* text, std::size_t text_length,
                 std::vector<Index>& zza, std::vector<Index>& zzlcp) {
    static_assert(std::is_same<Index, std::int32_t>::value ||
                      std::is_same<Index, std::int64_t>::value,
                  "ZZT optimized backend supports int32_t and int64_t only");

    if (text_length > static_cast<std::size_t>(std::numeric_limits<Index>::max())) {
        throw std::length_error("text is too large for the selected ZZT index width");
    }

    // Order-preserving remapping avoids an additional std::string copy of the
    // input while retaining exactly the byte alphabet order.
    std::array<int, 256> map{};
    map.fill(-1);
    for (std::size_t i = 0; i < text_length; ++i) map[text[i]] = 0;

    int sigma = 0;
    for (int ch = 0; ch < 256; ++ch) {
        if (map[static_cast<std::size_t>(ch)] == 0) {
            map[static_cast<std::size_t>(ch)] = sigma++;
        }
    }

    // Reserve one additional code, ordered after every actual byte, for the
    // original ZZT high boundary marker.
    int bits = 1;
    while ((1 << bits) <= sigma) ++bits;

    std::vector<zz::u8> letters(text_length);
    for (std::size_t i = 0; i < text_length; ++i) {
        letters[i] = static_cast<zz::u8>(map[text[i]]);
    }

    zz::Stats internal;
    zz::Params params;
    params.end_symbol = sigma;
    zz::build_from_letters<Index>(letters, bits, static_cast<std::size_t>(sigma),
                                  zza, zzlcp, params, &internal);

    BuildStats result;
    result.text_length = internal.n;
    result.alphabet_size = internal.sigma;
    result.bits_per_letter = internal.bits;
    result.letters_per_chunk = internal.letters_per_chunk;
    result.radix_rounds = internal.rounds;
    result.unresolved_after_radix = internal.unresolved_after;
    result.used_fallback = internal.used_fallback;
    result.fallback_mode = internal.fallback_mode;
    return result;
}

template BuildStats build<std::int32_t>(
    const unsigned char*, std::size_t, std::vector<std::int32_t>&,
    std::vector<std::int32_t>&);
template BuildStats build<std::int64_t>(
    const unsigned char*, std::size_t, std::vector<std::int64_t>&,
    std::vector<std::int64_t>&);

}  // namespace zzt_libsais

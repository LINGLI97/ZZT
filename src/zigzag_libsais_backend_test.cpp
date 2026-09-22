#include "zigzag_libsais_backend.h"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <random>
#include <vector>

namespace {

std::vector<unsigned char> zigzag(const std::vector<unsigned char>& text,
                                  std::size_t center) {
    std::vector<unsigned char> result;
    for (std::size_t pos = 1;; ++pos) {
        const long long index = static_cast<long long>(center) +
            ((pos % 2 == 0) ? 1 : -1) * static_cast<long long>(pos / 2);
        if (index < 0 || index >= static_cast<long long>(text.size())) break;
        result.push_back(text[static_cast<std::size_t>(index)]);
    }
    return result;
}

template <typename Index>
bool check(const std::vector<unsigned char>& text) {
    std::vector<Index> actual_order, actual_lcp;
    zzt_libsais::build<Index>(text.data(), text.size(), actual_order, actual_lcp);

    std::vector<std::vector<unsigned char>> strings(text.size());
    for (std::size_t i = 0; i < text.size(); ++i) strings[i] = zigzag(text, i);

    std::vector<std::size_t> expected_order(text.size());
    std::iota(expected_order.begin(), expected_order.end(), 0);
    std::stable_sort(expected_order.begin(), expected_order.end(),
                     [&](std::size_t left, std::size_t right) {
        const auto& a = strings[left];
        const auto& b = strings[right];
        std::size_t p = 0;
        while (p < a.size() && p < b.size() && a[p] == b[p]) ++p;
        if (p == a.size() || p == b.size()) {
            if (a.size() == b.size()) return false;
            return p != a.size();  // ended string has the high boundary symbol
        }
        return a[p] < b[p];
    });

    for (std::size_t rank = 0; rank < text.size(); ++rank) {
        if (static_cast<std::size_t>(actual_order[rank]) != expected_order[rank]) {
            std::cerr << "ZZA mismatch at rank " << rank << '\n';
            return false;
        }
        std::size_t lcp = 0;
        if (rank > 0) {
            const auto& a = strings[expected_order[rank - 1]];
            const auto& b = strings[expected_order[rank]];
            while (lcp < a.size() && lcp < b.size() && a[lcp] == b[lcp]) ++lcp;
        }
        if (static_cast<std::size_t>(actual_lcp[rank]) != lcp) {
            std::cerr << "ZZLCP mismatch at rank " << rank << '\n';
            return false;
        }
    }
    return true;
}

}  // namespace

int main() {
    std::mt19937_64 random(0x5a5a17ULL);
    for (std::size_t length = 1; length <= 160; ++length) {
        for (int repetition = 0; repetition < 20; ++repetition) {
            std::vector<unsigned char> text(length);
            const unsigned alphabet = 1 + static_cast<unsigned>(random() % 12);
            for (unsigned char& ch : text) {
                ch = static_cast<unsigned char>(1 + random() % alphabet);
            }
            if (!check<std::int32_t>(text) || !check<std::int64_t>(text)) return 1;
        }
    }
    std::cout << "optimized backend randomized 32/64-bit test: PASS\n";
    return 0;
}

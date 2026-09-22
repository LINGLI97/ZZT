#include "rmq-offline.h"

#include <algorithm>
#include <numeric>
#include <vector>

namespace {

INT find_root(std::vector<INT>& parent, INT x) {
    INT root = x;
    while (parent[static_cast<std::size_t>(root)] != root) {
        root = parent[static_cast<std::size_t>(root)];
    }
    while (parent[static_cast<std::size_t>(x)] != x) {
        const INT next = parent[static_cast<std::size_t>(x)];
        parent[static_cast<std::size_t>(x)] = root;
        x = next;
    }
    return root;
}

}  // namespace

// Linear-space offline RMQ (Arpa's monotone-stack method).  For each inclusive
// interval [L,R], O receives the position of a minimum value in A.
INT rmq_offline(INT* A, INT n, Query* Q, INT q) {
    if (n <= 0 || q <= 0) return 0;

    std::vector<INT> head(static_cast<std::size_t>(n), -1);
    std::vector<INT> next(static_cast<std::size_t>(q), -1);
    for (INT i = 0; i < q; ++i) {
        if (Q[i].L > Q[i].R) std::swap(Q[i].L, Q[i].R);
        if (Q[i].L < 0 || Q[i].R >= n) return -1;
        next[static_cast<std::size_t>(i)] = head[static_cast<std::size_t>(Q[i].R)];
        head[static_cast<std::size_t>(Q[i].R)] = i;
    }

    std::vector<INT> parent(static_cast<std::size_t>(n));
    std::iota(parent.begin(), parent.end(), static_cast<INT>(0));
    std::vector<INT> stack;
    stack.reserve(static_cast<std::size_t>(n));

    for (INT right = 0; right < n; ++right) {
        while (!stack.empty() &&
               A[stack.back()] > A[right]) {
            parent[static_cast<std::size_t>(stack.back())] = right;
            stack.pop_back();
        }
        stack.push_back(right);

        for (INT query = head[static_cast<std::size_t>(right)]; query != -1;
             query = next[static_cast<std::size_t>(query)]) {
            Q[query].O = find_root(parent, Q[query].L);
        }
    }
    return 0;
}

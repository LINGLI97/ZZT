// CC_BA.cpp — Baseline for Contextual Complexity via Dual Suffix Trees (Algorithm 1)
// Computes:
//   CC_T(P) = sum_{q>=0} |C_T(P, q)|
// where C_T(P, q) = set of distinct symmetric context pairs (L, R) with
// |L| = |R| = q such that LPR occurs in T.
//
// Algorithm:
//   1. Build forward ST on T$ and reverse ST on rev(T)$.
//   2. For each pattern P (length m), locate locus v of P in forward ST.
//   3. Single DFS over subtree(v): collect PRRange entries covering all depths.
//   4. For q = 0, 1, 2, ...:
//       d = m + q
//       Find all PRRanges whose [lo, hi] covers d → distinct PR strings at length d.
//       For each such PR: locate locus w of rev(PR) in reverse ST,
//                         count cut points at depth d+q in subtree(w) → distinct L's.
//       Break when no PRRange covers d.
//
// Note on textRev:
//   textRev[j] = T[text_size-1-j]  for j in [0, text_size),  textRev[text_size] = '$'
//   So rev(PR) where PR = T[s..s+d-1] starts at textRev[text_size - s - d].
//
// Build: make CC_BA
// Run:   ./run_CC_BA -f input.txt -p patterns.txt

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <unordered_set>
#include <vector>
#include <algorithm>
#include <cstring>
#include <chrono>
#include <stack>
#include <cctype>
#include <malloc.h>

#include "cmdline.h"
#include "suffixTree.h"
#include "SA_LCP_LCE.h"

using namespace std;

// ─── helpers ──────────────────────────────────────────────────────────────────

long long memory_usage() {
    struct mallinfo2 mi = mallinfo2();
    return mi.uordblks + mi.hblkhd;
}

void readfile_woDollar(const string& filename, const string& patternPath,
                       unsigned char*& text_string_woDollar,
                       vector<unsigned char*>& patterns,
                       INT& text_size, INT& alphabetSize,
                       vector<INT>& patternSizes)
{
    ifstream is_text(filename, ios::binary);
    if (!is_text) { cerr << "Error opening input file: " << filename << endl; return; }
    is_text.seekg(0, ios::end);
    text_size = is_text.tellg();
    is_text.seekg(0, ios::beg);

    unordered_set<unsigned char> alphabet;
    text_string_woDollar = (unsigned char*)malloc((text_size + 1) * sizeof(unsigned char));

    char c = 0;
    for (INT i = 0; i < text_size; i++) {
        is_text.read(&c, 1);
        text_string_woDollar[i] = (unsigned char)c;
        alphabet.insert((unsigned char)c);
    }
    is_text.close();
    alphabet.insert('$');
    text_string_woDollar[text_size] = '\0';
    alphabetSize = alphabet.size();

    ifstream is_pattern(patternPath, ios::binary);
    if (!is_pattern) { cerr << "Error opening pattern file: " << patternPath << endl; return; }
    string line;
    while (getline(is_pattern, line)) {
        istringstream iss(line);
        string pat_str;
        if (!(iss >> pat_str)) continue;
        unsigned char* pat = (unsigned char*)malloc(pat_str.size() + 1);
        copy(pat_str.begin(), pat_str.end(), pat);
        pat[pat_str.size()] = '\0';
        patternSizes.push_back((INT)pat_str.size());
        patterns.push_back(pat);
    }
    is_pattern.close();
}

unsigned char* addDollar(unsigned char* s, INT len) {
    unsigned char* out = (unsigned char*)malloc((len + 2) * sizeof(unsigned char));
    memcpy(out, s, len);
    out[len] = '$'; out[len + 1] = '\0';
    return out;
}

// Build rev(T)$ from textFwd = T$.
// textRev[j] = T[text_size-1-j]  for j in [0, text_size),  textRev[text_size] = '$'
unsigned char* buildRevText(unsigned char* textFwd, INT text_size) {
    unsigned char* rev = (unsigned char*)malloc((text_size + 2) * sizeof(unsigned char));
    for (INT j = 0; j < text_size; j++)
        rev[j] = textFwd[text_size - 1 - j];
    rev[text_size] = '$'; rev[text_size + 1] = '\0';
    return rev;
}

// Count cut points at depth d in the subtree of w (iterative DFS).
// Each cut point represents one distinct string of length d.
// Bug fix: a leaf node u satisfies u->depth = text_size+1 - u->start, so
// u->depth == d means the d-th character is '$'. Only count a leaf as a valid
// cut point at depth d if u->start + d <= text_size (no '$' in the string).
// Internal nodes never have '$' in their path label, so they're always valid.
INT countCutPoints(stNode* w, INT d, INT text_size) {
    auto valid = [&](stNode* u) {
        return !u->child.empty()          // internal: always valid
            || u->start + d <= text_size; // leaf: must not reach '$'
    };
    if (w->depth >= d) return valid(w) ? 1 : 0;
    INT cnt = 0;
    stack<stNode*> stk;
    stk.push(w);
    while (!stk.empty()) {
        stNode* u = stk.top(); stk.pop();
        if (u->depth >= d) { if (valid(u)) cnt++; continue; }
        for (auto& kv : u->child) stk.push(kv.second);
    }
    return cnt;
}

// Process a single (node u, depth d) pair for CC computation.
// d = length of PR string; cuttingDepth = 2d - m = depth we count L's at.
// Looks up rev(PR) in ST_rev, counts distinct L strings of length d-m at cuttingDepth.
static INT processPRDepth(stNode* u, INT d, INT m, INT n, INT text_size,
                           unsigned char* textRev, unsigned char* revBuf,
                           suffixTree& ST_rev)
{
    INT cuttingDepth = 2 * d - m;
    INT s        = u->start;
    INT revStart = n - s - d - 1;
    if (revStart < 0 || revStart + d > n) return 0;
    memcpy(revBuf, textRev + revStart, d);
    revBuf[d] = '\0';
    stNode* w = ST_rev.forward_search(revBuf, d);
    if (!w) return 0;
    // Leaf at exactly cuttingDepth means the d-th char is '$' — skip.
    if (w->child.empty() && w->depth == cuttingDepth) return 0;
    if (w->depth > d) {
        return (cuttingDepth <= w->depth) ? 1
                                          : countCutPoints(w, cuttingDepth, text_size);
    }
    return countCutPoints(w, cuttingDepth, text_size);
}



// ─── main ─────────────────────────────────────────────────────────────────────

int main(int argc, char* argv[]) {
    cmdline::parser parser;
    parser.add<string>("filePath",    'f', "path to input text file", false, "input.txt");
    parser.add<string>("patternPath", 'p', "path to pattern file",    false, "patterns.txt");
    parser.parse_check(argc, argv);

    string filePath    = parser.get<string>("filePath");
    string patternPath = parser.get<string>("patternPath");

    unsigned char* textStringWoDollar = nullptr;
    vector<unsigned char*> patterns;
    INT text_size = 0, alphabetSize = 0;
    vector<INT> patternSizes;

    readfile_woDollar(filePath, patternPath,
                      textStringWoDollar, patterns,
                      text_size, alphabetSize, patternSizes);

    unsigned char* textFwd = addDollar(textStringWoDollar, text_size);
    unsigned char* textRev = buildRevText(textFwd, text_size);
    INT n = text_size + 1;  // length of textFwd / textRev (including '$')

    // ── Build both suffix trees ────────────────────────────────────────────────
    auto t0 = chrono::high_resolution_clock::now();
    long long memStart = memory_usage();

    suffixTree ST_fwd(textFwd, n);
    suffixTree ST_rev(textRev, n);

    auto t1 = chrono::high_resolution_clock::now();
    long long memEnd = memory_usage();

    cout << "=== CC Baseline (Dual Suffix Tree) ===" << endl;
    cout << "Construction time: "
         << chrono::duration_cast<chrono::microseconds>(t1 - t0).count() * 1e-6
         << " s" << endl;
    cout << "Index memory: " << (memEnd - memStart) * 1e-6 << " MB" << endl;
    cout << "=== Start to query patterns: CC Baseline ===" << endl;

    // ── Query each pattern ─────────────────────────────────────────────────────
    for (INT pi = 0; pi < (INT)patterns.size(); pi++) {

        auto qStart = chrono::high_resolution_clock::now();

        unsigned char* P = patterns[pi];
        INT m            = patternSizes[pi];

        // Locate locus of P in forward suffix tree.
        // forward_search returns the lowest explicit node with depth >= m.
        stNode* v = ST_fwd.forward_search(P, m);

        if (!v) {
            cout << "Pattern " << pi << ": " << P << endl;
            cerr << "Pattern does not occur in the text." << endl;
            cout << "---------------------------------------" << endl;
            continue;
        }

        INT total = 0;
        INT lociCount = 0;  // number of loci in subtree(v) = total processPRDepth calls

        // Inline DFS over subtree(v): process each (node, depth) pair on-the-fly.
        // Avoids O(n^2) memory from storing all pairs in a vector.
        unsigned char* revBuf = (unsigned char*)malloc((n + 1) * sizeof(unsigned char));

        // v's own depths [m, v->depth] (v is the locus of P)
        {
            INT hi = v->child.empty() ? v->depth - 1 : v->depth;
            for (INT d = m; d <= hi; d++) {
                lociCount++;
                total += processPRDepth(v, d, m, n, text_size, textRev, revBuf, ST_rev);
            }
        }

        // DFS over strict descendants of v
        {
            stack<pair<stNode*, INT>> stk;  // (node, parentDepth)
            for (auto& kv : v->child) stk.push({kv.second, v->depth});
            while (!stk.empty()) {
                auto [u, parentD] = stk.top(); stk.pop();
                INT lo = parentD + 1;
                INT hi = u->child.empty() ? u->depth - 1 : u->depth;
                for (INT d = lo; d <= hi; d++) {
                    lociCount++;
                    total += processPRDepth(u, d, m, n, text_size, textRev, revBuf, ST_rev);
                }
                for (auto& kv : u->child) stk.push({kv.second, u->depth});
            }
        }

        free(revBuf);
        auto qEnd = chrono::high_resolution_clock::now();

        cout << "Pattern " << pi << ": " << P << endl;
        cout << "CC total: " << total << endl;
        cout << "loci(v): " << lociCount << endl;
        cout << "Query time: "
             << chrono::duration_cast<chrono::microseconds>(qEnd - qStart).count() * 1e-6
             << " s" << endl;
        cout << "---------------------------------------" << endl;
    }

    // ── Cleanup ────────────────────────────────────────────────────────────────
    free(textFwd);
    free(textRev);
    free(textStringWoDollar);
    for (auto& p : patterns) free(p);

    return 0;
}

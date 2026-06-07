// LCCS_BA_ST.cpp — Generalized Suffix Tree baseline for LCCS (multi-text)
//
// Implements Algorithm 1 from the paper.
//
// Problem: Given c texts T_1,...,T_c, threshold tau, query P of length m,
//          find the longest LPR with |L|=|R| appearing in >= tau distinct texts.
//
// Index:
//   ST    — generalized suffix tree on  T_1 $1 T_2 $2 ... T_c $c [255]
//   ST^R  — generalized suffix tree on  (T_1)^R $1 (T_2)^R $2 ... (T_c)^R $c [255]
//   (same separator bytes used in both; separators chosen from bytes [128,254])
//
// For every node v in ST and ST^R, colorCount(v) = #distinct text indices in subtree(v).
// The user must implement computeColorCounts() below.
//
// Query algorithm (per pattern P):
//   1. Find locus v of P in ST.
//   2. DFS over subtree(v); for each explicit node u with colorCount >= tau:
//        a. len_PR = string depth of u (clipped to within the text for leaves).
//        b. r = len_PR - m,  d = 2r + m  (target LPR length).
//        c. Construct (PR)^R from the representative occurrence at u->start.
//        d. Find locus w of (PR)^R in ST^R.
//        e. If any node in subtree(w) at string depth d has colorCount >= tau,
//           update bestLen.
//   3. Output bestLen.
//
// Build: make LCCS_BA_ST
// Run:   ./run_LCCS_BA_ST -f multiInput.txt -p patterns.txt -t 2

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
#include <malloc.h>

#include "cmdline.h"
#include "suffixTree.h"
#include "rmq-offline.h"

using namespace std;

// ─── memory helper ────────────────────────────────────────────────────────────

static long long memory_usage() {
    struct mallinfo2 mi = mallinfo2();
    return mi.uordblks + mi.hblkhd;
}

// ─── I/O ──────────────────────────────────────────────────────────────────────

// Read c texts (one per non-empty line, ASCII only).
// Builds:
//   textFwd  = T_0 $0 T_1 $1 ... T_{c-1} $_{c-1}  [255]
//   textRev  = (T_0)^R $0 (T_1)^R $1 ... (T_{c-1})^R $_{c-1}  [255]
//
// Separator bytes chosen from [128,254] not present in any text.
// tstart_fwd[i] / tend_fwd[i] = first/last position of T_i in textFwd (excl. separator).
// sep_fwd[i]   = position of separator $i between T_i and T_{i+1} in textFwd.
// (Analogous _rev arrays for textRev.)
// n = length of textFwd including terminal 255.
static void readMultiText(
    const string& filename,
    const string& patternPath,
    unsigned char*& textFwd,
    unsigned char*& textRev,
    vector<unsigned char*>& patterns,
    INT& n,
    int& c,
    vector<INT>& tstart_fwd, vector<INT>& tend_fwd, vector<INT>& sep_fwd,
    vector<INT>& tstart_rev, vector<INT>& tend_rev, vector<INT>& sep_rev,
    vector<INT>& patternSizes)
{
    ifstream is(filename);
    if (!is) { cerr << "Cannot open: " << filename << endl; return; }

    unordered_set<unsigned char> alphabet;
    vector<string> lines;
    string line;
    while (getline(is, line)) {
        if (line.empty()) continue;
        string filtered;
        for (char ch : line) {
            unsigned char uc = (unsigned char)ch;
            if (uc <= 127) { filtered += ch; alphabet.insert(uc); }
        }
        if (!filtered.empty()) lines.push_back(filtered);
    }
    is.close();

    c = (int)lines.size();

    // Pick separator bytes not already in the alphabet.
    vector<unsigned char> avail_sep;
    for (int i = 128; i <= 254; i++)
        if (!alphabet.count((unsigned char)i)) avail_sep.push_back((unsigned char)i);
    if ((int)avail_sep.size() < c) {
        cerr << "Not enough free separator bytes." << endl; return;
    }

    // Build forward concatenation.
    string fwd, rev;
    for (int i = 0; i < c; i++) {
        tstart_fwd.push_back((INT)fwd.size());
        fwd += lines[i];
        tend_fwd.push_back((INT)fwd.size() - 1);
        if (i < c - 1) {
            sep_fwd.push_back((INT)fwd.size());
            fwd += (char)avail_sep[i];
        }

        tstart_rev.push_back((INT)rev.size());
        rev += string(lines[i].rbegin(), lines[i].rend());
        tend_rev.push_back((INT)rev.size() - 1);
        if (i < c - 1) {
            sep_rev.push_back((INT)rev.size());
            rev += (char)avail_sep[i];
        }
    }

    INT text_size = (INT)fwd.size();
    n = text_size + 1;  // +1 for terminal 255

    textFwd = (unsigned char*)malloc(n + 1);
    memcpy(textFwd, fwd.data(), text_size);
    textFwd[text_size] = 255;
    textFwd[n] = '\0';

    textRev = (unsigned char*)malloc(n + 1);
    memcpy(textRev, rev.data(), text_size);
    textRev[text_size] = 255;
    textRev[n] = '\0';

    // Read patterns.
    ifstream ip(patternPath);
    if (!ip) { cerr << "Cannot open: " << patternPath << endl; return; }
    while (getline(ip, line)) {
        istringstream iss(line);
        string ps;
        if (!(iss >> ps)) continue;
        string fp;
        for (char ch : ps) if ((unsigned char)ch <= 127) fp += ch;
        if (fp.empty()) continue;
        unsigned char* pat = (unsigned char*)malloc(fp.size() + 1);
        memcpy(pat, fp.data(), fp.size());
        pat[fp.size()] = '\0';
        patternSizes.push_back((INT)fp.size());
        patterns.push_back(pat);
    }
    ip.close();
}

// ─── color helper ─────────────────────────────────────────────────────────────

// Returns the 0-based text index for a position in the concatenated string.
// sep[j] = position of the separator between text j and text j+1.
// Returns the count of separators strictly before pos, which equals the text index.
static int find_color(INT pos, const vector<INT>& sep) {
    int lo = 0, hi = (int)sep.size();
    while (lo < hi) {
        int mid = (lo + hi) / 2;
        if (sep[mid] < pos) lo = mid + 1;
        else                hi = mid;
    }
    return lo;  // 0-based text index
}

// ─── Color count: Hui (1992) algorithm ───────────────────────────────────────
//
// Computes node->colorCount for every node via the identity:
//
//   colorCount(v) = valid_leafCount(v) - duplicate(v)
//
// where:
//   valid_leafCount(v) = #leaves in subtree(v) that are inside some text (not at a separator)
//   duplicate(v)       = CPLcount(v) + Σ_{child c of v} duplicate(c)
//   CPLcount(v)        = #{consecutive same-color leaf pairs (x, prev) in DFS order
//                          whose LCA is exactly v}
//
// LCA is computed via Euler-tour + offline RMQ (rmq_offline).
//
// A leaf at pos is VALID iff  col = find_color(pos, sep)  satisfies
//   col < c  AND  tstart[col] <= pos <= tend[col].
// The color of a valid leaf is col.
//
// Called once for ST_fwd and once for ST_rev.
void computeColorCounts(stNode* root,
                        const vector<INT>& tstart,
                        const vector<INT>& tend,
                        const vector<INT>& sep,
                        int c)
{
    // ── Step 1: post-order DFS ────────────────────────────────────────────────
    // Assign each node a post-order integer ID.
    // Collect valid leaves; for each, record the ID of the previous same-color leaf.
    vector<stNode*> allNodes;          // allNodes[id] = node
    vector<stNode*> leafList;          // valid leaves, in DFS post-order
    unordered_map<stNode*, INT> nid;   // node → post-order id
    unordered_map<INT, INT> lastleaf;  // leaf_id → id of previous same-color leaf (-1 = none)
    {
        unordered_map<int, INT> last_seen;  // color → id of last valid leaf seen

        stack<pair<stNode*, bool>> st;
        st.push({root, false});
        while (!st.empty()) {
            auto [u, visited] = st.top();
            st.pop();
            if (!visited) {
                st.push({u, true});
                for (auto it = u->child.rbegin(); it != u->child.rend(); ++it)
                    st.push({it->second, false});
            } else {
                INT uid = (INT)allNodes.size();
                nid[u] = uid;
                allNodes.push_back(u);

                if (u->child.empty()) {
                    int col = find_color(u->start, sep);
                    bool valid = col >= 0 && col < c
                                 && u->start >= tstart[col] && u->start <= tend[col];
                    if (valid) {
                        auto it = last_seen.find(col);
                        lastleaf[uid] = (it == last_seen.end()) ? -1 : it->second;
                        last_seen[col] = uid;
                        leafList.push_back(u);
                    }
                }
            }
        }
    }

    INT N = (INT)allNodes.size();

    // ── Step 2: valid_leafCount (bottom-up, allNodes is post-order) ───────────
    vector<INT> vlc(N, 0);
    for (stNode* u : allNodes) {
        INT uid = nid[u];
        if (u->child.empty()) {
            int col = find_color(u->start, sep);
            vlc[uid] = (col >= 0 && col < c
                        && u->start >= tstart[col] && u->start <= tend[col]) ? 1 : 0;
        } else {
            INT sum = 0;
            for (auto& kv : u->child) sum += vlc[nid[kv.second]];
            vlc[uid] = sum;
        }
    }

    // ── Step 3: Euler tour for LCA ────────────────────────────────────────────
    // Each node is recorded every time we (re-)visit it: once before each child
    // and once after the last child. Total length = 2N - 1.
    // R[id] = index of the FIRST occurrence of node id in E.
    vector<INT> E, L;
    E.reserve(2 * N - 1);
    L.reserve(2 * N - 1);
    vector<INT> R(N, -1);
    {
        // Frame: (node, depth, next-child-index)
        stack<tuple<stNode*, INT, INT>> st;
        st.push({root, 0, 0});
        while (!st.empty()) {
            auto [u, depth, cidx] = st.top();
            st.pop();

            INT uid = nid[u];
            if (R[uid] == -1) R[uid] = (INT)E.size();
            E.push_back(uid);
            L.push_back(depth);

            // Advance to the cidx-th child
            auto it = u->child.begin();
            for (INT i = 0; i < cidx && it != u->child.end(); ++i, ++it) {}

            if (it != u->child.end()) {
                // Re-push parent (incremented child index) then push child
                st.push({u, depth, cidx + 1});
                st.push({it->second, depth + 1, 0});
            }
        }
    }

    // ── Step 4: build RMQ queries for same-color leaf pairs ──────────────────
    // For each valid leaf x with a previous same-color leaf prev,
    // we query RMQ(R[prev], R[x]) (or flipped so L <= R) to find their LCA.
    INT q = 0;
    for (auto& [uid, prev] : lastleaf)
        if (prev != -1) q++;

    vector<INT> CPLcount(N, 0);

    if (q > 0) {
        vector<Query> Q_rmq(q);
        INT qi = 0;
        for (auto& [uid, prev] : lastleaf) {
            if (prev == -1) continue;
            INT a = R[uid], b = R[prev];
            if (a > b) swap(a, b);
            Q_rmq[qi].L = a;
            Q_rmq[qi].R = b;
            qi++;
        }

        rmq_offline(L.data(), (INT)L.size(), Q_rmq.data(), q);

        // ── Step 5: CPLcount – tally LCA hits ────────────────────────────────
        for (INT i = 0; i < q; i++)
            CPLcount[E[Q_rmq[i].O]]++;
    }

    // Free large tour arrays early
    { vector<INT>().swap(E); vector<INT>().swap(L); vector<INT>().swap(R); }

    // ── Step 6: duplicate (bottom-up) ────────────────────────────────────────
    vector<INT> dup(N, 0);
    for (stNode* u : allNodes) {
        INT uid = nid[u];
        INT sum = CPLcount[uid];
        for (auto& kv : u->child) sum += dup[nid[kv.second]];
        dup[uid] = sum;
    }

    // ── Step 7: colorCount = valid_leafCount - duplicate ──────────────────────
    for (stNode* u : allNodes) {
        u->leafCount  = vlc[nid[u]];                    // total occurrences across all texts
        u->colorCount = vlc[nid[u]] - dup[nid[u]];
    }
}

// ─── Tree visualization: DOT/PNG export ───────────────────────────────────────
//
// Exports the generalized suffix tree to a Graphviz DOT file and converts it
// to a PNG image via `dot -Tpng`.
//
// Node styling:
//   Internal node  — green  if colorCount >= tau, grey otherwise
//                    label: "depth=D | colors=K"
//   Valid leaf     — colored by text index (one color per text)
//                    label: "text C | start=S"
//   Sep/invalid leaf — dark grey
// Edge label: actual substring from parent depth to child depth (up to 20 chars)
//
// Use -v to enable. Output files: <stem>_fwd.dot/.png and <stem>_rev.dot/.png

// Fill colors for leaves belonging to text 0..7 (cycled for more texts)
static const char* LEAF_HEX[] = {
    "#FF6B6B",   // text 0  — red
    "#6BCB77",   // text 1  — green
    "#4D96FF",   // text 2  — blue
    "#FFD93D",   // text 3  — yellow
    "#FF8EC8",   // text 4  — magenta
    "#6DECB9",   // text 5  — cyan
    "#C5A3FF",   // text 6  — purple
    "#FFA07A",   // text 7  — salmon
};
static const int N_LEAF_HEX = 8;

// Escape a string for use inside a DOT double-quoted label.
static string dotEscape(const unsigned char* text, INT start, INT end) {
    string s;
    for (INT k = start; k <= end; k++) {
        unsigned char ch = text[k];
        if      (ch == '"')                 s += "\\\"";
        else if (ch == '\\')               s += "\\\\";
        else if (ch >= 32 && ch < 127)     s += (char)ch;
        else { s += '['; s += to_string((int)ch); s += ']'; }
        if ((INT)s.size() >= 20) { s += ".."; break; }
    }
    return s;
}

static void exportDotDFS(stNode* node, ofstream& out,
                         const unsigned char* text,
                         const vector<INT>& sep,
                         const vector<INT>& tstart, const vector<INT>& tend,
                         int c, INT tau)
{
    bool isLeaf = node->child.empty();
    string label, fill, fontcolor = "black";

    if (node->parent == nullptr) {
        // Root
        label    = "ROOT\\ncolors=" + to_string(node->colorCount);
        fill     = (node->colorCount >= tau) ? "#90EE90" : "#DDDDDD";
    } else if (isLeaf) {
        int col  = find_color(node->start, sep);
        bool valid = col >= 0 && col < c
                     && node->start >= tstart[col] && node->start <= tend[col];
        if (valid) {
            label = "text " + to_string(col) + "\\nstart=" + to_string(node->start);
            fill  = LEAF_HEX[col % N_LEAF_HEX];
        } else {
            label     = "sep\\nstart=" + to_string(node->start);
            fill      = "#AAAAAA";
            fontcolor = "white";
        }
    } else {
        label = "depth=" + to_string(node->depth)
              + "\\ncolors=" + to_string(node->colorCount);
        fill  = (node->colorCount >= tau) ? "#90EE90" : "#DDDDDD";
    }

    out << "  \"" << (void*)node << "\" [label=\"" << label
        << "\", fillcolor=\"" << fill
        << "\", fontcolor=\"" << fontcolor << "\"];\n";

    // Edges + recurse
    for (auto& kv : node->child) {
        stNode* child    = kv.second;
        INT     estart   = child->start + node->depth;
        INT     eend     = child->start + child->depth - 1;
        string  el       = dotEscape(text, estart, eend);

        out << "  \"" << (void*)node << "\" -> \"" << (void*)child
            << "\" [label=\"" << el << "\"];\n";
        exportDotDFS(child, out, text, sep, tstart, tend, c, tau);
    }
}

static void exportColorDot(const string& dotPath, const string& title,
                            stNode* root, const unsigned char* text,
                            const vector<INT>& sep,
                            const vector<INT>& tstart, const vector<INT>& tend,
                            int c, INT tau)
{
    ofstream out(dotPath);
    if (!out) { cerr << "Cannot open: " << dotPath << "\n"; return; }

    out << "digraph G {\n";
    out << "  label=\"" << title << "\\ntau=" << tau << "  texts=" << c << "\";\n";
    out << "  labelloc=t; labeljust=l;\n";
    out << "  node [fontname=\"Courier\", shape=box, style=filled, fontsize=10];\n";
    out << "  edge [fontsize=9];\n";
    out << "  graph [nodesep=0.3, ranksep=0.8, splines=polyline];\n";

    // Legend subgraph
    out << "  subgraph cluster_legend {\n";
    out << "    label=\"Legend\"; style=filled; fillcolor=white; fontsize=10;\n";
    for (int i = 0; i < c; i++)
        out << "    leg" << i << " [label=\"text " << i << "\","
            << " fillcolor=\"" << LEAF_HEX[i % N_LEAF_HEX] << "\"];\n";
    out << "    leg_freq [label=\"tau-frequent\", fillcolor=\"#90EE90\"];\n";
    out << "    leg_sep  [label=\"separator\",    fillcolor=\"#AAAAAA\","
           " fontcolor=white];\n";
    out << "  }\n";

    exportDotDFS(root, out, text, sep, tstart, tend, c, tau);
    out << "}\n";
    out.close();
    cout << "Exported DOT: " << dotPath << "\n";
}

// ─── depth-d frequency check ──────────────────────────────────────────────────
//
// Returns true if subtree(w) contains a node w' at string depth d with
// w'.colorCount >= tau, where the occurrence doesn't cross text boundaries.
//
// Bug fix: leaf nodes in ST^R have depth = n - pos (including the terminal [255]).
// A leaf found at depth >= d must be further verified: pos + d - 1 <= tend[col].
// Internal nodes are always safe because their path label at depth d cannot
// include [255] or a separator (that would make colorCount = 0).
static inline bool leafInBounds(stNode* u, INT d,
                                 const vector<INT>& sep,
                                 const vector<INT>& tend, int c) {
    if (!u->child.empty()) return true;  // internal — always valid
    int col = find_color(u->start, sep);
    return col < c && u->start + d - 1 <= tend[col];
}

static bool existsColorFreqAtDepth(stNode* w, INT d, INT tau,
                                    const vector<INT>& sep,
                                    const vector<INT>& tend, int c) {
    if (w->colorCount < tau) return false;

    if (w->depth >= d)
        return leafInBounds(w, d, sep, tend, c);

    stack<stNode*> stk;
    for (auto& kv : w->child) stk.push(kv.second);

    while (!stk.empty()) {
        stNode* u = stk.top(); stk.pop();
        if (u->colorCount < tau) continue;
        if (u->parent->depth < d && u->depth >= d) {
            if (leafInBounds(u, d, sep, tend, c)) return true;
            // leaf crosses boundary — keep searching siblings
            continue;
        }
        if (u->depth < d)
            for (auto& kv : u->child) stk.push(kv.second);
    }
    return false;
}

// ─── main ─────────────────────────────────────────────────────────────────────

int main(int argc, char* argv[]) {
    cmdline::parser parser;
    parser.add<string>("filePath",    'f', "path to multi-text file",         false, "multiInput.txt");
    parser.add<string>("patternPath", 'p', "path to pattern file",             false, "patterns.txt");
    parser.add<int>   ("tau",         't', "support threshold",                false, 1);
    parser.add        ("visualize",   'v', "print suffix trees after building");
    parser.add<int>   ("max-depth",   'd', "max depth shown in visualization", false, 5);
    parser.parse_check(argc, argv);

    string filePath    = parser.get<string>("filePath");
    string patternPath = parser.get<string>("patternPath");
    INT    tau         = parser.get<int>("tau");
    bool   visualize   = parser.exist("visualize");
    int    maxDepth    = parser.get<int>("max-depth");

    unsigned char* textFwd = nullptr;
    unsigned char* textRev = nullptr;
    vector<unsigned char*> patterns;
    INT n = 0;
    int c = 0;
    vector<INT> tstart_fwd, tend_fwd, sep_fwd;
    vector<INT> tstart_rev, tend_rev, sep_rev;
    vector<INT> patternSizes;

    readMultiText(filePath, patternPath,
                  textFwd, textRev, patterns, n, c,
                  tstart_fwd, tend_fwd, sep_fwd,
                  tstart_rev, tend_rev, sep_rev,
                  patternSizes);

    auto t0 = chrono::high_resolution_clock::now();
    long long memStart = memory_usage();

    // Build both generalized suffix trees.
    suffixTree ST_fwd(textFwd, n);
    suffixTree ST_rev(textRev, n);

    // Compute color counts for both trees (USER implements computeColorCounts).
    computeColorCounts(ST_fwd.root, tstart_fwd, tend_fwd, sep_fwd, c);
    computeColorCounts(ST_rev.root, tstart_rev, tend_rev, sep_rev, c);

    if (visualize) {
        exportColorDot("ST_fwd.dot", "Forward Suffix Tree (T$)",
                       ST_fwd.root, textFwd, sep_fwd, tstart_fwd, tend_fwd, c, tau);
        exportColorDot("ST_rev.dot", "Reverse Suffix Tree (T^R$)",
                       ST_rev.root, textRev, sep_rev, tstart_rev, tend_rev, c, tau);
        system("dot -Tpng ST_fwd.dot -o ST_fwd.png && echo 'Saved: ST_fwd.png'");
        system("dot -Tpng ST_rev.dot -o ST_rev.png && echo 'Saved: ST_rev.png'");
    }

    auto t1 = chrono::high_resolution_clock::now();
    long long memEnd = memory_usage();

    cout << "=== LCCS Baseline (Generalized Suffix Tree) ===" << endl;
    cout << "Tau: " << tau << " | Texts: " << c << " | N: " << (n - 1) << endl;
    cout << "Construction + color time: "
         << chrono::duration_cast<chrono::microseconds>(t1 - t0).count() * 1e-6
         << " s" << endl;
    cout << "Index memory: " << (memEnd - memStart) * 1e-6 << " MB" << endl;
    cout << "=== Start querying ===" << endl;

    for (INT pi = 0; pi < (INT)patterns.size(); pi++) {
        auto qStart = chrono::high_resolution_clock::now();

        unsigned char* P = patterns[pi];
        INT m            = patternSizes[pi];

        // ── Step 1: locate locus of P in ST ──────────────────────────────────
        stNode* v = ST_fwd.forward_search(P, m);
        if (!v) {
            cout << "Pattern " << pi << ": " << P << endl;
            cerr << "Pattern not found in any text." << endl;
            cout << "---------------------------------------" << endl;
            continue;
        }

        INT bestLen = -1;

        // ── Step 2: DFS over subtree(v) ───────────────────────────────────────
        // Fix: for each explicit node u, check ALL lengths on the edge
        // [parent->depth+1 .. u->depth], not just u->depth.
        // This handles both (a) mid-edge locus (P ends at implicit node) and
        // (b) gaps between consecutive explicit nodes.
        unsigned char* revBuf = (unsigned char*)malloc(n + 1);  // reused each iteration

        stack<stNode*> dfs;
        dfs.push(v);

        while (!dfs.empty()) {
            stNode* u = dfs.top(); dfs.pop();

            // Pruning: subtree has too few distinct colors.
            if (u->colorCount < tau) continue;

            INT pos = u->start;

            // Determine which text this representative occurrence belongs to.
            int col = find_color(pos, sep_fwd);
            if (col < 0 || col >= c || pos < tstart_fwd[col] || pos > tend_fwd[col]) {
                // pos starts at a separator — path label invalid for all occurrences.
                continue;
            }

            INT max_valid_len = tend_fwd[col] - pos + 1;

            // Determine range of lengths to check on this edge:
            //   len_end   = min(u->depth, max_valid_len)
            //   len_start = max(parent->depth + 1, m)   (ensures r = len_PR - m >= 0)
            INT parent_depth = u->parent ? u->parent->depth : 0;
            INT len_end      = u->child.empty() ? max_valid_len
                                                 : min(u->depth, max_valid_len);
            INT len_start    = max(parent_depth + 1, m);

            // Push children only if path label at u->depth stays within the text.
            if (!u->child.empty() && u->depth <= max_valid_len) {
                for (auto& kv : u->child) dfs.push(kv.second);
            }
            // If u->depth > max_valid_len: path label crosses separator;
            // all descendants inherit it — don't push children.

            if (len_start > len_end) continue;

            // ── Inner loop: every length on this edge, longest first ──────────
            for (INT len_PR = len_end; len_PR >= len_start; len_PR--) {
                INT r = len_PR - m;
                INT d = 2 * r + m;
                if (d <= bestLen) break;   // smaller len_PR → smaller d; can't improve

                // ── Step 3: construct rev(PR) ─────────────────────────────────
                for (INT k = 0; k < len_PR; k++)
                    revBuf[k] = textFwd[pos + len_PR - 1 - k];
                revBuf[len_PR] = '\0';

                // ── Step 4: locus of rev(PR) in ST^R ─────────────────────────
                stNode* w = ST_rev.forward_search(revBuf, len_PR);
                if (!w) continue;

                // ── Step 5: any node in subtree(w) at depth d is tau-frequent? ─
                if (existsColorFreqAtDepth(w, d, tau, sep_rev, tend_rev, c))
                    bestLen = d;
            }
        }

        free(revBuf);

        // ── Second pass: locus of P^R in ST(T^R), searching in ST(T) ─────────
        // Symmetric to the first pass (correctness by Lemma 1 analogue for LCCS).
        unsigned char* P_rev2 = (unsigned char*)malloc((m + 1) * sizeof(unsigned char));
        for (INT k = 0; k < m; k++) P_rev2[k] = P[m - 1 - k];
        P_rev2[m] = '\0';

        stNode* v_rev2 = ST_rev.forward_search(P_rev2, m);
        free(P_rev2);

        if (v_rev2) {
            unsigned char* revBuf2 = (unsigned char*)malloc(n + 1);
            stack<stNode*> dfs2;
            dfs2.push(v_rev2);

            while (!dfs2.empty()) {
                stNode* u2 = dfs2.top(); dfs2.pop();
                if (u2->colorCount < tau) continue;

                INT pos2 = u2->start;
                int col2 = find_color(pos2, sep_rev);
                if (col2 < 0 || col2 >= c || pos2 < tstart_rev[col2] || pos2 > tend_rev[col2])
                    continue;

                INT max_valid_len2 = tend_rev[col2] - pos2 + 1;
                INT parent_depth2  = u2->parent ? u2->parent->depth : 0;
                INT len_end2   = u2->child.empty() ? max_valid_len2 : min(u2->depth, max_valid_len2);
                INT len_start2 = max(parent_depth2 + 1, m);

                if (!u2->child.empty() && u2->depth <= max_valid_len2)
                    for (auto& kv : u2->child) dfs2.push(kv.second);

                if (len_start2 > len_end2) continue;

                // u2 represents (LP)^R in T^R; reverse it to get LP and search in ST(T).
                for (INT len_LP_rev = len_end2; len_LP_rev >= len_start2; len_LP_rev--) {
                    INT l  = len_LP_rev - m;
                    INT d2 = 2 * l + m;
                    if (d2 <= bestLen) break;

                    for (INT k = 0; k < len_LP_rev; k++)
                        revBuf2[k] = textRev[pos2 + len_LP_rev - 1 - k];
                    revBuf2[len_LP_rev] = '\0';

                    stNode* w2 = ST_fwd.forward_search(revBuf2, len_LP_rev);
                    if (!w2) continue;

                    if (existsColorFreqAtDepth(w2, d2, tau, sep_fwd, tend_fwd, c))
                        bestLen = d2;
                }
            }
            free(revBuf2);
        }

        auto qEnd = chrono::high_resolution_clock::now();
        double qt = chrono::duration_cast<chrono::microseconds>(qEnd - qStart).count() * 1e-6;

        cout << "Pattern " << pi << ": " << P << endl;
        cout << "occ_T(P): " << v->leafCount << endl;
        if (bestLen < 0)
            cerr << "P exists but no LPR is " << tau << "-frequent." << endl;
        else
            cout << "LPR length: " << bestLen
                 << " (|L|=|R|=" << (bestLen - m) / 2 << ")" << endl;
        cout << "Query time: " << qt << " s" << endl;
        cout << "---------------------------------------" << endl;
    }

    // ── Cleanup ───────────────────────────────────────────────────────────────
    free(textFwd);
    free(textRev);
    for (auto& p : patterns) free(p);
    return 0;
}

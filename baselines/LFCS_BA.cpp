// make run_LFCS_BA
//./run_LFCS_BA -t 2
// dot -Tpng tree_fwd.dot -o tree_fwd.png
// dot -Tpng tree_rev.dot -o tree_rev.png


#include <iostream>
#include "cmdline.h"
#include <string>
#include <fstream>
#include <unordered_set>
#include <string>
#include <vector>
#include <malloc.h>
#include <cmath>
#include <algorithm>
#include <cstring>
#include <chrono>
#include <stack>
#include <cctype>
#include <sdsl/rmq_support.hpp>
#include "suffixTree.h"
#include "SA_LCP_LCE.h"

using namespace std;


long long memory_usage() {
    struct mallinfo2 mi = mallinfo2();
    return mi.uordblks + mi.hblkhd;
}


void readfile_woDollar(string &filename, string &patternPath, unsigned char * &text_string_woDollar, std::vector<unsigned char *> &patterns, INT& text_size, INT& alphabetSize, vector<INT> & patternSizes){
    std::ifstream is_text(filename, std::ios::binary);
    if (!is_text) {
        std::cerr << "Error opening input file: " << filename << std::endl;
        return;
    }

    is_text.seekg(0, std::ios::end);
    text_size = is_text.tellg();
    is_text.seekg(0, std::ios::beg);
    unordered_set<unsigned char> alphabet;

    text_string_woDollar = (unsigned char *)malloc((text_size + 1) * sizeof(unsigned char));

    char c = 0;
    for (INT i = 1; i < text_size + 1; i++) {
        is_text.read(reinterpret_cast<char *>(&c), 1);
        text_string_woDollar[i-1] = (unsigned char)c;
        alphabet.insert((unsigned char)c);
    }

    is_text.close();
    alphabet.insert('$');
    text_string_woDollar[text_size] = '\0';
    alphabetSize = alphabet.size();

    std::ifstream is_pattern(patternPath, std::ios::binary);
    if (!is_pattern) {
        std::cerr << "Error opening pattern file: " << patternPath << std::endl;
    }

    std::string line;
    while (std::getline(is_pattern, line)) {
        std::istringstream iss(line);
        std::string pattern_str;
        if (!(iss >> pattern_str)) {
            std::cerr << "Error reading line: " << line << std::endl;
            continue;
        }
        unsigned char *pattern = (unsigned char *)malloc((pattern_str.size() + 1) * sizeof(unsigned char));
        std::copy(pattern_str.begin(), pattern_str.end(), pattern);
        pattern[pattern_str.size()] = '\0';
        patternSizes.push_back(pattern_str.size());
        patterns.push_back(pattern);
    }
    is_pattern.close();
}


unsigned char * addDollar(unsigned char * textStringWoDollar, INT text_size){
    unsigned char * text_string_left_dollar = (unsigned char *) malloc((text_size + 2)*sizeof(unsigned char));
    for (INT i = 0; i < text_size; ++i) {
        text_string_left_dollar[i] = textStringWoDollar[i];
    }
    text_string_left_dollar[text_size] = '$';
    text_string_left_dollar[text_size + 1] = '\0';
    return text_string_left_dollar;
}
unsigned char* reverse(unsigned char* &s) {
    INT length = strlen((char*) s);
    unsigned char* reversed_s = ( unsigned char * ) malloc (  ( length + 1 ) * sizeof ( unsigned char ) );


    for (INT i = 0; i < length; i++) {
        reversed_s[i] = s[length - 1 - i];
    }
    reversed_s[length] = '\0'; // Don't forget to null-terminate the new string

    return reversed_s; // Return the new dynamically allocated reversed string
}


unsigned char* reverseString(unsigned char* &s) {
    INT length = strlen((char*) s);
    unsigned char* reversed_s = (unsigned char *) malloc((length + 1) * sizeof(unsigned char));
    for (INT i = 0; i < length - 1; i++) {
        reversed_s[i] = s[length - i - 2];
    }
    reversed_s[length - 1] = '$';
    reversed_s[length] = '\0';
    return reversed_s;
}


// Compute leaf counts for every node in the suffix tree (iterative post-order DFS)
void computeLeafCounts(stNode* root) {
    stack<pair<stNode*, bool>> st;
    st.push({root, false});
    while (!st.empty()) {
        auto &[node, processed] = st.top();
        if (!processed) {
            processed = true;
            for (auto it = node->child.rbegin(); it != node->child.rend(); ++it) {
                st.push({it->second, false});
            }
        } else {
            st.pop();
            if (node->child.empty()) {
                node->leafCount = 1;
            } else {
                node->leafCount = 0;
                for (auto &kv : node->child) {
                    node->leafCount += kv.second->leafCount;
                }
            }
        }
    }
}


// Compute deepPtr for every node: pointer to the deepest tau-frequent descendant
// (including the node itself if it is tau-frequent)
void computeDeepPtrs(stNode* root, INT tau) {
    stack<pair<stNode*, bool>> st;
    st.push({root, false});
    while (!st.empty()) {
        auto &[node, processed] = st.top();
        if (!processed) {
            processed = true;
            for (auto it = node->child.rbegin(); it != node->child.rend(); ++it) {
                st.push({it->second, false});
            }
        } else {
            st.pop();
            node->deepPtr = nullptr;
            // Gather best deepPtr from children
            for (auto &kv : node->child) {
                stNode* cp = kv.second->deepPtr;
                if (cp && (!node->deepPtr || cp->depth > node->deepPtr->depth)) {
                    node->deepPtr = cp;
                }
            }
            // If this node itself is tau-frequent and deeper than current deepPtr, use it
            if (node->leafCount >= tau) {
                if (!node->deepPtr || node->depth > node->deepPtr->depth) {
                    node->deepPtr = node;
                }
            }
        }
    }
}


int main(int argc, char * argv[]){

    cmdline::parser parser;
    parser.add<string>("filePath",    'f', "the path to input file",    false, "input.txt");
    parser.add<string>("patternPath", 'p', "the path to pattern file",  false, "patterns.txt");
    parser.add<int>   ("tau",         't', "the support",               false, 1);
    parser.add        ("visualize",   'v', "export suffix trees to DOT files after computeDeepPtrs");

    parser.parse_check(argc, argv);

    string filePath    = parser.get<string>("filePath");
    string patternPath = parser.get<string>("patternPath");
    INT tau            = parser.get<int>("tau");
    bool visualize     = parser.exist("visualize");

    unsigned char *textStringWoDollar;
    std::vector<unsigned char *> patterns;
    INT text_size    = 0;
    INT alphabetSize = 0;
    vector<INT> patternSizes;

    readfile_woDollar(filePath, patternPath, textStringWoDollar, patterns, text_size, alphabetSize, patternSizes);

    unsigned char *textFwd = addDollar(textStringWoDollar, text_size);
    unsigned char *textRev = reverseString(textFwd);

    INT n = text_size + 1;  // length including '$'

    auto start = std::chrono::high_resolution_clock::now();

    long long memStart = memory_usage();

    // Build forward and reverse suffix trees
    suffixTree ST_fwd(textFwd, n);
    suffixTree ST_rev(textRev, n);

    // Compute leaf counts and deep pointers for both trees
    computeLeafCounts(ST_fwd.root);
    computeLeafCounts(ST_rev.root);
    computeDeepPtrs(ST_fwd.root, tau);
    computeDeepPtrs(ST_rev.root, tau);

    if (visualize) {
        ST_fwd.exportSuffixTreeToDot("tree_fwd.dot", tau);
        ST_rev.exportSuffixTreeToDot("tree_rev.dot", tau);
    }

    auto end_construction = std::chrono::high_resolution_clock::now();
    long long memEnd = memory_usage();

    double construction_time = std::chrono::duration_cast<std::chrono::microseconds>(
        end_construction - start).count() * 1e-6;
    double index_memory_MB = (memEnd - memStart) * 1e-6;

    cout << "=== LFCS Baseline (Suffix Tree) ===" << endl;
    cout << "Tau: " << tau << endl;
    cout << "Construction time: " << construction_time << " s" << endl;
    cout << "Index memory: " << index_memory_MB << " MB" << endl;

    cout << "=== Start to query patterns: LFCS Baseline ===" << endl;

    for (INT i = 0; i < (INT)patterns.size(); i++) {

        auto queryStart = std::chrono::high_resolution_clock::now();

        unsigned char *P = patterns[i];
        INT m = patternSizes[i];

        // Step 1: locate locus of P in forward suffix tree
        stNode *vP = ST_fwd.forward_search(P, m);

        if (!vP) {
            cout << "Pattern " << i << ": " << P << endl;
            cerr << "The pattern does not exist in text string!" << endl;
            cout << "---------------------------------------" << endl;
            continue;
        }

        // Step 2: DFS of vP's subtree, collect tau-frequent nodes u (right extensions PR)
        // For each such u, look up rev(PR) in reverse tree and compute max LPR length
        INT best_len = -1;

        stack<stNode*> dfs;
        dfs.push(vP);

        while (!dfs.empty()) {
            stNode* u = dfs.top();
            dfs.pop();

            // Pruning: skip subtree if not tau-frequent
            if (u->leafCount < tau) continue;

            // This node u is tau-frequent: it represents a right extension PR
            INT len_PR=0;
            INT R_len = 0;
            if (u->child.empty()){
                len_PR = u->depth - 1;  // leaf
                R_len  = len_PR - m;                // |R| = |PR| - |P|

            }else{
                len_PR = u->depth ; // internal node
                R_len  = len_PR - m;                // |R| = |PR| - |P|
            }

            if (R_len >= 0) {
                // Construct rev(PR) from T[u->start .. u->start + len_PR - 1]
                unsigned char* revPR = (unsigned char*) malloc((len_PR + 1) * sizeof(unsigned char));
                for (INT k = 0; k < len_PR; k++) {
                    // cout<<u->start + len_PR - 1 - k<<endl;
                    // cout<<textFwd[u->start + len_PR - 1 - k]<<endl;
                    revPR[k] = textFwd[u->start + len_PR - 1 - k];
                }
                revPR[len_PR] = '\0';

                // Step 3: find locus of rev(PR) in reverse suffix tree
                stNode* vRev = ST_rev.forward_search(revPR, len_PR);
                free(revPR);

                if (vRev) {
                    stNode* uRev = vRev->deepPtr;
                    if (uRev) {

                        INT d_rev = 0;
                        
                        if (uRev->child.empty()){
                            d_rev = uRev->depth -1;
                        }else{
                            d_rev = uRev->depth;

                        }
                        
                        
                        INT L_prime = d_rev - len_PR;  // max left extension length
                        if (L_prime >= 0) {
                            INT s = min(L_prime, R_len);
                            INT lpr_len = m + 2 * s;
                            if (lpr_len > best_len) {
                                best_len = lpr_len;
                            }
                        }
                    }
                }
            }

            // Push children for further DFS
            for (auto &kv : u->child) {
                dfs.push(kv.second);
            }
        }

        // ── Second pass: locus of P^R in ST(T^R), searching in ST(T) ─────────
        // Symmetric to the first pass (Lemma: at least one of the two loci is explicit).
        unsigned char* P_rev = (unsigned char*)malloc((m + 1) * sizeof(unsigned char));
        for (INT k = 0; k < m; k++) P_rev[k] = P[m - 1 - k];
        P_rev[m] = '\0';

        stNode* vP_rev = ST_rev.forward_search(P_rev, m);
        free(P_rev);

        if (vP_rev) {
            stack<stNode*> dfs2;
            dfs2.push(vP_rev);
            while (!dfs2.empty()) {
                stNode* u2 = dfs2.top(); dfs2.pop();
                if (u2->leafCount < tau) continue;

                // u2 represents a left extension (LP)^R in T^R
                INT len_LP_rev = u2->child.empty() ? u2->depth - 1 : u2->depth;
                INT L_len = len_LP_rev - m;

                if (L_len >= 0) {
                    // Construct LP = reverse of (LP)^R stored in textRev
                    unsigned char* LP = (unsigned char*)malloc((len_LP_rev + 1) * sizeof(unsigned char));
                    for (INT k = 0; k < len_LP_rev; k++)
                        LP[k] = textRev[u2->start + len_LP_rev - 1 - k];
                    LP[len_LP_rev] = '\0';

                    // Search LP in ST(T) and use deepPtr to find max right extension R
                    stNode* vFwd2 = ST_fwd.forward_search(LP, len_LP_rev);
                    free(LP);

                    if (vFwd2) {
                        stNode* uFwd2 = vFwd2->deepPtr;
                        if (uFwd2) {
                            INT d_fwd = uFwd2->child.empty() ? uFwd2->depth - 1 : uFwd2->depth;
                            INT R_prime = d_fwd - len_LP_rev;
                            if (R_prime >= 0) {
                                INT s = min(R_prime, L_len);
                                INT lpr_len = m + 2 * s;
                                if (lpr_len > best_len) best_len = lpr_len;
                            }
                        }
                    }
                }

                for (auto& kv : u2->child) dfs2.push(kv.second);
            }
        }

        auto queryEnd = std::chrono::high_resolution_clock::now();
        double query_time = std::chrono::duration_cast<std::chrono::microseconds>(
            queryEnd - queryStart).count() * 1e-6;

        cout << "Pattern " << i << ": " << P << endl;
        cout << "occ_T(P): " << vP->leafCount << endl;
        if (best_len < 0) {
            cerr << "P exists in text string but LPR is not " << tau << "-frequent" << endl;
        } else {
            cout << "LPR length: " << best_len << endl;
        }
        cout << "Query time: " << query_time << " s" << endl;
        cout << "---------------------------------------" << endl;
    }

    // Cleanup
    free(textFwd);
    free(textRev);
    free(textStringWoDollar);
    for (auto &it : patterns) {
        free(it);
    }

    return 0;
}

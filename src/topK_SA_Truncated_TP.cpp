#include <iostream>
#include "cmdline.h"
#include <string>
#include <fstream>
#include <unordered_set>
#include <string>
#include <vector>
#include <malloc.h>
#include <cmath>
#include <algorithm>    // std::sort
#include <cstring>
#include <chrono>
#include <sdsl/rmq_support.hpp>					  //include header for range minimum queries
#include "compactTrieTruncated.h"
#include "SA_LCP_LCE.h"
#include "esaTruncated_TP.h"

using namespace std;



#include <malloc.h>

long long memory_usage() {
    struct mallinfo2 mi = mallinfo2();
    return mi.uordblks + mi.hblkhd;
}

// read context length and k size from file

void readfile_woMax(string &filename, string &patternPath, unsigned char * &text_string_woMax, std::vector<unsigned char *> &patterns,
                    std::vector<INT> &contextSizes, INT& text_size, INT& alphabetSize, vector<INT> & patternSizes, vector<INT> & kSizes, INT & B){
    std::ifstream is_text(filename, std::ios::binary);
    if (!is_text) {
        std::cerr << "Error opening input file: " << filename << std::endl;
        return;
    }

    is_text.seekg(0, std::ios::end);
    text_size = is_text.tellg();
    is_text.seekg(0, std::ios::beg);
    unordered_set<unsigned char> alphabet;

//    text_string = (unsigned char *)malloc((text_size + 3) * sizeof(unsigned char));
//    text_string[0] = '$';
    text_string_woMax = (unsigned char *)malloc((text_size + 1) * sizeof(unsigned char));


    char c = 0;
    for (INT i = 1; i < text_size + 1; i++) {
        is_text.read(reinterpret_cast<char *>(&c), 1);
//        text_string[i] = (unsigned char)c;
        text_string_woMax[i-1] = (unsigned char)c;

        alphabet.insert((unsigned char)c);

    }

    is_text.close();
//    text_string[text_size +1 ] = '$';
    alphabet.insert(255);

//    text_string[text_size +2] = '\0';
    text_string_woMax[text_size] = '\0';

//    text_size  = text_size ;

    alphabetSize = alphabet.size();


    /* open pattern file and write it into pattern*/
    std::ifstream is_pattern(patternPath, std::ios::binary);
    if (!is_pattern) {
        std::cerr << "Error opening pattern file: " << patternPath << std::endl;
    }


    std::string line;
    INT B_tmp=0;

    while (std::getline(is_pattern, line)) {
        std::istringstream iss(line);
        std::string pattern_str;
        INT l;
        INT r;
        // we let l =r
        INT k;
        if (!(iss >> pattern_str >> l>>r >> k)) {
            std::cerr << "Error reading line: " << line << std::endl;
            continue;  // Skip the invalid line
        }

        // Allocate memory for the pattern and store it in the vector
        unsigned char *pattern = (unsigned char *)malloc((pattern_str.size() + 1) * sizeof(unsigned char));
        std::copy(pattern_str.begin(), pattern_str.end(), pattern);
        pattern[pattern_str.size()] = '\0';  // null terminator
        patternSizes.push_back(pattern_str.size());
        patterns.push_back(pattern);
        contextSizes.push_back({l});
        kSizes.push_back(k);


        B_tmp = l+r + pattern_str.size();
        if (B_tmp > B){
            B = B_tmp;
        }
    }

    is_pattern.close();
}



// read context length from file

void readfile_woMax(string &filename, string &patternPath, unsigned char * &text_string_woMax, std::vector<unsigned char *> &patterns,
                    std::vector<INT> &contextSizes, INT& text_size, INT& alphabetSize, vector<INT> & patternSizes){
    std::ifstream is_text(filename, std::ios::binary);
    if (!is_text) {
        std::cerr << "Error opening input file: " << filename << std::endl;
        return;
    }

    is_text.seekg(0, std::ios::end);
    text_size = is_text.tellg();
    is_text.seekg(0, std::ios::beg);
    unordered_set<unsigned char> alphabet;

//    text_string = (unsigned char *)malloc((text_size + 3) * sizeof(unsigned char));
//    text_string[0] = '$';
    text_string_woMax = (unsigned char *)malloc((text_size + 1) * sizeof(unsigned char));


    char c = 0;
    for (INT i = 1; i < text_size + 1; i++) {
        is_text.read(reinterpret_cast<char *>(&c), 1);
//        text_string[i] = (unsigned char)c;
        text_string_woMax[i-1] = (unsigned char)c;

        alphabet.insert((unsigned char)c);

    }

    is_text.close();
//    text_string[text_size +1 ] = '$';
    alphabet.insert(255);

//    text_string[text_size +2] = '\0';
    text_string_woMax[text_size] = '\0';

//    text_size  = text_size ;

    alphabetSize = alphabet.size();


    /* open pattern file and write it into pattern*/
    std::ifstream is_pattern(patternPath, std::ios::binary);
    if (!is_pattern) {
        std::cerr << "Error opening pattern file: " << patternPath << std::endl;
    }


    std::string line;
    while (std::getline(is_pattern, line)) {
        std::istringstream iss(line);
        std::string pattern_str;
        INT l;

        if (!(iss >> pattern_str >> l>>l)) {
            std::cerr << "Error reading line: " << line << std::endl;
            continue;  // Skip the invalid line
        }

        // Allocate memory for the pattern and store it in the vector
        unsigned char *pattern = (unsigned char *)malloc((pattern_str.size() + 1) * sizeof(unsigned char));
        std::copy(pattern_str.begin(), pattern_str.end(), pattern);
        pattern[pattern_str.size()] = '\0';  // null terminator
        patternSizes.push_back(pattern_str.size());
        patterns.push_back(pattern);
        contextSizes.push_back({l});
    }

    is_pattern.close();
}



// read context length from cmd

void readfile_woMax(string &filename, string &patternPath,unsigned char * &text_string_woMax, std::vector<unsigned char *> &patterns,
                    std::vector<INT> &contextSizes, INT& text_size, INT& alphabetSize, vector<INT> & patternSizes, INT l){
    std::ifstream is_text(filename, std::ios::binary);
    if (!is_text) {
        std::cerr << "Error opening input file: " << filename << std::endl;
        return;
    }

    is_text.seekg(0, std::ios::end);
    text_size = is_text.tellg();
    is_text.seekg(0, std::ios::beg);
    unordered_set<unsigned char> alphabet;

//    text_string = (unsigned char *)malloc((text_size + 3) * sizeof(unsigned char));
//    text_string[0] = '$';
    text_string_woMax = (unsigned char *)malloc((text_size + 1) * sizeof(unsigned char));

    char c = 0;
    for (INT i = 1; i < text_size + 1; i++) {
        is_text.read(reinterpret_cast<char *>(&c), 1);
//        text_string[i] = (unsigned char)c;
        text_string_woMax[i-1] = (unsigned char)c;
        alphabet.insert((unsigned char)c);

    }

    is_text.close();
//    text_string[text_size +1 ] = '$';
    alphabet.insert(255);

//    text_string[text_size +2] = '\0';

    text_string_woMax[text_size] = '\0';

    text_size  = text_size ;

    alphabetSize = alphabet.size();


    /* open pattern file and write it into pattern*/
    std::ifstream is_pattern(patternPath, std::ios::binary);
    if (!is_pattern) {
        std::cerr << "Error opening pattern file: " << patternPath << std::endl;
    }


    std::string line;
    while (std::getline(is_pattern, line)) {
        std::istringstream iss(line);
        std::string pattern_str;
        INT useless_l;

        if (!(iss >> pattern_str >> useless_l>>useless_l)) {
            std::cerr << "Error reading line: " << line << std::endl;
            continue;  // Skip the invalid line
        }

        // Allocate memory for the pattern and store it in the vector
        unsigned char *pattern = (unsigned char *)malloc((pattern_str.size() + 1) * sizeof(unsigned char));
        std::copy(pattern_str.begin(), pattern_str.end(), pattern);
        pattern[pattern_str.size()] = '\0';  // null terminator
        patternSizes.push_back(pattern_str.size());
        patterns.push_back(pattern);
        contextSizes.push_back({l});
    }

    is_pattern.close();
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
    unsigned char* reversed_s = ( unsigned char * ) malloc (  ( length + 1 ) * sizeof ( unsigned char ) );


    for (INT i = 0; i < length - 1; i++) {
        reversed_s[i] = s[length  - i -2];
    }
    reversed_s[length - 1] = 255; // Don't forget to null-terminate the new string

    reversed_s[length] = '\0'; // Don't forget to null-terminate the new string

    return reversed_s; // Return the new dynamically allocated reversed string
}



//void generateZigZagArray(unsigned char * &T, std::vector<pair<unsigned char *, INT>> &zArray, INT text_size) {
//    for (INT i = 0; i < text_size; i++) {
//        cout<<i<<endl;
//        unsigned char* zi = new unsigned char[text_size + 2]; // Extra space for '$' and null-terminator
//        INT index = 0;
//        INT cnt = 0;
//        INT index_T = i;
//        while (index_T < text_size && index_T >= 0) {
//
//            zi[index] = T[index_T];
//            index++;
//            if(index % 2 == 0){
//                index_T = i - cnt;
//            }else{
//                cnt ++;
//                index_T = i + cnt;
//            }
//        }
//
//        zi[index++] = '$'; // Append '$'
//        zi[index] = '\0';  // Null-terminate the string
//        zArray.push_back({zi, i});
//
//    }
//}






unsigned char* generateZP(unsigned char* P, INT m) {
    unsigned char* ZP = new unsigned char[m + 1];
    INT x = ceil((float) m / 2 - 1); // Midpoint of pattern
    INT index = 0;
    INT cnt = 0;
    INT index_P = x;
    while (index_P < m && index_P >= 0) {

        ZP[index] = P[index_P];
        index++;
        if(index % 2 == 0){
            index_P = x - cnt;
        }else{
            cnt ++;
            index_P = x + cnt;
        }
    }

    ZP[index] = '\0'; // Null-terminate
    return ZP;
}


INT compareZStrings(INT i, INT j, SA_LCP_LCE& DS_org, SA_LCP_LCE& DS_rev, INT B) {

//    cout<<"DS_rev.text_size - i - 2: "<<DS_rev.text_size - i - 2<<"; DS_rev.text_size - j - 2:"<<DS_rev.text_size - j - 2<<endl;

    INT r = DS_rev.LCE(DS_rev.text_size - i - 2, DS_rev.text_size - j - 2); // even positions
//    cout<<"i+1: "<<i +1<<"; j+1: "<< j +1<<endl;

    INT f = DS_org.LCE(i + 1, j + 1);                                       // odd positions

    if (r <= f) {
        if (2 * r + 1 > B) {
            return B;
        } else {
            return 2 * r;
        }
    } else {
        // 2f + 1 is an index, so add 1 to get the length
        if (2 * f + 2 > B) {
            return B;
        } else {
            return 2 * f + 1;
        }

    }
}


bool isZLess(INT i, INT j, SA_LCP_LCE& DS_org, SA_LCP_LCE& DS_rev, unsigned char* T, INT text_size, INT B) {
    INT mismatch_pos = compareZStrings(i, j, DS_org, DS_rev, B);
    unsigned char ci = getZigZagChar(i, mismatch_pos, T, text_size, B);
    unsigned char cj = getZigZagChar(j, mismatch_pos, T, text_size, B);
    return ci <= cj;
}




void mergeZ(INT* indices, INT left, INT mid, INT right,
            std::vector<INT>& temp, SA_LCP_LCE& DS_org, SA_LCP_LCE& DS_rev,
            unsigned char* T, INT text_size, INT B) {

    INT i = left, j = mid, k = left;
    while (i < mid && j < right) {
        if (isZLess(indices[i], indices[j], DS_org, DS_rev, T, text_size, B)) {
            temp[k++] = indices[i++];
        } else {
            temp[k++] = indices[j++];
        }
    }

    while (i < mid)  temp[k++] = indices[i++];
    while (j < right) temp[k++] = indices[j++];

    for (INT t = left; t < right; ++t){
        indices[t] = temp[t];
    }
}



void mergeSortIterativeZigZag(INT * indices, unsigned char* T, INT text_size,
                              SA_LCP_LCE& DS_org, SA_LCP_LCE& DS_rev, INT* LCP, INT B){
    vector<INT> temp(text_size);


    for (INT width = 1; width < text_size; width *= 2) {
//        cout<<width<<endl;
        for (INT i = 0; i < text_size; i += 2 * width) {
            INT left = i;
            INT mid = min(i + width, text_size);
            INT right = min(i + 2 * width, text_size);
//            cout<<"left: "<<left<<"; mid: "<<mid<<"; right: "<<right<<endl;
            if (mid < right) {
                mergeZ(indices, left, mid, right, temp, DS_org, DS_rev, T, text_size, B);
            }
        }
    }

    // Build LCP

    LCP[0] = 0;
    for (INT i = 1; i < text_size; ++i) {
        LCP[i] = compareZStrings(indices[i - 1], indices[i], DS_org, DS_rev,B);
    }


}

INT compareZStrings_direct(INT i, INT j, unsigned char* T, INT text_size, INT B) {
    for (INT d = 0; d < B; d++) {
        unsigned char ci = getZigZagChar(i, d, T, text_size, B);
        unsigned char cj = getZigZagChar(j, d, T, text_size, B);
        if (ci != cj) return d;
    }
    return B;
}

bool isZLess_direct(INT i, INT j, unsigned char* T, INT text_size, INT B) {
    INT d = compareZStrings_direct(i, j, T, text_size, B);
    return getZigZagChar(i, d, T, text_size, B) <= getZigZagChar(j, d, T, text_size, B);
}

void mergeZ_direct(INT* indices, INT left, INT mid, INT right,
                   vector<INT>& temp, unsigned char* T, INT text_size, INT B) {
    INT i = left, j = mid, k = left;
    while (i < mid && j < right) {
        if (isZLess_direct(indices[i], indices[j], T, text_size, B))
            temp[k++] = indices[i++];
        else
            temp[k++] = indices[j++];
    }
    while (i < mid)  temp[k++] = indices[i++];
    while (j < right) temp[k++] = indices[j++];
    for (INT t = left; t < right; ++t) indices[t] = temp[t];
}

void mergeSortIterativeZigZag_direct(INT* indices, unsigned char* T, INT text_size, INT* LCP, INT B) {
    vector<INT> temp(text_size);
    for (INT width = 1; width < text_size; width *= 2) {
        for (INT i = 0; i < text_size; i += 2 * width) {
            INT left = i;
            INT mid = min(i + width, text_size);
            INT right = min(i + 2 * width, text_size);
            if (mid < right)
                mergeZ_direct(indices, left, mid, right, temp, T, text_size, B);
        }
    }
    LCP[0] = 0;
    for (INT i = 1; i < text_size; ++i)
        LCP[i] = compareZStrings_direct(indices[i - 1], indices[i], T, text_size, B);
}

void printArray(const char* name, INT* array, INT size) {
    std::cout << name << ": [";
    for (INT i = 0; i < size; i++) {
        std::cout << array[i];
        if (i < size - 1) {
            std::cout << ", ";
        }
    }
    std::cout << "]" << std::endl;
}

void printstring(const char* name, INT* array, INT size,  unsigned char* text) {
    std::cout << name << ":"<<endl;
    for (INT i = 0; i < size; i++) {
        for (INT j =array[i];j< size;j++){
            std::cout << text[j];
        }
        if (i < size - 1) {
            std::cout <<endl;
        }
    }
    std::cout << std::endl;
}


unsigned char * addMax(unsigned char * textStringWoMax,INT text_size){
    unsigned char * text_string_left_Max = (unsigned char *) malloc((text_size + 2)*sizeof(unsigned char));

    for (INT i = 0; i < text_size; ++i) {
        text_string_left_Max[i] = textStringWoMax[i];
    }
    text_string_left_Max [text_size] = 255;
    text_string_left_Max[text_size +1] ='\0';
    return text_string_left_Max;
}




static inline void build_id2right_index(const std::vector<B>& b,
                                        std::vector<pair<INT, INT>>& id2right)
{
    id2right.clear();
    id2right.reserve(b.size());
    for (size_t i = 0; i < b.size(); ++i) {
        if (b[i].id >= 0 && b[i].right_node >= 0) {
            INT id      = (INT)b[i].id;
            INT rightId = (INT)b[ b[i].right_node ].id;
            id2right.emplace_back(id, rightId);
        }
    }

    std::sort(id2right.begin(), id2right.end(),
              [](const pair<INT, INT>& a, const pair<INT, INT>& b){ return a.first < b.first; });


}

static inline bool next_greater_id_with_right(INT x,
                                              const std::vector<pair<INT, INT>>& id2right,
                                              INT& out_id,
                                              INT& out_right)
{
    auto it = std::upper_bound(
            id2right.begin(), id2right.end(), x,
            [](INT value, const pair<INT, INT>& elem){ return value < elem.first; } // Find the first elem.first > x
    );
    if (it == id2right.end()) return false;
    out_id = it->first;
    out_right = it->second;
    return true;
}

static inline bool get_right_if_exists(INT x,
                                       const std::vector<pair<INT, INT>>& id2right,
                                       INT& out_right)
{
    auto it = std::lower_bound(
            id2right.begin(), id2right.end(), x,
            [](const pair<INT, INT>& elem, INT value){ return elem.first < value; }
    );
    if (it != id2right.end() && it->first == x) {
        out_right = it->second;
        return true;
    }
    return false;
}


int main(int argc, char * argv[]) {

    cmdline::parser parser;
    parser.add<string>("filePath", 'f', "the path to input file", false, "input.txt");
    parser.add<string>("patternPath", 'p', "the path to pattern file", false, "patterns.txt");
    parser.add<int>("L", 'l', "the length of context ", false, -1);
    parser.add<int>("K", 'k', "top k ", false, -1);
    parser.add("bottom", 'b', "select bottom k instead of top k");


    parser.parse_check(argc, argv);

    string filePath = parser.get<string>("filePath");

    string patternPath = parser.get<string>("patternPath");

    // if l,k are not specified (negative), use the default value in the pattern file
    INT l = parser.get<int>("L");
    INT k = parser.get<int>("K");

    // if bottom is specified, use bottom k instead of top k
    bool bottomMode = parser.exist("bottom");
    unsigned char *textStringWoMax;


    /* readfile into text_string and pattern */
//    unsigned char *text_string_woMax;

    std::vector<unsigned char *> patterns;
    std::vector<INT> contextSizes;

    INT text_size = 0;
    INT alphabetSize = 0;
    vector<INT> patternSizes;

    vector<INT> kSizes;

    INT Bound = 0;
    readfile_woMax(filePath, patternPath, textStringWoMax, patterns, contextSizes, text_size, alphabetSize, patternSizes, kSizes, Bound);


    if (l > 0) {
        INT Bound_tmp = 0;
        INT Bound_new = 0 ;
        for (INT i = 0; i < contextSizes.size(); i++) {
            contextSizes[i] = l;
            Bound_tmp = l + l + patternSizes[i];
            if (Bound_tmp > Bound_new){
                Bound_new = Bound_tmp;
            }
        }
        Bound = Bound_new;
    }
    if (k > 0) {
        for (INT i = 0; i < kSizes.size(); i++) {
            kSizes[i] = k;
        }
    }
    cout<< "B = "<< Bound <<endl;

    unsigned char *textStringWLeftMax = addMax(textStringWoMax, text_size);
    unsigned char *textStringWLeftMax_rev = reverseString(textStringWLeftMax);


    long long IndexSpace_start = memory_usage();



    /*Prepared SA, invSA, LCP, LCE, rmq for the original string*/
    INT text_size_oneMax = text_size + 1;


    auto start = std::chrono::high_resolution_clock::now();

    // construct SA and LCP for Z


    INT *SA_Z = (INT *) malloc((text_size) * sizeof(INT));

    for (INT i = 0; i < text_size; ++i) {
        SA_Z[i] = i;

    }


    INT *LCP_Z = (INT *) malloc((text_size) * sizeof(INT));


    rmq_succinct_sct<> rmq;


#ifdef USE_DIRECT_COMPARE
    mergeSortIterativeZigZag_direct(SA_Z, textStringWoMax, text_size, LCP_Z, Bound);
    {
#else
    {
        SA_LCP_LCE DS_org(textStringWLeftMax, text_size_oneMax);

        /*Prepared SA_rev, invSA_rev, LCP_rev, LCE_rev, rmq_rev*/

        SA_LCP_LCE DS_rev(textStringWLeftMax_rev, text_size_oneMax);
        // mergeStart = std::chrono::high_resolution_clock::now();

        mergeSortIterativeZigZag(SA_Z, textStringWoMax, text_size, DS_org, DS_rev, LCP_Z, Bound);
#endif

        // mergeEnd = std::chrono::high_resolution_clock::now();

        int_vector<> v(text_size, 0); // create a vector of length n and initialize it with 0s

        for (INT i = 0; i < text_size; i++) {
            v[i] = LCP_Z[i];
        }


        util::assign(rmq, rmq_succinct_sct<>(&v));

#ifdef VERBOSE
        printArray("SA", DS_org.SA, text_size_oneMax);
    printArray("invSA", DS_org.invSA, text_size_oneMax);

    printArray("LCP", DS_org.LCP, text_size_oneMax);

    printstring("SA->string",DS_org.SA, text_size_oneMax, DS_org.T);


    printArray("SA reverse", DS_rev.SA, text_size_oneMax);
    printArray("invSA reverse", DS_rev.invSA, text_size_oneMax);

    printArray("LCP reverse", DS_rev.LCP, text_size_oneMax);
    printstring("SA->string reverse",DS_rev.SA, text_size_oneMax, DS_rev.T);

#endif

    }


#ifdef VERBOSE


    cout << "SA_Z: ";
    for (INT i = 0; i < text_size; ++i) cout << SA_Z[i] << " ";
    cout << endl;

    cout << "LCP_Z: ";
    for (INT i = 0; i < text_size; ++i) cout << LCP_Z[i] << " ";
    cout << endl;
#endif
// from id to rightmost id and min gap
    unordered_map<uint64_t, std::pair<uint64_t, INT>> id2right;
#ifdef USE_3D
    using scored_point3 = std::pair<point3, uint64_t>;
    std::vector<scored_point3> points;
#else
    std::vector<point4> points;
#endif


    // for later use
    auto tuple_start = std::chrono::high_resolution_clock::now();
    auto tuple_end = std::chrono::high_resolution_clock::now();

    {
        vector<B> b(text_size_oneMax);

//        cout<<b.size()<<endl;
        b.reserve(2*text_size_oneMax-1);
//        cout<<b.size()<<endl;

        tuple_start = std::chrono::high_resolution_clock::now();


        construct_tuples(text_size_oneMax, SA_Z, b, LCP_Z, Bound);
        tuple_end = std::chrono::high_resolution_clock::now();

        //    cout<<b.size()<<endl;

//    build_id2right_index(b, id2right);

//id2right: from id to rightmost id and min gap
        for (int i = 0; i < b.size(); i++) {

            if (b[i].id>0){
                id2right[b[i].id] = {b[b[i].right_node].id, b[i].min_gap};

            }

        }



#ifdef VERBOSE
//        cout << "The tuples before are " << endl;
//     for (int i = 0; i < b.size(); i++) {
//       cout << "i=" << i << ", l=" << b[i].l << ", r=" << b[i].r << ", lcp=" << b[i].lcp  <<", id = "<< b[i].id << ", parent=" << b[i].parent <<", # of children:"<< b[i].leaf_cnt <<", rightmost leaf id: "<<b[i].right_node<<"Min gap"<<b[i].min_gap <<", children are: "<<endl;
////       for (const auto &child: b[i].ch) {
////         cout << child << ",";
////       }
////       cout << endl;
//     }

    cout << "\n====================  All Nodes in B  ====================\n";
    for (size_t i = 0; i < b.size(); i++) {
        const B& node = b[i];
        cout << "Node #" << i << ":\n";
        cout << "  id         = " << node.id << "\n";
        cout << "  l, r, lcp  = (" << node.l << ", " << node.r << ", " << node.lcp << ")\n";
        cout << "  parent     = " << node.parent << "\n";
//        cout << "  leaf_cnt   = " << node.leaf_cnt << "\n";
        cout << "  right_node = " << node.right_node << "\n";
        cout << "  min_gap    = ";
        if (node.min_gap == INT_MAX) cout << "INF";
        else cout << node.min_gap;
        cout << "\n";

        cout << "  occ (size=" << node.occ.size() << "): ";
        if (!node.occ.empty()) {
            for (size_t j = 0; j < node.occ.size(); j++) {
                cout << node.occ[j];
                if (j + 1 < node.occ.size()) cout << ",";
            }
        } else {
            cout << "(empty)";
        }
        cout << "\n-----------------------------------------------------------\n";
    }


#endif

        for (int i = 0; i < b.size(); i++) {

            if (b[i].parent != -1){
#ifdef USE_3D
                point3 tuple_3D;
                bg::set<0>(tuple_3D, (uint64_t) b[i].id);
                bg::set<1>(tuple_3D, (uint64_t) b[i].lcp);
                bg::set<2>(tuple_3D, (uint64_t) b[b[i].parent].lcp + 1);
                points.emplace_back(tuple_3D, (uint64_t) b[i].min_gap);
#else
                point4 tuple_4D;
                bg::set<0>(tuple_4D, (uint64_t) b[i].id);
                bg::set<1>(tuple_4D, (uint64_t) b[i].lcp);
                bg::set<2>(tuple_4D, (uint64_t) b[b[i].parent].lcp + 1);
                bg::set<3>(tuple_4D, (uint64_t) b[i].min_gap);
                points.emplace_back(tuple_4D);
#endif
            }

        }


    }



// for all the point in SA_Z and LCP_Z: add all the nodes into points








    // Here are the points that will be used to build the R*-tree

// Step 4: build R-tree
    long long Rtree_start = memory_usage();

#ifdef USE_3D
    bgi::rtree<scored_point3, bgi::rstar<32>> RT(points);
#else
    bgi::rtree<point4, bgi::rstar<32>> RT(points);
#endif
    long long Rtree_end = memory_usage();


    auto end = std::chrono::high_resolution_clock::now();


    cout<<"Size of the points: "<<points.size()<<endl;


    // clear the points
    points.clear();
    points.shrink_to_fit();

#ifdef USE_3D
    std::vector<scored_point3>().swap(points);
#else
    std::vector<point4>().swap(points);
#endif

    long long IndexSpace_end = memory_usage();
    long long memory_Rtree = Rtree_end - Rtree_start;

    long long memory_Index = IndexSpace_end - IndexSpace_start;


    double ZZT_time = std::chrono::duration_cast<std::chrono::microseconds>(tuple_end - start).count() * 0.000001;
    double Construction_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() * 0.000001;

    cout<<"====================================== Zigzag Suffix Array Version Index Construction=============================="<<endl;

    cout<<"ZZT construction time: "<< ZZT_time<< " seconds"<<endl;
    cout<<"Total construction time: "<< Construction_time<< " seconds"<<endl;

    cout<<"R* tree Index memory: "<< memory_Rtree / (1024.0 * 1024.0)<<"MB"<<endl;

    cout<<"Index memory: "<< memory_Index / (1024.0 * 1024.0)<<"MB"<<endl;





    cout << "=========================== Start to query all the patterns: Top K ==========================="<< endl;


    for (INT i = 0; i < patterns.size(); i++) {
        auto queryStart = std::chrono::high_resolution_clock::now();

        INT contextSize = contextSizes[i];
        INT kSize = kSizes[i];
        auto time1 = std::chrono::high_resolution_clock::now();

        unsigned char *ZP = generateZP(patterns[i], patternSizes[i]);
        auto time2 = std::chrono::high_resolution_clock::now();

        // ZP size = P size
        pair<INT, INT> intervalP = pattern_matching_Z(ZP,textStringWoMax,SA_Z,LCP_Z,rmq, text_size,patternSizes[i], Bound);
        auto time3 = std::chrono::high_resolution_clock::now();

        if (intervalP.second < intervalP.first){
            std::cout << "Pattern " << i << ": " << patterns[i] << std::endl;
            std::cout << "l: " << contextSize << std::endl;



            std::cerr << "The pattern does not exist in text string!" << std::endl;
            cout << "---------------------------------------" << endl;

            delete[] ZP;
            continue;
        }
//        cout << "Interval: [" << intervalP.first << ", " << intervalP.second << "]" << endl;


        INT id_P = 0;
        if (intervalP.second > intervalP.first){
            INT LCP_P_ix = rmq(intervalP.first+1, intervalP.second);
            INT LCP_P = LCP_Z[LCP_P_ix];
            id_P = (uint64_t) intervalP.first * (uint64_t) text_size_oneMax + (uint64_t) LCP_P ;

        }else{

            INT LCP_P = zigzag_length(SA_Z[intervalP.first],text_size_oneMax,Bound);
            id_P = (uint64_t) intervalP.first * (uint64_t) text_size_oneMax +(uint64_t) LCP_P;

        }

        auto time4 = std::chrono::high_resolution_clock::now();


        uint64_t rightpreorderId = id2right[id_P].first;
        INT minGap = id2right[id_P].second;
        // next_id, the explicit node under P
        //rightpreorderId pf next_id
//        next_greater_id_with_right(id_P, id2right, next_id, rightpreorderId);





#ifdef USE_3D
        point3 min_point, max_point;
        bg::set<0>(min_point, static_cast<uint64_t>(id_P));
        bg::set<1>(min_point, static_cast<uint64_t>(patternSizes[i] + 2 * contextSize));
        bg::set<2>(min_point, static_cast<uint64_t>(0));
        bg::set<0>(max_point, static_cast<uint64_t>(rightpreorderId));
        bg::set<1>(max_point, static_cast<uint64_t>(INT_MAX));
        bg::set<2>(max_point, static_cast<uint64_t>(patternSizes[i] + 2 * contextSize));
        bg::model::box<point3> query_box(min_point, max_point);
        std::vector<scored_point3> cand;
#else
        point4 min_point, max_point;
        bg::set<0>(min_point, static_cast<uint64_t>(id_P));
        bg::set<1>(min_point, static_cast<uint64_t>(patternSizes[i] + 2 * contextSize));
        bg::set<2>(min_point, static_cast<uint64_t>(0));
        bg::set<3>(min_point, static_cast<uint64_t>(minGap));
        bg::set<0>(max_point, static_cast<uint64_t>(rightpreorderId));
        bg::set<1>(max_point, static_cast<uint64_t>(INT_MAX));
        bg::set<2>(max_point, static_cast<uint64_t>(patternSizes[i] + 2 * contextSize));
        bg::set<3>(max_point, static_cast<uint64_t>(INT_MAX));
        bg::model::box<point4> query_box(min_point, max_point);
        std::vector<point4> cand;
#endif
        auto time34 = std::chrono::high_resolution_clock::now();

        RT.query(bgi::intersects(query_box), std::back_inserter(cand));
        auto time5 = std::chrono::high_resolution_clock::now();
        double query_time4 = std::chrono::duration_cast<std::chrono::microseconds>(time5 - time34).count() * 0.000001;

        INT m  = cand.size();
        INT kk = std::min<INT>(kSize, m);

#ifdef USE_3D
        std::vector<uint64_t> scores(m);
        for (INT j = 0; j < m; j++) scores[j] = cand[j].second;
        if (kk > 0) {
            if (bottomMode)
                std::nth_element(scores.begin(), scores.begin() + kk, scores.end());
            else
                std::nth_element(scores.begin(), scores.begin() + kk, scores.end(), std::greater<uint64_t>{});
            scores.resize(kk);
        } else {
            scores.clear();
        }
#else
        auto cmp_d4 = [bottomMode](const point4& a, const point4& b) {
            const uint64_t da = bg::get<3>(a);
            const uint64_t db = bg::get<3>(b);
            return bottomMode ? (da < db) : (da > db);
        };
        if (kk > 0) {
            std::nth_element(cand.begin(), cand.begin() + kk, cand.end(), cmp_d4);
            cand.resize(kk);
        } else {
            cand.clear();
        }
#endif




        auto queryEnd = std::chrono::high_resolution_clock::now();

        double query_time =std::chrono::duration_cast<std::chrono::microseconds>(queryEnd - queryStart).count() * 0.000001;

        double time43 =std::chrono::duration_cast<std::chrono::microseconds>(time4 - time3).count() * 0.000001;
        double time32 =std::chrono::duration_cast<std::chrono::microseconds>(time3 - time2).count() * 0.000001;
        double time21 =std::chrono::duration_cast<std::chrono::microseconds>(time2 - time1).count() * 0.000001;



        std::cout << "Pattern " << i << ": " << patterns[i] << std::endl;

        if(!bottomMode){
            cout<<"Top mode: "<<endl;
        } else{
            cout<<"Bottom mode: "<<endl;
        }

        std::cout << "Requested k = " << kSize << ", candidates m = " << m
#ifdef USE_3D
                  << ", returned = " << scores.size() << " (min(k,m))\n";
#else
                  << ", returned = " << cand.size() << " (min(k,m))\n";
#endif

        // cout<<"Time for query_start 1: "<<time21<<endl;

        // cout<<"Time for query_start 2: "<<time32<<endl;

        // cout<<"Time for query_start 3: "<<time43<<endl;

        // cout<<"Time for query_start 4: "<<query_time4<<endl;

        cout<<"Time for query: "<<query_time<<endl;

        std::cout << "---------------------------------------" << endl;

        delete[] ZP;

    }


































    free(textStringWLeftMax);
    free(textStringWLeftMax_rev);
    free(textStringWoMax);

    for (auto &it: patterns){
        free(it);
    }
    free(SA_Z);
    free(LCP_Z);




}

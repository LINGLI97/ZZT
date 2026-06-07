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
#include "compactTrie_CC.h"
#include "SA_LCP_LCE.h"


using namespace std;



#include <malloc.h>

long long memory_usage() {
    struct mallinfo2 mi = mallinfo2();
    return mi.uordblks + mi.hblkhd;
}

// read text T, patterns and its length from file

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

//    text_string = (unsigned char *)malloc((text_size + 3) * sizeof(unsigned char));
//    text_string[0] = '$';
    text_string_woDollar = (unsigned char *)malloc((text_size + 1) * sizeof(unsigned char));


    char c = 0;
    for (INT i = 1; i < text_size + 1; i++) {
        is_text.read(reinterpret_cast<char *>(&c), 1);
//        text_string[i] = (unsigned char)c;
        text_string_woDollar[i-1] = (unsigned char)c;

        alphabet.insert((unsigned char)c);

    }

    is_text.close();
//    text_string[text_size +1 ] = '$';
    alphabet.insert('$');

//    text_string[text_size +2] = '\0';
    text_string_woDollar[text_size] = '\0';

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
        if (!(iss >> pattern_str)) {
            std::cerr << "Error reading line: " << line << std::endl;
            continue;  // Skip the invalid line
        }

        // Allocate memory for the pattern and store it in the vector
        unsigned char *pattern = (unsigned char *)malloc((pattern_str.size() + 1) * sizeof(unsigned char));
        std::copy(pattern_str.begin(), pattern_str.end(), pattern);
        pattern[pattern_str.size()] = '\0';  // null terminator
        patternSizes.push_back(pattern_str.size());
        patterns.push_back(pattern);
    }

    is_pattern.close();
}



// read context length from file

void readfile_woDollar(string &filename, string &patternPath, unsigned char * &text_string_woDollar, std::vector<unsigned char *> &patterns,
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
    text_string_woDollar = (unsigned char *)malloc((text_size + 1) * sizeof(unsigned char));


    char c = 0;
    for (INT i = 1; i < text_size + 1; i++) {
        is_text.read(reinterpret_cast<char *>(&c), 1);
//        text_string[i] = (unsigned char)c;
        text_string_woDollar[i-1] = (unsigned char)c;

        alphabet.insert((unsigned char)c);

    }

    is_text.close();
//    text_string[text_size +1 ] = '$';
    alphabet.insert('$');

//    text_string[text_size +2] = '\0';
    text_string_woDollar[text_size] = '\0';

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

void readfile_woDollar(string &filename, string &patternPath,unsigned char * &text_string_woDollar, std::vector<unsigned char *> &patterns,
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
    text_string_woDollar = (unsigned char *)malloc((text_size + 1) * sizeof(unsigned char));

    char c = 0;
    for (INT i = 1; i < text_size + 1; i++) {
        is_text.read(reinterpret_cast<char *>(&c), 1);
//        text_string[i] = (unsigned char)c;
        text_string_woDollar[i-1] = (unsigned char)c;
        alphabet.insert((unsigned char)c);

    }

    is_text.close();
//    text_string[text_size +1 ] = '$';
    alphabet.insert('$');

//    text_string[text_size +2] = '\0';

    text_string_woDollar[text_size] = '\0';

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
    reversed_s[length - 1] = '$'; // Don't forget to null-terminate the new string

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


INT compareZStrings(INT i, INT j, SA_LCP_LCE& DS_org, SA_LCP_LCE& DS_rev) {

//    cout<<"DS_rev.text_size - i - 2: "<<DS_rev.text_size - i - 2<<"; DS_rev.text_size - j - 2:"<<DS_rev.text_size - j - 2<<endl;

    INT r = DS_rev.LCE(DS_rev.text_size - i - 2, DS_rev.text_size - j - 2); // even positions
//    cout<<"i+1: "<<i +1<<"; j+1: "<< j +1<<endl;

    INT f = DS_org.LCE(i + 1, j + 1);                                       // odd positions

    if (r <= f)
        return 2 * r;
    else
        return 2 * f + 1;
}


bool isZLess(INT i, INT j, SA_LCP_LCE& DS_org, SA_LCP_LCE& DS_rev, unsigned char* T, INT text_size) {
    INT mismatch_pos = compareZStrings(i, j, DS_org, DS_rev);
    unsigned char ci = getZigZagChar(i, mismatch_pos, T, text_size);
    unsigned char cj = getZigZagChar(j, mismatch_pos, T, text_size);
    return ci <= cj;
}




void mergeZ(std::vector<INT>& indices, INT left, INT mid, INT right,
            std::vector<INT>& temp, SA_LCP_LCE& DS_org, SA_LCP_LCE& DS_rev,
            unsigned char* T, INT text_size) {

    INT i = left, j = mid, k = left;
    while (i < mid && j < right) {
        if (isZLess(indices[i], indices[j], DS_org, DS_rev, T, text_size)) {
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



void mergeSortIterativeZigZag(std::vector<INT>& indices, unsigned char* T, INT text_size,
                              SA_LCP_LCE& DS_org, SA_LCP_LCE& DS_rev, std::vector<INT>& LCP) {
    vector<INT> temp(text_size);

    for (INT width = 1; width < text_size; width *= 2) {
//        cout<<width<<endl;
        for (INT i = 0; i < text_size; i += 2 * width) {
            INT left = i;
            INT mid = min(i + width, text_size);
            INT right = min(i + 2 * width, text_size);
//            cout<<"left: "<<left<<"; mid: "<<mid<<"; right: "<<right<<endl;
            if (mid < right) {
                mergeZ(indices, left, mid, right, temp, DS_org, DS_rev, T, text_size);
            }
        }
    }

    // Build LCP
    LCP.resize(text_size);
    LCP[0] = 0;
    for (INT i = 1; i < text_size; ++i) {
        LCP[i] = compareZStrings(indices[i - 1], indices[i], DS_org, DS_rev);
    }
}

INT compareZStrings_direct(INT i, INT j, unsigned char* T, INT text_size) {
    for (INT d = 0; ; d++) {
        unsigned char ci = getZigZagChar(i, d, T, text_size);
        unsigned char cj = getZigZagChar(j, d, T, text_size);
        if (ci != cj || (ci == 255 && cj == 255)) return d;
    }
}

bool isZLess_direct(INT i, INT j, unsigned char* T, INT text_size) {
    INT d = compareZStrings_direct(i, j, T, text_size);
    return getZigZagChar(i, d, T, text_size) <= getZigZagChar(j, d, T, text_size);
}

void mergeZ_direct(std::vector<INT>& indices, INT left, INT mid, INT right,
                   std::vector<INT>& temp, unsigned char* T, INT text_size) {
    INT i = left, j = mid, k = left;
    while (i < mid && j < right) {
        if (isZLess_direct(indices[i], indices[j], T, text_size))
            temp[k++] = indices[i++];
        else
            temp[k++] = indices[j++];
    }
    while (i < mid)  temp[k++] = indices[i++];
    while (j < right) temp[k++] = indices[j++];
    for (INT t = left; t < right; ++t) indices[t] = temp[t];
}

void mergeSortIterativeZigZag_direct(std::vector<INT>& indices, unsigned char* T, INT text_size,
                                     std::vector<INT>& LCP) {
    vector<INT> temp(text_size);
    for (INT width = 1; width < text_size; width *= 2) {
        for (INT i = 0; i < text_size; i += 2 * width) {
            INT left = i;
            INT mid = min(i + width, text_size);
            INT right = min(i + 2 * width, text_size);
            if (mid < right)
                mergeZ_direct(indices, left, mid, right, temp, T, text_size);
        }
    }
    LCP.resize(text_size);
    LCP[0] = 0;
    for (INT i = 1; i < text_size; ++i)
        LCP[i] = compareZStrings_direct(indices[i - 1], indices[i], T, text_size);
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


unsigned char * addDollar(unsigned char * textStringWoDollar,INT text_size){
    unsigned char * text_string_left_dollar = (unsigned char *) malloc((text_size + 2)*sizeof(unsigned char));

    for (INT i = 0; i < text_size; ++i) {
        text_string_left_dollar[i] = textStringWoDollar[i];
    }
    text_string_left_dollar [text_size] = '$';
    text_string_left_dollar[text_size +1] ='\0';
    return text_string_left_dollar;
}


int main(int argc, char * argv[]){

    cmdline::parser parser;
    parser.add<string>("filePath", 'f', "the path to input file", false, "input.txt");
    parser.add<string>("patternPath", 'p', "the path to pattern file", false, "patterns.txt");



    parser.parse_check(argc, argv);

    string filePath = parser.get<string>("filePath");

    string patternPath = parser.get<string>("patternPath");


    // if bottom is specified, use bottom k instead of top k
    unsigned char *textStringWoDollar;


    /* readfile into text_string and pattern */
//    unsigned char *text_string_woDollar;

    std::vector<unsigned char *> patterns;

    INT text_size = 0;
    INT alphabetSize= 0;
    vector<INT> patternSizes;


    readfile_woDollar(filePath, patternPath, textStringWoDollar, patterns, text_size, alphabetSize, patternSizes);




    unsigned char *textStringWLeftDollar = addDollar(textStringWoDollar,text_size);
    unsigned char *textStringWLeftDollar_rev = reverseString(textStringWLeftDollar);

    auto start = std::chrono::high_resolution_clock::now();

    std::vector<INT> indices(text_size);
    for (INT i = 0; i < text_size; ++i) indices[i] = i;

    std::vector<INT> LCP;

#ifdef USE_DIRECT_COMPARE
    mergeSortIterativeZigZag_direct(indices, textStringWoDollar, text_size, LCP);
#else
    /*Prepared SA, invSA, LCP, LCE, rmq for the original string*/
    INT text_size_oneDollar =  text_size + 1;
    SA_LCP_LCE DS_org(textStringWLeftDollar,text_size_oneDollar);

    /*Prepared SA_rev, invSA_rev, LCP_rev, LCE_rev, rmq_rev*/

    SA_LCP_LCE DS_rev(textStringWLeftDollar_rev, text_size_oneDollar);

#ifdef VERBOSE
    printArray("SA", DS_org.SA, text_size_oneDollar);
    printArray("invSA", DS_org.invSA, text_size_oneDollar);

    printArray("LCP", DS_org.LCP, text_size_oneDollar);

    printstring("SA->string",DS_org.SA, text_size_oneDollar, DS_org.T);


    printArray("SA reverse", DS_rev.SA, text_size_oneDollar);
    printArray("invSA reverse", DS_rev.invSA, text_size_oneDollar);

    printArray("LCP reverse", DS_rev.LCP, text_size_oneDollar);
    printstring("SA->string reverse",DS_rev.SA, text_size_oneDollar, DS_rev.T);

#endif

    mergeSortIterativeZigZag(indices, textStringWoDollar, text_size, DS_org, DS_rev, LCP);
#endif


#ifdef VERBOSE
    // Print sorted zigzag strings
    cout << "Sorted ZigZag Strings:" << endl;
    for (INT i = 0; i < text_size; ++i) {
        INT start_idx = indices[i];
        cout << "ZZ[" << i << "] (starting at " << start_idx << "): ";

        // Generate and print zigzag string for this position
        INT pos = 0;
        INT cnt = 0;
        INT index_T = start_idx;

        while (index_T < text_size && index_T >= 0) {
            cout << textStringWoDollar[index_T];
            pos++;
            if (pos % 2 == 0) {
                index_T = start_idx - cnt;
            } else {
                cnt++;
                index_T = start_idx + cnt;
            }
        }
        cout << "$" << endl;
    }
    cout << endl;
#endif


    // for (INT i = 0; i < text_size; ++i){
    //     cout<<"indices["<<i<<"]:"<<indices[i]<<endl;
    //
    // }
#ifdef VERBOSE
    cout << "LCP: ";
    for (INT i = 0; i < LCP.size(); ++i) cout << LCP[i] << " ";
    cout << endl;

#endif


    long long IndexSpace_start = memory_usage();

    compactTrie Trie(indices, LCP, textStringWoDollar);
    auto t0 = std::chrono::high_resolution_clock::now();

    Trie.buildCC();
    // Trie.visualize("trie_visualization");


// ---- Report timing ----


    auto end = std::chrono::high_resolution_clock::now();







    long long IndexSpace_end = memory_usage();

    long long memory_Index = IndexSpace_end - IndexSpace_start;



    double Construction_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() * 0.000001;
    double ZZT_time = std::chrono::duration_cast<std::chrono::microseconds>(t0 - start).count() * 0.000001;






    cout<<"====================================== Zigzag Index Construction=============================="<<endl;

    cout<<"ZZT construction time: "<< ZZT_time<< " seconds"<<endl;
    cout<<"Total construction time: "<< Construction_time<< " seconds"<<endl;


    cout<<"Index memory: "<< memory_Index / (1024.0 * 1024.0)<<"MB"<<endl;



    cout << "=========================== Start to query all the patterns: Contextual Complexity ==========================="<< endl;

    // Start to query all the patterns
    for (INT i = 0; i < patterns.size(); i++) {
        auto queryStart = std::chrono::high_resolution_clock::now();

        unsigned char *ZP = generateZP(patterns[i], patternSizes[i]);

        // auto time1 = std::chrono::high_resolution_clock::now();
        // double query_time1 =std::chrono::duration_cast<std::chrono::microseconds>(time1 - queryStart).count() * 0.000001;

        Node *up = Trie.forward_search(ZP, patternSizes[i]);

        if (!up) {

//            std::cout << "Text string: " << text_string << std::endl;
            std::cout << "Pattern " << i << ": " << patterns[i] << std::endl;
#ifdef VERBOSE
            std::cout<<"ZP: "<<ZP<<endl;
#endif



            std::cerr << "The pattern does not exist in text string!" << std::endl;
            cout << "---------------------------------------" << endl;

            delete[] ZP;
            continue;
        }

        INT valueCC=0;
        INT d = up->depth - patternSizes[i];  // Distance from w to v

        if (d == 0) {
            // w is an explicit node
            valueCC = up->val_even;
        }
        else if (d % 2 == 0) {
            // d > 0 and d is even
            valueCC = d / 2 + up->val_even;
        }
        else {
            // d > 0 and d is odd
            valueCC = d / 2 + up->val_odd + up->child.size();
        }


        std::cout << "Pattern " << i << ": " << patterns[i] << std::endl;
        std::cout << "Contextual Complexity: " << valueCC << endl;
        auto queryEnd = std::chrono::high_resolution_clock::now();

        double query_time =std::chrono::duration_cast<std::chrono::microseconds>(queryEnd - queryStart).count() * 0.000001;

        cout << "Query time: " << query_time << " s" << endl;

        std::cout << "---------------------------------------" << endl;


#ifdef VERBOSE
        std::cout<<"ZP: "<<ZP<<endl;
#endif


        delete[] ZP;




        }




    free(textStringWLeftDollar);
    free(textStringWLeftDollar_rev);
    free(textStringWoDollar);

    for (auto &it: patterns){
        free(it);
    }





}

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

#include "compactTrie_LCCS.h"
#include "SA_LCP_LCE.h"


using namespace std;



#include <malloc.h>

long long memory_usage() {
    struct mallinfo2 mi = mallinfo2();
    return mi.uordblks + mi.hblkhd;
}

// read the multiple text input and concat them

void readfile(string &filename, string &patternPath, unsigned char * &text_string_woMax, std::vector<unsigned char *> &patterns, INT& text_size, INT& alphabetSize, vector<INT> & patternSizes, vector<INT> & lineSizes, vector<INT> & IdxSeparators){
    std::ifstream is_text(filename);
    if (!is_text) {
        std::cerr << "Error opening input file: " << filename << std::endl;
        return;
    }

    unordered_set<unsigned char> alphabet;
    std::string concatenated;
    std::string line;

    // First read all lines to collect the set of characters already in use
    std::vector<std::string> lines;
    while (std::getline(is_text, line)) {
        if (!line.empty()) {
            std::string filtered_line;
            // Filter: keep ASCII characters only (0-127)
            for (char c : line) {
                unsigned char uc = (unsigned char)c;
                if (uc <= 127) {  // Accept ASCII characters only
                    filtered_line += c;
                    alphabet.insert(uc);
                }
                // Ignore characters above 127
            }
            if (!filtered_line.empty()) {  // Keep non-empty lines only
                lines.push_back(filtered_line);
            }
        }
    }
    is_text.close();

    // Use characters absent from the input as separators, one distinct separator per text line
    // save 255 for the line end
    std::vector<unsigned char> available_separators;
    for (int i = 128; i <= 254; i++) {  // Search the range 128..254
        if (alphabet.find((unsigned char)i) == alphabet.end()) {
            available_separators.push_back((unsigned char)i);
        }
    }

    if (available_separators.size() < lines.size() - 1) {
        std::cerr << "Error: Not enough unused characters for separators!" << std::endl;
        return;
    }

    // Concatenate the text lines using distinct separators
    int separator_idx = 0;
    for (size_t i = 0; i < lines.size(); i++) {
        if (i > 0) {
            unsigned char sep = available_separators[separator_idx++];
            IdxSeparators.push_back(concatenated.size());
            concatenated += (char)sep;
            alphabet.insert(sep); // Add separator to alphabet
        }
        lineSizes.push_back(lines[i].size());
        concatenated += lines[i];
    }

    // Allocate memory and copy concatenated string
    text_size = concatenated.size();
    text_string_woMax = (unsigned char *)malloc((text_size + 1) * sizeof(unsigned char));

    for (INT i = 0; i < text_size; i++) {
        text_string_woMax[i] = (unsigned char)concatenated[i];
    }
    text_string_woMax[text_size] = '\0';

    alphabetSize = alphabet.size();


    /* open pattern file and write it into pattern*/
    std::ifstream is_pattern(patternPath, std::ios::binary);
    if (!is_pattern) {
        std::cerr << "Error opening pattern file: " << patternPath << std::endl;
    }


    while (std::getline(is_pattern, line)) {
        std::istringstream iss(line);
        std::string pattern_str;

        if (!(iss >> pattern_str)) {
            std::cerr << "Error reading line: " << line << std::endl;
            continue;  // Skip the invalid line
        }

        // Filter the pattern: keep ASCII characters only (0-127)
        std::string filtered_pattern;
        for (char c : pattern_str) {
            unsigned char uc = (unsigned char)c;
            if (uc <= 127) {
                filtered_pattern += c;
            }
        }

        if (filtered_pattern.empty()) {
            continue;  // Skip empty patterns
        }


        // Allocate memory for the pattern and store it in the vector
        unsigned char *pattern = (unsigned char *)malloc((filtered_pattern.size() + 1) * sizeof(unsigned char));
        std::copy(filtered_pattern.begin(), filtered_pattern.end(), pattern);
        pattern[filtered_pattern.size()] = '\0';  // null terminator
        patternSizes.push_back(filtered_pattern.size());
        patterns.push_back(pattern);
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


bool isZLess(pair<INT,INT> idx_i, pair<INT,INT> idx_j, SA_LCP_LCE& DS_org, SA_LCP_LCE& DS_rev, unsigned char* T, INT text_size) {
    INT i = idx_i.first;
    INT j = idx_j.first;
    INT mismatch_pos = compareZStrings(i, j, DS_org, DS_rev);
    unsigned char ci = getZigZagChar(i, mismatch_pos, T, text_size);
    unsigned char cj = getZigZagChar(j, mismatch_pos, T, text_size);
    return ci <= cj;
}




void mergeZ(std::vector<pair<INT,INT>>& indices, INT left, INT mid, INT right,
            std::vector<pair<INT,INT>>& temp, SA_LCP_LCE& DS_org, SA_LCP_LCE& DS_rev,
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



void mergeSortIterativeZigZag(std::vector<pair<INT,INT>>& indices, unsigned char* T, INT text_size,
                              SA_LCP_LCE& DS_org, SA_LCP_LCE& DS_rev, std::vector<INT>& LCP) {
    vector<pair<INT,INT>> temp(text_size);

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
        LCP[i] = compareZStrings(indices[i - 1].first, indices[i].first, DS_org, DS_rev);
    }
}

INT compareZStrings_direct(INT i, INT j, unsigned char* T, INT text_size) {
    for (INT d = 0; ; d++) {
        unsigned char ci = getZigZagChar(i, d, T, text_size);
        unsigned char cj = getZigZagChar(j, d, T, text_size);
        if (ci != cj || (ci == 255 && cj == 255)) return d;
    }
}

bool isZLess_direct(pair<INT,INT> idx_i, pair<INT,INT> idx_j, unsigned char* T, INT text_size) {
    INT i = idx_i.first;
    INT j = idx_j.first;
    INT d = compareZStrings_direct(i, j, T, text_size);
    return getZigZagChar(i, d, T, text_size) <= getZigZagChar(j, d, T, text_size);
}

void mergeZ_direct(std::vector<pair<INT,INT>>& indices, INT left, INT mid, INT right,
                   std::vector<pair<INT,INT>>& temp, unsigned char* T, INT text_size) {
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

void mergeSortIterativeZigZag_direct(std::vector<pair<INT,INT>>& indices, unsigned char* T, INT text_size,
                                     std::vector<INT>& LCP) {
    vector<pair<INT,INT>> temp(text_size);
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
        LCP[i] = compareZStrings_direct(indices[i - 1].first, indices[i].first, T, text_size);
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
    unsigned char * text_string_right_max = (unsigned char *) malloc((text_size + 2)*sizeof(unsigned char));

    for (INT i = 0; i < text_size; ++i) {
        text_string_right_max[i] = textStringWoMax[i];
    }
    text_string_right_max [text_size] = 255;
    text_string_right_max[text_size +1] ='\0';
    return text_string_right_max;
}


int main(int argc, char * argv[]){

    cmdline::parser parser;
    parser.add<string>("filePath", 'f', "the path to input file", false, "multiInput.txt");
    parser.add<string>("patternPath", 'p', "the path to pattern file", false, "patterns.txt");
    parser.add<int>("tau", 't', "the support", false, 1);



    parser.parse_check(argc, argv);

    string filePath = parser.get<string>("filePath");

    string patternPath = parser.get<string>("patternPath");

    INT tau = parser.get<int>("tau");


    unsigned char *textStringWoMax;



    std::vector<unsigned char *> patterns;

    INT text_size = 0;
    INT alphabetSize= 0;
    vector<INT> patternSizes;
    vector<INT> lineSizes;
    vector<INT> IdxSeparators;


    readfile(filePath, patternPath, textStringWoMax, patterns, text_size, alphabetSize, patternSizes, lineSizes, IdxSeparators);



    unsigned char *textStringWRightMax = addMax(textStringWoMax,text_size);
    unsigned char *textStringWRightMax_rev = reverseString(textStringWRightMax);

    auto start = std::chrono::high_resolution_clock::now();

//index, text id
    std::vector<pair<INT,INT>> indices(text_size);

    // Determine the text ID for each position using separator locations
    INT current_text_id = 0;
    INT separator_idx = 0;

    for (INT i = 0; i < text_size; ++i){
        // Check whether the next separator has been reached
        if (separator_idx < (INT)IdxSeparators.size() && i == IdxSeparators[separator_idx]) {
            current_text_id++;
            separator_idx++;
        }
        indices[i] = make_pair(i, current_text_id);
    }

    std::vector<INT> LCP;

#ifdef USE_DIRECT_COMPARE
    mergeSortIterativeZigZag_direct(indices, textStringWoMax, text_size, LCP);
#else
    /*Prepared SA, invSA, LCP, LCE, rmq for the original string*/
    INT text_size_oneMax =  text_size + 1;
    SA_LCP_LCE DS_org(textStringWRightMax,text_size_oneMax);

    /*Prepared SA_rev, invSA_rev, LCP_rev, LCE_rev, rmq_rev*/

    SA_LCP_LCE DS_rev(textStringWRightMax_rev, text_size_oneMax);

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

    mergeSortIterativeZigZag(indices, textStringWoMax, text_size, DS_org, DS_rev, LCP);
#endif





#ifdef VERBOSE
    cout << "LCP: ";
    for (INT i = 0; i < LCP.size(); ++i) cout << LCP[i] << " ";
    cout << endl;

#endif


    long long IndexSpace_start = memory_usage();


    compactTrie Trie(indices, LCP, textStringWoMax,IdxSeparators);
    auto t0 = std::chrono::high_resolution_clock::now();

    long long Trie_end = memory_usage();


    Trie.computeCSS();

    Trie.buildPointers(tau);

// ---- Report timing ----


    auto end = std::chrono::high_resolution_clock::now();







    long long IndexSpace_end = memory_usage();

    long long memory_Index = IndexSpace_end - IndexSpace_start;


    long long memory_Trie = Trie_end - IndexSpace_start;

    double Construction_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() * 0.000001;
    double ZZT_time = std::chrono::duration_cast<std::chrono::microseconds>(t0 - start).count() * 0.000001;








    cout<<"====================================== Zigzag Index Construction=============================="<<endl;
    cout<<"Tau: "<<tau<<endl;
    cout<<"ZZT construction time: "<< ZZT_time<< " seconds"<<endl;
    cout<<"Total construction time: "<< Construction_time<< " seconds"<<endl;


    cout<<"Trie Index memory: "<< memory_Trie / (1024.0 * 1024.0)<<"MB"<<endl;


    cout<<"Index memory: "<< memory_Index / (1024.0 * 1024.0)<<"MB"<<endl;




    cout << "=========================== Start to query all the patterns: LCCS ==========================="<< endl;

    // Start to query all the patterns
    // Then query each pattern
    for (INT i = 0; i < patterns.size(); i++) {
        auto queryStart = std::chrono::high_resolution_clock::now();

        unsigned char *ZP = generateZP(patterns[i], patternSizes[i]);

        Node *locus = Trie.forward_search(ZP, patternSizes[i]);

        if (!locus) {

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


        // 4. Check the pointer and return the result
        if (locus->ptr == nullptr) {
            std::cout << "Pattern " << i << ": " << patterns[i] << std::endl;

            cout << "No LPR string exists with tau >= " << tau << endl;
        } else {
            Node *u = locus->ptr;
            std::cout << "Pattern " << i << ": " << patterns[i] << std::endl;
            cout << "LPR exists with tau >= " << tau <<", P starts at " << u->start<< endl;

            // Output str(u) as the answer
            // cout<<"u->start: "<<u->start<<"; u->depth:"<<u->depth<<endl;
        }
        auto queryEnd = std::chrono::high_resolution_clock::now();
        double query_time =std::chrono::duration_cast<std::chrono::microseconds>(queryEnd - queryStart).count() * 0.000001;
        cout << "Query time: " << query_time << " s" << endl;

        std::cout << "---------------------------------------" << endl;

        delete[] ZP;
    }
//
//         auto time5 = std::chrono::high_resolution_clock::now();
//
//
//         auto queryEnd = std::chrono::high_resolution_clock::now();
//
//         double query_time =std::chrono::duration_cast<std::chrono::microseconds>(queryEnd - queryStart).count() * 0.000001;
//
//
//
//         std::cout << "Pattern " << i << ": " << patterns[i] << std::endl;
//
//
//
//
//         std::cout << "---------------------------------------" << endl;
//
//
// #ifdef VERBOSE
//         std::cout<<"ZP: "<<ZP<<endl;
// #endif
//
//
//         delete[] ZP;
//
//
//
//
//         }
//



    free(textStringWRightMax);
    free(textStringWRightMax_rev);
    free(textStringWoMax);

    for (auto &it: patterns){
        free(it);
    }





}

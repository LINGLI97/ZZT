
// navarro code: mutiple patterns
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <assert.h>
#include <sys/time.h>
#include "cmdline.h"
#include "baseline_GAP.h"
#include <unordered_set>
#ifdef _USE_64
#include <divsufsort64.h>                                         // include header for suffix sort
#endif

#ifdef _USE_32
#include <divsufsort.h>                                           // include header for suffix sort
#endif



#include <sdsl/bit_vectors.hpp>					  // include header for bit vectors
#include <sdsl/rmq_support.hpp>					  //include header for range minimum queries
//#include "stack.h"        					  // include header for stack structure

using namespace sdsl;
using namespace std;


#include <iostream>

#include <malloc.h>
long long memory_usage() {
    struct mallinfo2 mi = mallinfo2();
    return mi.uordblks + mi.hblkhd;
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


/* Reversing a string in place in linear time */
void reverse( unsigned char * &s )
{
    INT length = strlen( ( char * ) s ) ;
    unsigned char temp = ' ';
    INT i = 0, j = length-1;

    while( i < j )
    {
        temp = s[j];
        s[j] = s[i];
        s[i] = temp;
        i++; j--;
    }
}


// Returns a new reversed string without modifying the original
unsigned char* reverseReturn(unsigned char* &s) {
    INT length = strlen((char*) s);
    unsigned char* reversed_s = ( unsigned char * ) malloc (  ( length + 1 ) * sizeof ( unsigned char ) );


    for (INT i = 0; i < length; i++) {
        reversed_s[i] = s[length - 1 - i];
    }
    reversed_s[length] = '\0'; // Don't forget to null-terminate the new string

    return reversed_s; // Return the new dynamically allocated reversed string
}


unsigned int LCParray ( unsigned char *text, INT n, INT * SA, INT * ISA, INT * LCP )
{
    INT i=0, j=0;

    LCP[0] = 0;
    for ( i = 0; i < n; i++ ) // compute LCP[ISA[i]]
        if ( ISA[i] != 0 )
        {
            if ( i == 0) j = 0;
            else j = (LCP[ISA[i-1]] >= 2) ? LCP[ISA[i-1]]-1 : 0;
            while ( text[i+j] == text[SA[ISA[i]-1]+j] )
                j++;
            LCP[ISA[i]] = j;
        }

    return ( 1 );
}


/* Computes the length of lcp of two suffixes of two strings */
INT lcp ( unsigned char *  x, INT M, unsigned char * y, INT l, INT a_size, INT w_size )
{
    INT xx = a_size;
    if ( M >= xx ) return 0;
    INT yy = w_size;
    if ( l >= yy ) return 0;

    INT i = 0;
    while ( ( M + i < xx ) && ( l + i < yy ) )
    {
        if ( x[M+i] != y[l+i] )	break;
        i++;
    }
    return i;
}




/* Searching a list of strings using LCP from "Algorithms on Strings" by Crochemore et al. Algorithm takes O(m + log n), where n is the list size and m the length of pattern */
pair<INT,INT> pattern_matching ( unsigned char *  p, unsigned char *  T, INT * SA, INT * LCP, rmq_succinct_sct<> &rmq, INT n, INT p_size )
{

    INT m = p_size; //length of pattern
    INT N = n; //length of string
    INT d = -1;
    INT ld = 0;
    INT f = n;
    INT lf = 0;

    pair<INT,INT> interval;

    while ( d + 1 < f )
    {

        INT i = (d + f)/2;

        /* lcp(i,f) */
        INT lcpif;

        if( f == n )
            lcpif = 0;
        else lcpif = LCP[rmq ( i + 1, f ) ];

        /* lcp(d,i) */
        INT lcpdi;

        if( i == n )
            lcpdi = 0;
        else lcpdi = LCP[rmq ( d + 1, i ) ];

        if ( ( ld <= lcpif ) && ( lcpif < lf ) )
        {
            d = i;
            ld = lcpif;
        }
        else if ( ( ld <= lf ) && ( lf < lcpif ) ) 	f = i;
        else if ( ( lf <= lcpdi ) && ( lcpdi < ld ) )
        {
            f = i;
            lf = lcpdi;
        }
        else if ( ( lf <= ld ) && ( ld < lcpdi ) )	d = i;
        else
        {
            INT l = std::max (ld, lf);
            l = l + lcp ( T, SA[i] + l, p, l, n, p_size );
            if ( l == m ) //lower bound is found, let's find the upper bound
            {
                INT e = i;
                while ( d + 1 < e )
                {
                    INT j = (d + e)/2;

                    /* lcp(j,e) */
                    INT lcpje;

                    if( e == n )
                        lcpje = 0;
                    else lcpje = LCP[rmq ( j + 1, e ) ];

                    if ( lcpje < m ) 	d = j;
                    else 			e = j;
                }

                /* lcp(d,e) */
                INT lcpde;

                if( e == n )
                    lcpde = 0;
                else lcpde = LCP[rmq ( d + 1, e ) ];

                if ( lcpde >= m )	d = std::max (d-1,( INT ) -1 );

                e = i;
                while ( e + 1 < f )
                {
                    INT j = (e + f)/2;

                    /* lcp(e,j) */
                    INT lcpej;

                    if( j == n )
                        lcpej = 0;
                    else lcpej = LCP[rmq ( e + 1, j ) ];

                    if ( lcpej < m ) 	f = j;
                    else 			e = j;
                }

                /* lcp(e,f) */
                INT lcpef;

                if( f == n )
                    lcpef = 0;
                else lcpef = LCP[rmq ( e + 1, f ) ];

                if ( lcpef >= m )	f = std::min (f+1,n);

                interval.first = d + 1;
                interval.second = f - 1;
                return interval;


            }
            else if ( ( l == N - SA[i] ) || ( ( SA[i] + l < N ) && ( l != m ) && ( T[SA[i]+l] < p[l] ) ) )
            {
                d = i;
                ld = l;
            }
            else
            {
                f = i;
                lf = l;
            }
        }
    }


    interval.first = d + 1;
    interval.second = f - 1;
    return interval;
}




struct Range {
    INT s, e;
    Range(INT start, INT end) : s(start), e(end) {}
};


void findSubIntervalsRMQ(INT s, INT e, INT t, INT * LCP, rmq_succinct_sct<> &rmq, std::vector<std::pair<INT, INT>> &subIntervals) {
    //s: starting; e: ending, t: m + l, where m is |P|, l = |X| =|Y|


    if (s > e){

        fprintf(stderr, " Error: starting position is bigger than ending position!\n" );
        exit( EXIT_FAILURE );
    }
    //P only one occurrence

    if (s < e){
        stack<Range> ranges;
        ranges.push(Range(s, e));
        vector<INT> breakPoints;

        while (!ranges.empty()) {
            Range range = ranges.top();
            ranges.pop();

            if (range.s > range.e)
            {

                fprintf(stderr, " Error: starting position is bigger than ending position!\n" );
                exit( EXIT_FAILURE );
            }
//            if (range.s == range.e) {
//                continue;
//            }

            INT p = rmq(range.s, range.e);
            if (LCP[p] < t) {
                if (range.s < p) {
                    ranges.push(Range(range.s, p - 1));
                }

                if (p < range.e){
                    ranges.push(Range(p + 1, range.e));
                }

                breakPoints.push_back(p);
            }

        }
        sort(breakPoints.begin(),breakPoints.end());
        INT current_start = s;
        // get all subintervals
        for (INT i = 1; i < breakPoints.size(); ++i) {
            INT current_breakpoint = breakPoints[i];

            // Create an interval from the current start to the current breakpoint
            subIntervals.push_back({current_start, current_breakpoint-1});

            // Update the start for the next interval, which begins right after the current breakpoint
            current_start = current_breakpoint;
        }

        // If the last breakpoint is not the end, add the final interval
        if (current_start < e+1) {
            subIntervals.push_back({current_start,e});
        }

    } else{
        subIntervals.push_back({s,e});
    }



}

void findSubIntervalsScaning(INT s, INT e, INT t, INT * LCP, std::vector<std::pair<INT, INT>> &subIntervals) {
    //s: starting; e: ending, t: m + l, where m is |P|, l = |X| =|Y|
    if (s > e){
        fprintf(stderr, " Error: starting position is bigger than ending position!\n" );
        exit( EXIT_FAILURE );

    }

    //P only one occurrence
    if (s < e){
        INT current_start = s;

        for (INT i = s + 1; i < e +1; ++i) {
            // Check if the LCP value at this position is less than t (i.e., m+l)
            if (LCP[i] < t) {
                // If we are not already at the start of a new interval, end the current one
                if (current_start < i) {
                    subIntervals.push_back({current_start, i - 1});
                }
                // Start a new interval
                current_start = i;
            }
        }

        // add the last interval if it ends at the 'end'
        if (current_start < e +1) {
            subIntervals.push_back({current_start, e});
        }
    } else{
        subIntervals.push_back({s,e});


    }



}

//// read file and add $ at the beginning and end of text string, using x, y from input file
//void readfile(string &filename, string &patternPath, unsigned char * &text_string, std::vector<unsigned char *> &patterns,
//              std::vector<std::pair<INT, INT>> &contextSizes, INT& text_size, vector<INT> & patternSizes){
//    std::ifstream is_text(filename, std::ios::binary);
//    if (!is_text) {
//        std::cerr << "Error opening input file: " << filename << std::endl;
//
//    }
//
//
//    is_text.seekg(0, std::ios::end);
//    text_size = is_text.tellg();
//    is_text.seekg(0, std::ios::beg);
//
//
//
//
//    text_string = (unsigned char*) malloc((text_size +  3) * sizeof(unsigned char));
//
//    text_string[0] = '$';
//
//    char c = 0;
//    for (INT i = 0; i < text_size; i++) {
//        is_text.read(reinterpret_cast<char*>(&c), 1);
//        text_string[i + 1] = (unsigned char)c;
//    }
//
//
//    is_text.close();
//    text_string[text_size+1] = '$';           // end with '$'
//    text_string[text_size+2] = '\0';          // null
//
//    text_size = text_size +2;
////    text_size = text_size + 2 ;
//    is_text.close();
//
//
//    /* open pattern file and write it into pattern*/
//    std::ifstream is_pattern(patternPath, std::ios::binary);
//    if (!is_pattern) {
//        std::cerr << "Error opening pattern file: " << patternPath << std::endl;
//    }
//
//
//    std::string line;
//    while (std::getline(is_pattern, line)) {
//        std::istringstream iss(line);
//        std::string pattern_str;
//        INT x;
//        INT y;
//
//        if (!(iss >> pattern_str >> x >> y)) {
//            std::cerr << "Error reading line: " << line << std::endl;
//            continue;  // Skip the invalid line
//        }
//
//        // Allocate memory for the pattern and store it in the vector
//        unsigned char *pattern = (unsigned char *)malloc((pattern_str.size() + 1) * sizeof(unsigned char));
//        std::copy(pattern_str.begin(), pattern_str.end(), pattern);
//        pattern[pattern_str.size()] = '\0';  // null terminator
//        patternSizes.push_back(pattern_str.size());
//        patterns.push_back(pattern);
//        contextSizes.push_back({x, y});
//    }
//
//    is_pattern.close();
//}



// read file and add $ at the beginning and end of text string, using x, y from input
void readfile(string &filename, string &patternPath, unsigned char * &text_string, std::vector<unsigned char *> &patterns,
              std::vector<pair<INT,INT>> &contextSizes, INT& text_size, vector<INT> & patternSizes,  vector<INT> & kSizes){

    std::ifstream is_text(filename, std::ios::binary);
    if (!is_text) {
        std::cerr << "Error opening input file: " << filename << std::endl;

    }


    is_text.seekg(0, std::ios::end);
    text_size = is_text.tellg();
    is_text.seekg(0, std::ios::beg);




    text_string = (unsigned char*) malloc((text_size +  3) * sizeof(unsigned char));

    text_string[0] = '$';

    char c = 0;
    for (INT i = 0; i < text_size; i++) {
        is_text.read(reinterpret_cast<char*>(&c), 1);
        text_string[i + 1] = (unsigned char)c;
    }


    is_text.close();
    text_string[text_size+1] = '$';           // end with '$'
    text_string[text_size+2] = '\0';          // null

    text_size = text_size +2;
//    text_size = text_size + 2 ;
    is_text.close();


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
        INT r;
        INT k;

        if (!(iss >> pattern_str >> l >> r>>k)) {
            std::cerr << "Error reading line: " << line << std::endl;
            continue;  // Skip the invalid line
        }

        // Allocate memory for the pattern and store it in the vector
        unsigned char *pattern = (unsigned char *)malloc((pattern_str.size() + 1) * sizeof(unsigned char));
        std::copy(pattern_str.begin(), pattern_str.end(), pattern);
        pattern[pattern_str.size()] = '\0';  // null terminator
        patternSizes.push_back(pattern_str.size());
        patterns.push_back(pattern);
        contextSizes.push_back({l,r});
        kSizes.push_back(k);

    }

    is_pattern.close();
}
int main (int argc, char *argv[])
{

    /* get the specified parameters from parser */

    cmdline::parser parser;
    parser.add<string>("filePath", 'f', "the path to input file", false, "input.txt");
    parser.add<string>("patternPath", 'p', "the path to pattern file", false, "patterns.txt");
    parser.add<int>("L", 'l', "the length of context ", false, -1);
    parser.add<int>("R", 'r', "the length of right context ", false, -1);

    parser.add<int>("K", 'k', "top k ", false, -1);
    parser.add("bottom", 'b', "select bottom k instead of top k");


    parser.parse_check(argc, argv);


    string filename = parser.get<string>("filePath");

    string patternPath = parser.get<string>("patternPath");

    // if l,k are not specified (negative), use the default value in the pattern file
    INT l = parser.get<int>("L");
    INT r = parser.get<int>("R");

    INT k = parser.get<int>("K");

    // if bottom is specified, use bottom k instead of top k
    bool bottomMode = parser.exist("bottom");

//    /* open input file and write it into text_string */
//
// add $ to the beginning and end of text_string
    unsigned char* text_string;
    std::vector<unsigned char *> patterns;
    std::vector<pair<INT,INT>> contextSizes;


    INT n = 0;

    vector<INT> patternSizes;

    vector<INT> kSizes;


    readfile(filename,patternPath,text_string, patterns, contextSizes, n, patternSizes,kSizes);

    if (l > 0 and r > 0) {
        for (INT i = 0; i < contextSizes.size(); i++) {
            contextSizes[i].first = l;
            contextSizes[i].second = r;
        }
    }
    if (k > 0){
        for (INT i = 0; i < kSizes.size(); i++) {
            kSizes[i] = k;
        }
    }

//    vector<INT> patternSizes;
//    std::vector<unsigned char *> patterns;
//    std::vector<std::pair<INT, INT>> contextSizes;
//
//
//    std::ifstream is_text(filename, std::ios::binary);
//    if (!is_text) {
//        std::cerr << "Error opening input file: " << filename << std::endl;
//        return -1;
//    }
//    is_text.seekg(0, std::ios::end);
//    INT text_size = is_text.tellg();
//    is_text.seekg(0, std::ios::beg);



//    unsigned char* text_string = (unsigned char*) malloc((text_size + 1) * sizeof(unsigned char));
//
//    char c = 0;
//    for (INT i = 0; i < text_size; i++) {
//        is_text.read(reinterpret_cast<char*>(&c), 1);
//        text_string[i] = (unsigned char)c;
//    }
//
//    is_text.close();
//    text_string[text_size] = '\0';








    /*reversed string*/
    unsigned char* text_string_rev = reverseReturn(text_string);


    long long used_start = memory_usage();







    auto Construction_start = std::chrono::high_resolution_clock::now();

    /*Prepared SA_rev, invSA_rev, LCP_rev*/
    INT * SA_rev;
    INT * invSA_rev;
    INT * LCP_rev;
    SA_rev = ( INT * ) malloc( ( n ) * sizeof( INT ) );
    if( ( SA_rev == NULL) )
    {

        fprintf(stderr, " Error: Cannot allocate memory for SA.\n" );
        return ( 0 );
    }

#ifdef _USE_64
    if( divsufsort64( text_string_rev, SA_rev,  n ) != 0 )
    {

        fprintf(stderr, " Error: SA computation failed.\n" );
        exit( EXIT_FAILURE );
    }
#endif

#ifdef _USE_32
    if( divsufsort(text_string_rev, SA_rev,  n ) != 0 )
        {
            fprintf(stderr, " Error: SA computation failed.\n" );
            exit( EXIT_FAILURE );
        }
#endif



    /*Compute the inverse SA_rev array */
    invSA_rev = ( INT * ) malloc( (n) * sizeof( INT ) );
    if( ( invSA_rev == NULL) )
    {

        fprintf(stderr, " Error: Cannot allocate memory for invSA.\n" );
        return ( 0 );
    }

    for ( INT i = 0; i < n; i ++ )
    {
        invSA_rev [SA_rev[i]] = i;
    }


    LCP_rev = ( INT * ) malloc  (( n ) *sizeof( INT ) );
    if( ( LCP_rev == NULL) )
    {
        fprintf(stderr, " Error: Cannot allocate memory for LCP.\n" );
        return ( 0 );
    }


    /* Compute the LCP_rev array */
    LCParray( text_string_rev, n, SA_rev, invSA_rev, LCP_rev );


    int_vector<> v1( n , 0 ); // create a vector of length n and initialize it with 0s

    for ( INT i = 0; i < n; i ++ )
    {
        v1[i] = LCP_rev[i];
    }

    rmq_succinct_sct<> rmq_rev(&v1);

    util::clear(v1);


    /*Prepared SA, invSA, LCP for the normal string*/


    INT * SA;
    INT * LCP;
    INT * invSA;
    SA = ( INT * ) malloc( ( n ) * sizeof( INT ) );

    if( ( SA == NULL) )
    {
        fprintf(stderr, " Error: Cannot allocate memory for SA.\n" );
        return ( 0 );
    }
    long long end = memory_usage();


#ifdef _USE_64
    if( divsufsort64( text_string, SA,  n ) != 0 )
        {
            fprintf(stderr, " Error: SA computation failed.\n" );
            exit( EXIT_FAILURE );
        }
#endif

#ifdef _USE_32
    if( divsufsort( text_string, SA,  n ) != 0 )
        {
            fprintf(stderr, " Error: SA computation failed.\n" );
            exit( EXIT_FAILURE );
        }
#endif

    /*Compute the inverse SA array */
    invSA = ( INT * ) malloc( ( n ) * sizeof( INT ) );



    if( ( invSA == NULL) )
    {
        fprintf(stderr, " Error: Cannot allocate memory for invSA.\n" );
        return ( 0 );
    }

    for ( INT i = 0; i < n; i ++ )
    {
        invSA [SA[i]] = i;
    }



    LCP = ( INT * ) malloc  ( ( n ) * sizeof( INT ) );
    if( ( LCP == NULL) )
    {
        fprintf(stderr, " Error: Cannot allocate memory for LCP.\n" );
        return ( 0 );
    }



    /* Compute the LCP array */
    LCParray( text_string, n, SA, invSA, LCP );


    int_vector<> v2( n , 0 ); // create a vector of length n and initialize it with 0s


    for ( INT i = 0; i < n; i ++ )
    {
        v2[i] = LCP[i];
    }

    rmq_succinct_sct<> rmq(&v2);

    util::clear(v2);


    /* Construct C array*/

    INT* C;
    C = ( INT * ) malloc( ( n ) * sizeof( INT ) );
    if( ( C == NULL) )
    {
        fprintf(stderr, " Error: Cannot allocate memory for C.\n" );
        return ( 0 );
    }

    for (INT i = 0; i < n; i++) {
        C[i] = invSA[n - SA_rev[i] -1];

    }

    int_vector<> v3( n , 0 ); // create a vector of length n and initialize it with 0s

    for ( INT i = 0; i < n; i ++ )
    {
        v3[i] = C[i];
    }

    rmq_succinct_sct<> rmq_C(&v3);

    util::clear(v3);


    auto Construction_end = std::chrono::high_resolution_clock::now();
    double Construction_time = std::chrono::duration_cast < std::chrono::microseconds > (Construction_end - Construction_start).count()*0.000001;




    cout<< "Time for construction of Navarro's baseline: "<<Construction_time<<endl;

    long long used_end = memory_usage();
    long long memory_used = used_end-used_start;
    cout << "Index memory in Navarro's baseline: " << memory_used / (1024.0 * 1024.0) << " MB" << endl;
//
//
//    printArray("SA_rev", SA_rev, n);
//
//
//    printArray("invSA_rev", invSA_rev, n);
//
//
//    printArray("LCP_rev", LCP_rev, n);
//
//    printArray("SA", SA, n);
//
//
//    printArray("invSA", invSA, n);
//
//
//    printArray("LCP", LCP, n);
//
//    printstring("SA->string",SA, n,text_string);
//    printstring("SA_rev->string",SA_rev, n,text_string_rev);
#ifdef VERBOSE
    printArray("SA_rev", SA_rev, n);


    printArray("invSA_rev", invSA_rev, n);


    printArray("LCP_rev", LCP_rev, n);

    printArray("SA", SA, n);


    printArray("invSA", invSA, n);


    printArray("LCP", LCP, n);

    printstring("SA->string",SA, n,text_string);
    printstring("SA_rev->string",SA_rev, n,text_string_rev);

#endif


    cout<<"====================Starting multiple queries saved in "<<patternPath<<"========================"<<endl;

    for (INT i =0; i<patterns.size(); i ++) {
        auto Query_start = std::chrono::high_resolution_clock::now();


        INT x = contextSizes[i].first;

        INT y = contextSizes[i].second;
        INT k = kSizes[i];

        /*reversed pattern*/
        unsigned char *pattern_rev = reverseReturn(patterns[i]);

/*Given P and context length l, use RMQ function to partition the intervals*/
        pair<INT, INT> interval_rev = pattern_matching(pattern_rev, text_string_rev, SA_rev, LCP_rev, rmq_rev, n,
                                                       patternSizes[i]);

        //    cout<<interval_rev.first<<" "<<interval_rev.second<<endl;
        // RMQ to partition the intervals

        if (interval_rev.first > interval_rev.second) {
            std::cout << "Pattern " <<i<<": " << patterns[i]<< std::endl;
            std::cout << "x: " << x << std::endl;

            std::cout << "y: " << y << std::endl;
            cout<<"Error: starting position is bigger than ending position! Please check whether your pattern exists or not!"<<endl;
            cout<<"---------------------------------------"<<endl;

            free(pattern_rev);
            continue;
        }
        auto time1 = std::chrono::high_resolution_clock::now();
        double timepoint_1 = std::chrono::duration_cast<std::chrono::microseconds>(time1 - Query_start).count() * 0.000001;

        std::vector<std::pair<INT, INT>> subIntervalsRMQ;
        findSubIntervalsRMQ(interval_rev.first, interval_rev.second, patternSizes[i] + x, LCP_rev, rmq_rev,
                            subIntervalsRMQ);

        auto time2 = std::chrono::high_resolution_clock::now();
        double timepoint_2 = std::chrono::duration_cast<std::chrono::microseconds>(time2 - time1).count() * 0.000001;


        //    std::vector<std::pair<INT, INT>> subIntervalsScaning;
        //    findSubIntervalsScaning(interval_rev.first, interval_rev.second, pattern_size + l , LCP_rev, subIntervalsScaning);




        std::vector<std::pair<INT, INT>> IntervalsMapped;

        //Mapping the suffix array intervals of the reversed string to the intervals of the normal string


        for (pair<INT, INT> subInterval: subIntervalsRMQ) {
            INT normalIntervalFirst, normalIntervalSecond;
            INT indexFirst = n - patternSizes[i] - x - SA_rev[rmq_C(subInterval.first, subInterval.second)];
            //            INT indexSecond = n - pattern_size - l  - SA_rev[rmq_C(subInterval.first,subInterval.second)];

            if (indexFirst > -1) {
                normalIntervalFirst = invSA[indexFirst];
                normalIntervalSecond = normalIntervalFirst + subInterval.second - subInterval.first;
            } else {
                continue;

            }


            if (normalIntervalFirst < normalIntervalSecond) {
                IntervalsMapped.push_back({normalIntervalFirst, normalIntervalSecond});
            } else {
                IntervalsMapped.push_back({normalIntervalSecond, normalIntervalFirst});
            }


        }

        auto time3 = std::chrono::high_resolution_clock::now();
        double timepoint_3 = std::chrono::duration_cast<std::chrono::microseconds>(time3 - time2).count() * 0.000001;

        std::vector<std::pair<INT, INT>> subIntervalsRMQNonNeg;


        // conducting partitioning on every mapped subInterval further by sharing the first m + 2 * l symbols
        for (pair<INT, INT> subInterval: IntervalsMapped) {
            //        if (subInterval.first > -1 and subInterval.second > -1){
            std::vector<std::pair<INT, INT>> subIntervalsRMQNormal;
            findSubIntervalsRMQ(subInterval.first, subInterval.second, patternSizes[i] + x + y, LCP, rmq,
                                subIntervalsRMQNormal);

            //        std::vector<std::pair<INT, INT>> subIntervalsScaningNormal;
            //        findSubIntervalsScaning( subInterval.first, subInterval.second, pattern_size + 2 * l , LCP, subIntervalsScaningNormal);

            for (pair<INT, INT> subsubInterval: subIntervalsRMQNormal) {
                if (n - patternSizes[i] - x - y - SA[subsubInterval.first] > -1) {
                    subIntervalsRMQNonNeg.push_back(subsubInterval);
                } else {
                    continue;
                }
            }

        }

        auto time4 = std::chrono::high_resolution_clock::now();
        double timepoint_4 = std::chrono::duration_cast<std::chrono::microseconds>(time4 - time3).count() * 0.000001;


        vector<pair<INT,INT>> frequency_sumGap;
        for (const auto& interval : subIntervalsRMQNonNeg) {
            vector<INT> occ;
            for (INT j = interval.first; j <= interval.second; j++) {
                occ.push_back(SA[j]);
            }
            sort(occ.begin(), occ.end());

            INT gap = abs(occ.at(occ.size()-1) - occ.at(0));
            frequency_sumGap.push_back({gap,interval.second - interval.first + 1});
        }
        auto time45 = std::chrono::high_resolution_clock::now();

        double timepoint_45 = std::chrono::duration_cast<std::chrono::microseconds>(time45 - time4).count() * 0.000001;

        double timepoint_6, timepoint_7;
        if (bottomMode) {
            // bottom -k
            auto time5 = std::chrono::high_resolution_clock::now();

            sort(frequency_sumGap.begin(), frequency_sumGap.end());
            auto time6 = std::chrono::high_resolution_clock::now();
            timepoint_6 = std::chrono::duration_cast<std::chrono::microseconds>(time6 - time5).count() * 0.000001;

            if(frequency_sumGap.size()>k){
                for (int xx = 0; xx < k; ++xx) {

                }
                std::cout << "Pattern " << i << ": " << patterns[i] << std::endl;
                std::cout << "l: " <<contextSizes[i].first <<"; r: " <<contextSizes[i].second << "; k: " << k << std::endl;

                std::cout << "We can get bottom " << kSizes[i] << " from " << frequency_sumGap.size() << " results." << endl;

            } else if (frequency_sumGap.size()<k){

                for (int xx = 0; xx < frequency_sumGap.size(); ++xx) {

                }
                std::cout << "Pattern " << i << ": " << patterns[i] << std::endl;
                std::cout << "l: " <<contextSizes[i].first <<"; r: " <<contextSizes[i].second << "; k: " << k << std::endl;

                std::cout << "We can get only get bottom " << frequency_sumGap.size() << " though k = " << k << endl;


            } else{
                for (int xx = 0; xx < frequency_sumGap.size(); ++xx) {

                }
                std::cout << "Pattern " << i << ": " << patterns[i] << std::endl;
                std::cout << "l: " <<contextSizes[i].first <<"; r: " <<contextSizes[i].second << "; k: " << k << std::endl;

                std::cout << "We can get bottom " << k << endl;

            }
            auto time7 = std::chrono::high_resolution_clock::now();
            timepoint_7 = std::chrono::duration_cast<std::chrono::microseconds>(time7 - time6).count() * 0.000001;

        } else{

            //top -k
            auto time5 = std::chrono::high_resolution_clock::now();

            std::sort(frequency_sumGap.begin(), frequency_sumGap.end(),
                      [](const auto& a, const auto& b) {
                          return a.first > b.first;
                      });
            auto time6 = std::chrono::high_resolution_clock::now();
            timepoint_6 = std::chrono::duration_cast<std::chrono::microseconds>(time6 - time5).count() * 0.000001;

            if(frequency_sumGap.size()>k){
                for (int xx = 0; xx < k; ++xx) {

                }
                std::cout << "Pattern " << i << ": " << patterns[i] << std::endl;
                std::cout << "l: " <<contextSizes[i].first <<"; r: " <<contextSizes[i].second << "; k: " << k << std::endl;

                std::cout << "We can get top " << kSizes[i] << " from " << frequency_sumGap.size() << " results." << endl;

            } else if (frequency_sumGap.size()<k){

                for (int xx = 0; xx < frequency_sumGap.size(); ++xx) {

                }
                std::cout << "Pattern " << i << ": " << patterns[i] << std::endl;
                std::cout << "l: " <<contextSizes[i].first <<"; r: " <<contextSizes[i].second << "; k: " << k << std::endl;

                std::cout << "We can get only get top " << frequency_sumGap.size() << " though k = " << k << endl;


            } else{
                for (int xx = 0; xx < frequency_sumGap.size(); ++xx) {

                }
                std::cout << "Pattern " << i << ": " << patterns[i] << std::endl;
                std::cout << "l: " <<contextSizes[i].first <<"; r: " <<contextSizes[i].second << "; k: " << k << std::endl;

                std::cout << "We can get top " << k << endl;

            }
            auto time7 = std::chrono::high_resolution_clock::now();
            timepoint_7 = std::chrono::duration_cast<std::chrono::microseconds>(time7 - time6).count() * 0.000001;

        }


        auto Query_end = std::chrono::high_resolution_clock::now();

        //    long long used_end = memory_usage();
        //    long long memory_used = used_end-used_start;
        double Query_time = std::chrono::duration_cast<std::chrono::microseconds>(Query_end - Query_start).count() * 0.000001;

        // cout<<"Time for time point 1 "<<timepoint_1<<endl;
        // cout<<"Time for time point 2 "<<timepoint_2<<endl;
        // cout<<"Time for time point 3 "<<timepoint_3<<endl;
        // cout<<"Time for time point 4 "<<timepoint_4<<endl;

        // cout<<"Time for time point 45 "<<timepoint_45<<endl;

        // cout<<"Time for time point 6 "<<timepoint_6<<endl;
        // cout<<"Time for time point 7 "<<timepoint_7<<endl;

        cout<<"There are "<<subIntervalsRMQNonNeg.size()<< " distinct XPY that occur in T."<<endl;

        cout<<"Time for query: "<<Query_time<<endl;
        cout<<"------------------------------------------------------"<<endl;

        free(pattern_rev);

    }


    /*print out information*/



//
//
//#ifdef VERBOSE
//    cout<< "The experiment for given P = ";
//    for ( INT i = 0; i < pattern_size; i ++ ){
//        cout<<pattern[i];
//    }
//    cout<<" while the length of context x = "<<x<<"; y = "<<y<<": "<<endl;
//    // Print the generated intervals
//    for (const auto& interval : subIntervalsRMQNonNeg) {
//        std::cout << "SA Interval: [" << interval.first << ", " << interval.second << "]; ";
//        cout<<"occ: ";
//
//        for (INT i = interval.first;i < interval.second + 1; i++){
//            if (i == interval.second){
//                cout<<SA[i];
//
//            } else{
//                cout<<SA[i]<<", ";
//
//            }
//
//        }
//        cout<<": ";
//        for (INT i = 0;i < pattern_size + x + y ;i ++){
//            if (i == x or i == pattern_size + x){
//                cout<<" ";
//            }
//            cout<<text_string[SA[interval.first] + i];
//        }
//        cout<<std::endl;
//    }
//#endif




    /*free the memory*/

    free(text_string);
    free(text_string_rev);

    for (auto &it: patterns){
        free(it);
    }

    free(SA_rev);
    free(invSA_rev);
    free(LCP_rev);

    free(SA);
    free(invSA);
    free(LCP);

    free(C);

}
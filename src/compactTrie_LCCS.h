#ifndef COMPACTTRIE_H
#define COMPACTTRIE_H

#include <cstdint>

#include "compactTrie.h"
#ifdef _USE_64
typedef int64_t INT;
#endif

#ifdef _USE_32
typedef int32_t INT;

#endif
//#define BOOST_GEOMETRY_DONT_USE_POINT_XY
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/index/rtree.hpp>
namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;
using point3 = bg::model::point<INT, 3, bg::cs::cartesian>;

using point4 = bg::model::point<INT, 4, bg::cs::cartesian>;

#include <iostream>
#include <vector>
#include "node_LCCS.h"
#include <cstring>


// Structures used by the offline RMQ/LCA routine
struct LCAQuery {
    INT node_a_id;  // First node ID
    INT node_b_id;  // Second node ID
    INT result_id;  // Result node ID (filled by the offline algorithm)
};

//using namespace std;

unsigned char getZigZagChar(INT i, INT j, unsigned char* T, INT text_size);

 // Compute the maximum zigzag string length from position i, bounded by separators

INT getZigZagMaxLength(INT i, INT text_size, const vector<INT>& IdxSeparators);

class compactTrie {

public:
    unsigned char* T;
    INT text_size;
    INT numText;
    vector<pair<INT,INT>> *indices;
    explicit compactTrie(std::vector<pair<INT,INT>> &indices, std::vector<INT>& LCP, unsigned char* T,std::vector<INT>& IdxSeparators);
    Node * root;
    Node * forward_search( unsigned char* P, INT& pattern_size);

    // Compute CSS-related values (leafCount, CPLcount, duplicate, css) for the whole tree
    void computeCSS();

    // For each explicit node, build a pointer to the deepest subtree node with css >= tau
    void buildPointers(INT tau);

    void deleteTreeIteratively();
    ~compactTrie();

};


#endif

#ifndef COMPACTTRIE_H
#define COMPACTTRIE_H

#include <cstdint>
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
#include "node_CC.h"
#include <cstring>



//using namespace std;

unsigned char getZigZagChar(INT i, INT j, unsigned char* T, INT text_size);

class compactTrie {

public:
    unsigned char* T;
    INT text_size;
    vector<INT> *indices;
    explicit compactTrie(std::vector<INT> &indices, std::vector<INT>& LCP, unsigned char* T);
    Node * root;
    Node * forward_search( unsigned char* P, INT& pattern_size);

    void buildCC();

    void visualize(const std::string& filename);

    void deleteTreeIteratively();
    ~compactTrie();

};


#endif

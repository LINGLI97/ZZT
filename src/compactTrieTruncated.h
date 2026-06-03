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
using point3 = bg::model::point<uint64_t , 3, bg::cs::cartesian>;

using point4 = bg::model::point<uint64_t , 4, bg::cs::cartesian>;

#include <iostream>
#include <vector>
#include "node.h"
#include <cstring>



//using namespace std;

unsigned char getZigZagChar(INT i, INT j, unsigned char* T, INT text_size, INT Bound);

//class compactTrie {
//
//public:
//    INT B;
//    unsigned char* T;
//    INT text_size;
//    vector<INT> *indices;
//    explicit compactTrie(std::vector<INT> &indices, std::vector<INT>& LCP, unsigned char* T, INT B);
//    Node * root;
//    Node * forward_search( unsigned char* P, INT& pattern_size);
//    void initPreorder();
//    void initScore();
//
//    void addPoints(std::vector<point3> &points);
//    void addPoints(std::vector<point4> &points);
//
//    void deleteTreeIteratively();
//    ~compactTrie();
//
//};
//

#endif

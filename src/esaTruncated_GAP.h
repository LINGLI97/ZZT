
#ifndef ZIGZAG_ESA_H
#define ZIGZAG_ESA_H



//#define BOOST_GEOMETRY_DONT_USE_POINT_XY
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/index/rtree.hpp>
namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;
#ifdef _USE_64
#include <divsufsort64.h>                                         // include header for suffix sort
typedef int64_t INT;
#endif

#ifdef _USE_32
#include <divsufsort.h>                                           // include header for suffix sort
typedef int32_t INT;


#endif

using point3 = bg::model::point<uint64_t, 3, bg::cs::cartesian>;

using point4 = bg::model::point<uint64_t, 4, bg::cs::cartesian>;

#include <cstdint>




#include <iostream>
#include <vector>

#include <cstring>


#include <bits/stdc++.h>





#include <sdsl/suffix_trees.hpp>
#include "unordered_dense.h"

#include <sdsl/rmq_support.hpp>
#include <stack>


struct B {
    uint64_t id = 0;
    INT l = 0;
    INT r = 0;
    INT lcp = 0;
    INT parent = -1;
    INT cursor = 0;
//    std::vector<INT> ch;
    INT gap_sum = 0;
    std::vector<INT> occ;    // 升序的“出现位置”列表（如 SA[pos]）
    INT right_node = -1;
//    INT leaf_cnt = 0;
};

// 只声明你要暴露给外部调用的函数
unsigned int LCParray(unsigned char *text, INT n, std::vector<INT> &SA, INT *ISA, std::vector<INT> &LCP);

void construct_tuples(INT n, INT *SA, std::vector<B>& b, INT* LCP, INT Bound);

INT zigzag_length(INT i, INT n, INT Bound);
std::pair<INT,INT> pattern_matching_Z(unsigned char *  p, unsigned char *  T, INT* SA, INT* LCP, sdsl::rmq_succinct_sct<> &rmq, INT n, INT p_size, INT Bound);

#endif //ZIGZAG_ESA_H

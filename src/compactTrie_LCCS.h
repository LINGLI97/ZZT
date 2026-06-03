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


// Offline RMQ/LCA 相关结构
struct LCAQuery {
    INT node_a_id;  // 第一个节点ID
    INT node_b_id;  // 第二个节点ID
    INT result_id;  // 结果节点ID（由offline算法填充）
};

//using namespace std;

unsigned char getZigZagChar(INT i, INT j, unsigned char* T, INT text_size);

 // 计算从位置i开始的zigzag字符串的最大长度（受separator限制）

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

    // 对整棵树计算 CSS（leafCount, CPLcount, duplicate, css）
    void computeCSS();

    // 为每个explicit节点建立pointer，指向子树中满足css >= tau且深度最大的节点
    void buildPointers(INT tau);

    void deleteTreeIteratively();
    ~compactTrie();

};


#endif

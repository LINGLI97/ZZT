#ifndef NODE_H
#define NODE_H


#include <string>
#include <unordered_map>
#include <vector>
#include <unordered_set>
using namespace std;
#ifdef _USE_64
typedef int64_t INT;
#endif

#ifdef _USE_32
typedef int32_t INT;
#endif


class Node
{
public:
    INT start;
    INT depth;
    unsigned char label; // label is the label of edge which links its parent and itself
    Node * parent;
    Node();
    Node(unsigned char &terminate_label);
    Node( INT i, INT d, unsigned char &l );
    void setDepth( INT d);
//DFS



    // 叶子所属文本编号（颜色），内部结点设为 -1
    INT textID;

    INT leafCount;      // 子树中的叶子总数
    INT CPLcount;       // 作为 color-pair-lca 的次数
    INT duplicate;      // 子树中重复的叶子数
    INT css;            // 最终的 Color Set Size（不同颜色数）
    Node * ptr;
    INT id;

    map<unsigned char, Node*> child;


    Node * getChild( unsigned char l );

    void addChild( Node * childNode,  unsigned char &l );

    void setParent ( Node * parentNode);
    INT numChild();

    std::vector<Node*> allChild();


    ~Node();
};









#endif

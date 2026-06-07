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



    // Text ID (color) of the leaf; set to -1 for internal nodes
    INT textID;

    INT leafCount;      // Total number of leaves in the subtree
    INT CPLcount;       // Number of times it serves as a color-pair LCA
    INT duplicate;      // Number of duplicate leaves in the subtree
    INT css;            // Final Color Set Size (number of distinct colors)
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

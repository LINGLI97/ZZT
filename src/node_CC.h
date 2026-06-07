#ifndef NODE_H
#define NODE_H


#include <string>
#include <unordered_map>
#include <map>
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
    INT val_even;   // Number of contact points obtained by cutting at even depths from this node
    INT val_odd ;
    Node();
    Node(unsigned char &terminate_label);
    Node( INT i, INT d, unsigned char &l );

    void setDepth( INT d);

//DFS
    bool visited;



    unordered_map<unsigned char, Node*> child;


    Node * getChild( unsigned char l );

    void addChild( Node * childNode,  unsigned char &l );

    void setParent ( Node * parentNode);
    INT numChild();

    std::vector<Node*> allChild();


    ~Node();
};









#endif

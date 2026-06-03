#include <iostream>
#include <string>
#include <unordered_map>
#include "node_CC.h"

using namespace std;


Node::Node()
{
    this->start = 0;
    this->depth = 0;
    this->label = 0;
    this->parent = NULL;
    this->visited = false;
    this->val_even=0;
    this->val_odd=0;

}



Node::Node(unsigned char &terminate_label)
{
    this->start = 0;
    this->depth = 0;
    this->label = terminate_label;
    this->parent = NULL;
    this->visited = false;
    this->val_even=0;
    this->val_odd=0;


}

Node::Node( INT i, INT d, unsigned char &l)
{
    this->start = i;
    this->depth = d;
    this->label = l;
    this->parent = NULL;
    this->visited = false;
    this->val_even=0;
    this->val_odd=0;

}


void Node::setDepth( INT d){

    this->depth = d;
}



Node * Node::getChild( unsigned char l )
{

    unordered_map<unsigned char, Node*>::iterator it;



    it = this->child.find(l);
    if ( it == this->child.end() )
        return NULL;
    else
        return it->second;
}


void Node::addChild( Node * childNode, unsigned char &l)
{
    auto it = this->child.find( l );
    if ( it == this->child.end() )
    {
        pair <unsigned char, Node*> insertChild( l, childNode );
        this->child.insert( insertChild);
    }
    else
    {
        it->second = childNode;
    }
    childNode->label = l;
    childNode->parent = this;
}

void Node::setParent( Node * parentNode )
{
    this->parent = parentNode;
}


INT Node::numChild()
{
    if (this->child.empty()){
        return 0;
    }
    return this->child.size();
}




std::vector<Node*> Node::allChild() {
    std::vector<Node*> allChildren;
    for (auto& pair : this->child) {
        allChildren.push_back(pair.second);
    }
    return allChildren;
}



Node::~Node()
{



}


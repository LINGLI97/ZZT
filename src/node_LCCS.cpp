#include <iostream>
#include <string>
#include <map>
#include <unordered_map>

#include "node_LCCS.h"

using namespace std;


Node::Node()
{
    this->start = 0;
    this->depth = 0;
    this->label = 0;
    this->parent = NULL;
    this->textID = -1;
    this->leafCount = 0;
    this->CPLcount = 0;
    this->duplicate = 0;
    this->css = 0;

    this->id = -1;
    this->ptr = NULL;


}



Node::Node(unsigned char &terminate_label)
{
    this->start = 0;
    this->depth = 0;
    this->label = terminate_label;
    this->parent = NULL;
    this->textID = -1;
    this->leafCount = 0;
    this->CPLcount = 0;
    this->duplicate = 0;
    this->css = 0;
    this->id = -1;
    this->ptr = NULL;


}

Node::Node( INT i, INT d, unsigned char &l)
{
    this->start = i;
    this->depth = d;
    this->label = l;
    this->parent = NULL;
    this->textID = -1;
    this->leafCount = 0;
    this->CPLcount = 0;
    this->duplicate = 0;
    this->css = 0;
    this->id = -1;
    this->ptr = NULL;

}


void Node::setDepth( INT d){

    this->depth = d;
}



Node * Node::getChild( unsigned char l )
{

    map<unsigned char, Node*>::iterator it;



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


#include <iostream>
#include <string>
#include <stack>
#include <cstring>
#include <cctype>
#include <unordered_set>
#include <fstream>
#include "suffixTree.h"

using namespace std;



suffixTree::suffixTree(unsigned char* T, const INT &text_size)
{
//    size_t memory_start = memory_usage()*0.001;

    this->T = T;
    this->n = text_size;


    //set the value of terminate_label

    this->root = new stNode();
    this->root->setSLink( this->root);



    stNode * u = this->root;
    int d = 0;


    for ( INT i = 0; i < this->n; i++ )
    {
        while (((i+d) < this -> n) && (d == u->depth) && (u->getChild(T[i + d]) != NULL)) {


            u = u->getChild(T[i + d]);
            d = d + 1;
            while ((i + d < this->n) && (u->start +d < this->n) && (d < u->depth) && (T[u->start + d] == T[i + d])) {
                d = d + 1;
            }
        }

        if ( d < u->depth)
        {
            u= createNode (u, d );
        }
        createLeaf( i, u, d);

        if ( u->slink == NULL )
        {
            ComputeSuffixLink( u );
        }
        u = u->slink;

//        d = u->getDepth(); // same as d = max( d-1, 0 );
        d = max( d-1, 0 );


    }
    //the only child of root



//    memory = memory_usage()*0.001 - memory_start;

}



//void suffixTree::getAll_ul(unordered_map<stNode *, stNode *> &result) {
//    std::stack<stNode*> stack;
//    stack.push(root);
//
//    while (!stack.empty()) {
//        stNode* node = stack.top();
//        stack.pop();
//
//        // If the current node is not heavy, it's a light node, start a new heavy path
//        if (!node->heavy) {
//            stNode* current = node;
//            stNode* heavyLeaf = nullptr;
//
//            // Traverse along the heavy path to find the corresponding heavy leaf
//            while (current && current->heavy) {
//                bool hasHeavyChild = false;
//                for (auto &it : current->child) {
//                    if (it.second->heavy) {
//                        current = it.second;
//                        hasHeavyChild = true;
//                        break;
//                    }
//                }
//
//                // If no heavy child, we have found the heavy leaf
//                if (!hasHeavyChild) {
//                    heavyLeaf = current;
//                    break;
//                }
//            }
//
//            // If the light node is also a leaf, it is its own heavy leaf
//            if (node->child.empty()) {
//                heavyLeaf = node;
//            }
//
//            // If a heavy leaf was found, map the light node to this heavy leaf
//            if (heavyLeaf) {
//                result[node] = heavyLeaf;
//            }
//        }
//
//        // Push all children to the stack for further processing
//        for (auto &it : node->child) {
//            stack.push(it.second);
//        }
//    }
//
//}




void suffixTree::ComputeSuffixLink( stNode * u )
{
    INT d = u->depth;
    stNode * v = u->parent->slink;

    while ( v->depth < d-1 )
    {
        // go down
        v = v->getChild( this->T[u->start + v->depth + 1] );
    }
    if ( v->depth > d-1 )
    {
        //create a new node as a suffix link node
        v = createNode( v, d-1);
    }
    u->setSLink (v);
//    u-> slink->setDepth(d -1);
}

stNode * suffixTree::createNode(stNode * u, INT d )
{

    // add a new node v between p and u
    INT i = u->start;
    stNode * p = u->parent;


    stNode * v = new stNode( i, d,  this->T[i+p->depth]);
    v->addChild( u, this->T[i+d]);
    p->addChild( v,  this->T[i+p->depth]);

    return v;
}


void suffixTree::createLeaf( INT i, stNode * u, INT d )
{
    // create a leaf node connected to u
    // (n+2)(n+1)+1 represents $

    INT depth = this->n-i ;
    stNode *leaf  = new stNode(i, depth, this->T[i+d]) ;
    u->addChild( leaf, this->T[i+d]);
//    pos2leaf.insert({i, leaf});


}



void suffixTree::deleteTreeIteratively() {
    std::stack<stNode*> toDelete;
    toDelete.push(root);

    while (!toDelete.empty()) {
        stNode* current = toDelete.top();
        toDelete.pop();

        for (auto it = current->child.begin(); it != current->child.end();) {
            if (it->second != nullptr){
                toDelete.push(it->second);
                it->second = nullptr;

            }
            it = current->child.erase(it);
        }
        delete current;

    }
}



stNode *suffixTree::forward_search(unsigned char *P, INT& pattern_size){

    INT d = 0;
    stNode * u = this->root;
    stNode * v = NULL;
    bool match = true;
    while ( match )
    {
        v = u->getChild( P[d] );
        if ( v == NULL )
        {
            match = false;
            break;
        }
        INT i, j;
        for ( i = d, j = d; i < pattern_size && j < v->depth ; i++, j++ )
        {
//            cout<<"P["<<i<<"]"<<P[i]<<endl;
//            cout<<"T["<<v->start+j<<"]"<<this->T[v->start+j]<<endl;

            if ( P[i] != this->T[v->start+j] )
            {
                match = false;
                break;
            }
        }
        d = v->depth;
        u = v;
        if ( match && d >= pattern_size )
        {
//            INT tmp = u->start+ pattern_size-1;
            return u;

        }
    }
    return nullptr;

}



// Recursive helper: emit DOT nodes and edges for the subtree rooted at `node`.
// Node style:
//   label   = start / depth  (+ leafCount + deepPtr info)
//   green   = tau-frequent (leafCount >= tau)
//   grey    = not tau-frequent
//   red border = this node is a deepPtr target of some other node
// Edge label = actual text substring (up to 10 chars, then "..")
// Dashed orange arrow = deepPtr pointer (omitted when deepPtr == node itself)
void suffixTree::generateDot(stNode* node, std::ofstream& dotFile,
                              INT tau, std::unordered_set<stNode*>& targets) {
    if (!node) return;

    bool isFrequent = (node->leafCount >= tau);
    bool isTarget   = targets.count(node) > 0;

    // ---- node label ----
    std::string label;
    if (node->parent == nullptr) label = "ROOT\\n";
    label += "start=" + std::to_string(node->start)
           + "  depth=" + std::to_string(node->depth)
           + "\\nlc=" + std::to_string(node->leafCount);
    if (node->deepPtr)
        label += "\\ndp->(" + std::to_string(node->deepPtr->start)
               + "," + std::to_string(node->deepPtr->depth) + ")";
    else
        label += "\\ndp->null";

    // ---- node style ----
    std::string fillcolor = isFrequent ? "#90EE90" : "#EEEEEE";
    std::string color     = isTarget   ? "red"     : "black";
    std::string penwidth  = isTarget   ? "3"       : "1";

    dotFile << "\"" << node << "\" [label=\"" << label
            << "\", fillcolor=\"" << fillcolor
            << "\", color=\""     << color
            << "\", penwidth="    << penwidth << "];\n";

    // ---- tree edges to children ----
    for (auto& kv : node->child) {
        stNode* child = kv.second;

        // Edge substring: T[child->start + node->depth .. child->start + child->depth - 1]
        std::string edgeStr;
        INT edgeStart = child->start + node->depth;
        INT edgeEnd   = child->start + child->depth - 1;
        for (INT k = edgeStart; k <= edgeEnd; ++k) {
            unsigned char c = T[k];
            if      (c == '$')          edgeStr += '$';
            else if (std::isprint(c))   edgeStr += (char)c;
            else                        edgeStr += '?';
            if ((INT)edgeStr.size() >= 10) { edgeStr += ".."; break; }
        }

        dotFile << "\"" << node << "\" -> \"" << child
                << "\" [label=\"" << edgeStr << "\"];\n";

        generateDot(child, dotFile, tau, targets);
    }

    // ---- deepPtr dashed arrow (skip self-loops) ----
    if (node->deepPtr && node->deepPtr != node) {
        dotFile << "\"" << node << "\" -> \"" << node->deepPtr
                << "\" [style=dashed, color=orange, constraint=false,"
                   " label=\"dp\", fontcolor=orange];\n";
    }
}

void suffixTree::exportSuffixTreeToDot(const std::string& filename, INT tau) {
    std::ofstream dotFile(filename);
    if (!dotFile.is_open()) {
        std::cerr << "Unable to open file for writing: " << filename << "\n";
        return;
    }

    dotFile << "digraph SuffixTree {\n";
    dotFile << "node [fontname=\"Courier\", shape=box, style=filled];\n";
    dotFile << "edge [fontsize=10, color=black];\n";
    dotFile << "graph [nodesep=0.5, ranksep=1, splines=polyline];\n";

    // Collect all deepPtr targets (for red-border highlighting)
    std::unordered_set<stNode*> targets;
    {
        std::stack<stNode*> st;
        st.push(root);
        while (!st.empty()) {
            stNode* node = st.top(); st.pop();
            if (node->deepPtr) targets.insert(node->deepPtr);
            for (auto& kv : node->child) st.push(kv.second);
        }
    }

    generateDot(root, dotFile, tau, targets);

    dotFile << "}\n";
    dotFile.close();
    std::cout << "Exported DOT: " << filename << "\n";
}


suffixTree::~suffixTree() {

//    delete this->root;
    deleteTreeIteratively();


}

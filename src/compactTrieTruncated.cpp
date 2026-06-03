#include "compactTrieTruncated.h"

using namespace std;

unsigned char getZigZagChar(INT i, INT j, unsigned char* T, INT text_size, INT Bound) {

    if (j + 1 >Bound){
        return 256 + i;
    }

    INT cnt = (j + 1) / 2;
    INT index_T;
    if (j % 2 == 0) {
        index_T = i - cnt;
    } else {
        index_T = i + cnt;
    }

    if (index_T < 0 || index_T >= text_size) {
        return 255; // sentinel symbol
    }

    return T[index_T];
}

//compactTrie::compactTrie(std::vector<INT> &indices, vector<INT>& LCP,unsigned char* T, INT B){
//
//    this->T = T;
//    this->B = B;
//    this->text_size =  indices.size();
//    this->indices = &indices;
//    this->root = new Node();
//    Node * current = this->root;
//    INT d = 0;
//
//
//    INT lcp_value =0;
//    for (INT i = 0; i < text_size; i++) {
//        if (i != 0){
//            lcp_value = LCP[i];    // LCP value between current and previous suffix
//        }
//        Node* currentChild;
//        // Walk back to the right node based on the LCP value
//        while (current->depth > lcp_value) {
//            currentChild = current;
//            current = current->parent;
//        }
//
//        if (current->depth == lcp_value){
//
//            unsigned char new_node_label = getZigZagChar(indices[i],lcp_value, this->T, this->text_size, B);
//
//            Node* new_node = new Node(i, INT_MAX, new_node_label);
//            new_node->parent = current;
////            new_node->phi = prefixesStarting[i].second;
//            current->addChild(new_node, new_node_label);
//
//            current = new_node;
//
//
//        } else{
//
//
//            unsigned char v_label = getZigZagChar(indices[i-1],current->depth, this->T, this->text_size,B);
//
//            unsigned char currentChild_label = getZigZagChar(indices[i-1], lcp_value, this->T, this->text_size,B);
//            unsigned char x_label = getZigZagChar(indices[i], lcp_value, this->T, this->text_size,B);
//
//            //add the branch node v
//            Node * v = new Node( i-1, lcp_value, v_label);
//            current->addChild( v, v_label);
//            v->addChild( currentChild, currentChild_label);
//
//            current = v;
//
//            // add the other leaf x
//            Node* x = new Node(i, INT_MAX, x_label);
//
////            Node * x = new Node( suffix_start, DS.text_size - suffix_start, x_label);
////            x->phi = prefixesStarting[i].second;
//            current->addChild(x, x_label);
//            current = x;
//        }
//        // Move current to the new node
//
//
//    }
//}
//
//
//
//
//Node *compactTrie::forward_search(unsigned char *P, INT& pattern_size){
//
//    INT d = 0;
//    Node * u = this->root;
//    Node * v = NULL;
//    bool match = true;
//    while ( match )
//    {
//        v = u->getChild( P[d] );
//        if ( v == NULL )
//        {
//            match = false;
//            break;
//        }
//        INT i, j;
//        for ( i = d, j = d; i < pattern_size && j < v->depth ; i++, j++ )
//        {
////            cout<<"P["<<i<<"]"<<P[i]<<endl;
////            cout<<"T["<<v->start+j<<"]"<<this->T[v->start+j]<<endl;
//
////            if ( P[i] != this->T[v->start+j] )
//
//
//            if ( P[i] != getZigZagChar((*this->indices)[v->start], j, this->T, this->text_size,B) )
//            {
//                match = false;
//                break;
//            }
//        }
//        d = v->depth;
//        u = v;
//        if ( d >= pattern_size )
//        {
////            INT tmp = u->start+ pattern_size-1;
//            return u;
//
//        }
//    }
//    return nullptr;
//
//}
//
//
//
//void compactTrie::initPreorder(){
//    std::stack<Node *> stack;
//    stack.push(root);
//    INT preorderCounter = 0;
//
//    while (!stack.empty()) {
//        Node *top = stack.top();
//        stack.pop();
//
//        top->preorderId = preorderCounter++;
//
//        for (auto it = top->child.rbegin(); it != top->child.rend(); ++it) {
//            stack.push(it->second);
//        }
//    }
//}
//
//
//void compactTrie::initScore(){
//    std::stack<Node *> stack;
//    stack.push(root);
//
//    while (!stack.empty()) {
//        Node *top = stack.top();
//
//
//        if (top->visited){
//
//            if (top->allChild().empty()){
//                top->score = 1;
//            } else{
//                top->score = 0;
//                for (auto it: top->child) {
//                    top->score += it.second->score;
//                }
//            }
//            stack.pop();
//        } else{
//
//            top->visited = true;
//            for (auto it: top->child) {
//                stack.push(it.second);
//            }
//        }
//    }
//
//
//}
//
//void compactTrie::addPoints(vector<point3> &points) {
//    std::stack<Node *> stack;
//    stack.push(root);
//    while (!stack.empty()) {
//        Node *top = stack.top();
//
//        stack.pop();  // Pop before size calculation to avoid double counting
//
//        if (top->parent){
//
//#ifdef VERBOSE
//            std::cout<<"add the point: "<<top->preorderId<<", "<<top->depth<<", "<<top->parent->depth + 1<< endl;
//#endif
//            point3 tuple_3D;
//            bg::set<0>(tuple_3D, top->preorderId);
//            bg::set<1>(tuple_3D, top->depth);
//            bg::set<2>(tuple_3D, top->parent->depth + 1);
//            points.emplace_back(tuple_3D);
//        }
//
//        for (auto &it: top->child) {
//            stack.push(it.second);
//        }
//
//
//    }
//
//
//
//}
//
//
//
//void compactTrie::addPoints(vector<point4> &points) {
//    std::stack<Node *> stack;
//    stack.push(root);
//    while (!stack.empty()) {
//        Node *top = stack.top();
//
//        stack.pop();  // Pop before size calculation to avoid double counting
//
//        if (top->parent){
//
//#ifdef VERBOSE
//            std::cout<<"add the point: "<<top->preorderId<<", "<<top->depth<<", "<<top->parent->depth + 1<< endl;
//#endif
//            point4 tuple_4D;
//            bg::set<0>(tuple_4D, top->preorderId);
//            bg::set<1>(tuple_4D, top->depth);
//            bg::set<2>(tuple_4D, top->parent->depth + 1);
//            bg::set<3>(tuple_4D, top->score);
//
//            points.emplace_back(tuple_4D);
//        }
//
//        for (auto &it: top->child) {
//            stack.push(it.second);
//        }
//
//
//    }
//
//
//
//}
//
//
//void compactTrie::deleteTreeIteratively() {
//    std::stack<Node*> toDelete;
//    toDelete.push(root);
//
//    while (!toDelete.empty()) {
//        Node* current = toDelete.top();
//        toDelete.pop();
//
//        for (auto it = current->child.begin(); it != current->child.end();) {
//            if (it->second != nullptr){
//                toDelete.push(it->second);
//                it->second = nullptr;
//
//            }
//            it = current->child.erase(it);
//        }
//        delete current;
//
//    }
//}
//
//
//
//
//compactTrie::~compactTrie(){
//
//    deleteTreeIteratively();
//}

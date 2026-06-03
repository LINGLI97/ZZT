#include "compactTrie_LFCS.h"


using namespace std;

unsigned char getZigZagChar(INT i, INT j, unsigned char* T, INT text_size) {
    INT cnt = (j + 1) / 2;
    INT index_T;
    if (j % 2 == 0) {
        index_T = i - cnt;
    } else {
        index_T = i + cnt;
    }

    if (index_T < 0 || index_T >= text_size) {
        return 255 ; // sentinel symbol
    }

    return T[index_T];
}

compactTrie::compactTrie(std::vector<INT> &indices, vector<INT>& LCP,unsigned char* T){

    this->T = T;
    this->text_size =  indices.size();
    this->indices = &indices;
    this->root = new Node();
    Node * current = this->root;
    INT d = 0;


    INT lcp_value =0;
    for (INT i = 0; i < text_size; i++) {
        if (i != 0){
            lcp_value = LCP[i];    // LCP value between current and previous suffix
        }
        Node* currentChild;
        // Walk back to the right node based on the LCP value
        while (current->depth > lcp_value) {
            currentChild = current;
            current = current->parent;
        }

        if (current->depth == lcp_value){

            unsigned char new_node_label = getZigZagChar(indices[i],lcp_value, this->T, this->text_size);

            INT actual_depth_new = std::min((INT)(2*(indices[i]+1)), (INT)(2*(this->text_size - indices[i]) - 1));
            Node* new_node = new Node(indices[i], actual_depth_new, new_node_label);
            new_node->parent = current;
//            new_node->phi = prefixesStarting[i].second;
            current->addChild(new_node, new_node_label);

            current = new_node;


        } else{


            unsigned char v_label = getZigZagChar(indices[i-1],current->depth, this->T, this->text_size);

            unsigned char currentChild_label = getZigZagChar(indices[i-1], lcp_value, this->T, this->text_size);
            unsigned char x_label = getZigZagChar(indices[i], lcp_value, this->T, this->text_size);

            //add the branch node v
            Node * v = new Node(indices[i-1], lcp_value, v_label);
            current->addChild( v, v_label);
            v->addChild( currentChild, currentChild_label);

            current = v;

            // add the other leaf x
            INT actual_depth_x = std::min((INT)(2*(indices[i]+1)), (INT)(2*(this->text_size - indices[i]) - 1));
            Node* x = new Node(indices[i], actual_depth_x, x_label);

//            Node * x = new Node( suffix_start, DS.text_size - suffix_start, x_label);
//            x->phi = prefixesStarting[i].second;
            current->addChild(x, x_label);
            current = x;
        }
        // Move current to the new node


    }
}

void compactTrie::buildLFCS(INT tau) {
    std::stack<Node*> st;
    st.push(root);

    while(!st.empty())
    {
        Node* v = st.top();

        if(!v->visited)
        {
            // first time reach v
            v->visited = true;

            // push children
            for(auto &kv : v->child)
                st.push(kv.second);
        }
        else
        {
            // second time reach v -> postorder
            st.pop();

            if(v->child.empty())
                v->leafCount = 1;
            else{
                INT sum = 0;
                for(auto &kv: v->child)
                    sum += kv.second->leafCount;
                v->leafCount = sum;
            }

            // 找子树中最深的满足条件的节点
            Node* best = NULL;
            INT bestDepth = -1;

            // ✅ 检查所有孩子及其子树
            for(auto &kv: v->child)
            {
                Node* child = kv.second;

                // 情况1: child itself
                if(child->leafCount >= tau && child->depth > bestDepth){
                    bestDepth = child->depth;
                    best = child;
                }

                // 情况2: check the subtree of child if child->frequentPtr != child
                if (child->frequentPtr != child)
                {
                    if(child->frequentPtr && child->frequentPtr->depth > bestDepth){
                        bestDepth = child->frequentPtr->depth;
                        best = child->frequentPtr;
                    }

                }

            }

            // ✅ 检查 v 本身
            if(v->leafCount >= tau && v->depth > bestDepth){
                bestDepth = v->depth;
                best = v;
            }

            v->frequentPtr = best;
        }
    }
}




Node *compactTrie::forward_search(unsigned char *P, INT& pattern_size){

    INT d = 0;
    Node * u = this->root;
    Node * v = NULL;
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
            // cout<<"P["<<i<<"]: "<<P[i]<<endl;
            // cout<<getZigZagChar(v->start, j, this->T, this->text_size)<<endl;

//            cout<<"T["<<v->start+j<<"]"<<this->T[v->start+j]<<endl;

//            if ( P[i] != this->T[v->start+j] )


            if ( P[i] != getZigZagChar(v->start, j, this->T, this->text_size) )
            {
                match = false;
                break;
            }
        }
        if ( !match ) break;
        d = v->depth;
        u = v;
        if ( d >= pattern_size )
        {
//            INT tmp = u->start+ pattern_size-1;
            return u;

        }
    }
    return nullptr;

}




void compactTrie::deleteTreeIteratively() {
    std::stack<Node*> toDelete;
    toDelete.push(root);

    while (!toDelete.empty()) {
        Node* current = toDelete.top();
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




compactTrie::~compactTrie(){

    deleteTreeIteratively();
}

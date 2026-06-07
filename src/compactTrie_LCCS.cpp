#include "compactTrie_LCCS.h"

#include "compactTrie.h"
#include "compactTrie.h"
#include "compactTrie.h"
#include "compactTrie.h"
#include "compactTrie.h"

#include "rmq-offline.h"
using namespace std;

unsigned char getZigZagChar(INT i, INT j, unsigned char* T, INT text_size) {
    INT cnt = (j + 1) / 2;
    INT index_T;
    if (j % 2 == 0) {
        index_T = i - cnt;
    } else {
        index_T = i + cnt;
    }

    // If out of bounds, return 255 as the boundary marker
    if (index_T < 0 || index_T >= text_size) {
        return 255; // sentinel symbol for boundary
    }

    // Return the character directly, including separators
    return T[index_T];
}

// Compute the maximum zigzag string length starting at position i, bounded by separators
INT getZigZagMaxLength(INT i, INT text_size, const vector<INT>& IdxSeparators) {
    // Find the left and right boundaries of the text containing position i
    INT left_bound = -1;
    INT right_bound = text_size;

    if (!IdxSeparators.empty()) {
        // Use binary search to find the first separator >= i
        auto it = std::lower_bound(IdxSeparators.begin(), IdxSeparators.end(), i);

        // If i is exactly a separator position, it cannot be a starting point
        if (it != IdxSeparators.end() && *it == i) {
            return 1;
        }

        // Find the nearest separator to the left of i (the last one < i)
        if (it != IdxSeparators.begin()) {
            --it;
            left_bound = *it ;
            ++it;
        }

        // Find the nearest separator to the right of i (the first one > i)
        if (it != IdxSeparators.end()) {
            right_bound = *it ;
        }
    }

    // Use the formula to compute the maximum reachable zigzag length.
    // The zigzag starts at i and expands alternately to the left and right.

    INT left_steps = i - left_bound;     // Number of steps available to the left
    INT right_steps = right_bound - i -1;   // Number of steps available to the right
    INT steps = std::min(left_steps, right_steps);

    // Length = initial position 1 + left and right expansions of 'steps' each.
    // If one side still has remaining characters, add 1 more.
    INT length = 0;
    if (left_steps <= right_steps) {
        length = 2 * steps + 1;
    } else {
        length = 2 * steps + 2;
    }

    return length;
}

compactTrie::compactTrie(std::vector<pair<INT,INT>> &indices, vector<INT>& LCP,unsigned char* T, std::vector<INT>& IdxSeparators){

    this->T = T;
    this->text_size =  indices.size();
    this->indices = &indices;
    this->numText = IdxSeparators.size() + 1;

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

            unsigned char new_node_label = getZigZagChar(indices[i].first,lcp_value, this->T, this->text_size);

            INT newNodeLength = getZigZagMaxLength(indices[i].first, this->text_size, IdxSeparators);
            Node* new_node = new Node(indices[i].first, newNodeLength, new_node_label);
            new_node->parent = current;
            new_node->textID = indices[i].second;  // Store the text ID of the leaf
//            new_node->phi = prefixesStarting[i].second;
            current->addChild(new_node, new_node_label);

            current = new_node;


        } else{


            unsigned char v_label = getZigZagChar(indices[i-1].first,current->depth, this->T, this->text_size);

            unsigned char currentChild_label = getZigZagChar(indices[i-1].first, lcp_value, this->T, this->text_size);
            unsigned char x_label = getZigZagChar(indices[i].first, lcp_value, this->T, this->text_size);

            //add the branch node v

            Node * v = new Node( indices[i-1].first, lcp_value, v_label);
            current->addChild( v, v_label);

            v->addChild( currentChild, currentChild_label);

            current = v;

            // add the other leaf x
            INT lengthX = getZigZagMaxLength(indices[i].first, this->text_size, IdxSeparators);
            Node* x = new Node(indices[i].first, lengthX, x_label);

            x->textID = indices[i].second;  // Store the text ID of the leaf

//            Node * x = new Node( suffix_start, DS.text_size - suffix_start, x_label);
//            x->phi = prefixesStarting[i].second;
            current->addChild(x, x_label);
            current = x;
        }
        // Move current to the new node


    }
}



// ===== buildPointers =====
void compactTrie::buildPointers(INT tau) {
    // For each node, use DFS to find the deepest subtree node with css >= tau.
    // This is done with a post-order traversal from the leaves upward.

    stack<pair<Node*, bool>> st;
    st.push({root, false});

    while (!st.empty()) {
        auto [u, visited] = st.top();
        st.pop();

        if (visited) {
            // Post-order phase: process the current node
            Node* maxDepthNode = nullptr;
            INT maxDepth = -1;

            // First check whether the current node itself satisfies the condition
            if (u->css >= tau) {
                maxDepthNode = u;
                maxDepth = u->depth;
            }

            // Check the pointers of all child nodes
            for (auto &kv : u->child) {
                Node* child = kv.second;
                if (child->ptr != nullptr) {
                    // The child has a pointer; inspect the pointed node
                    if (child->ptr->depth > maxDepth) {
                        maxDepth = child->ptr->depth;
                        maxDepthNode = child->ptr;
                    }
                }
            }

            // Set the pointer for the current node
            u->ptr = maxDepthNode;

        } else {
            // Pre-order phase: push the node again for later processing, then push all children
            st.push({u, true});
            for (auto it = u->child.rbegin(); it != u->child.rend(); ++it) {
                st.push({it->second, false});
            }
        }
    }
}

// ===== CSS related =====
void compactTrie::computeCSS()
{
    // ========== Single DFS: complete all prerequisite computations ==========
    vector<Node*> leafList;
    vector<Node*> allNodes;
    unordered_map<INT, INT> lastleaf_map;   // leaf_id -> prev_same_color_leaf_id

    {
        unordered_map<INT, INT> last_seen;      // color -> last_leaf_id

        stack<pair<Node*, bool>> st;
        st.push({root, false});
        INT cur_id = 0;

        while (!st.empty()) {
            auto [u, visited] = st.top();
            st.pop();

            if (visited) {
                // Post-order phase
                u->id = cur_id++;
                allNodes.push_back(u);

                if (u->child.empty()) {
                    // Leaf node
                    u->leafCount = 1;
                    leafList.push_back(u);

                    // Compute lastleaf at the same time
                    INT xid = u->id;
                    INT color = u->textID;

                    auto it = last_seen.find(color);
                    lastleaf_map[xid] = (it == last_seen.end()) ? -1 : it->second;
                    last_seen[color] = xid;

                } else {
                    // Internal node
                    INT sum = 0;
                    for (auto &kv : u->child) {
                        sum += kv.second->leafCount;
                    }
                    u->leafCount = sum;
                }
            } else {
                st.push({u, true});
                for (auto it = u->child.rbegin(); it != u->child.rend(); ++it) {
                    st.push({it->second, false});
                }
            }
        }
    }

    INT n = (INT)allNodes.size();



    // ========== Build the Euler tour ==========
    INT s = 2 * n - 1;  // Euler tour length
    vector<INT> E;   // Euler tour: sequence of node IDs
    vector<INT> L;   // Levels: sequence of depths
    E.reserve(s);
    L.reserve(s);
    vector<INT> R(n, -1);   // Representative: first occurrence position of each node

    {
        stack<tuple<Node*, INT, INT>> st;  // (node, depth, child_idx)
        st.push({root, 0, 0});

        while (!st.empty()) {
            auto [u, depth, child_idx] = st.top();
            st.pop();

            INT uid = u->id;

            // Record the Euler tour
            if (R[uid] == -1) {
                R[uid] = (INT)E.size();  // First occurrence position
            }
            E.push_back(uid);
            L.push_back(depth);

            // Retrieve the child list
            vector<Node*> children;
            for (auto &kv : u->child) {
                children.push_back(kv.second);
            }

            // If there are still unvisited child nodes
            if (child_idx < (INT)children.size()) {
                // Push the parent again so it is revisited after returning from the child
                st.push({u, depth, child_idx + 1});
                // Visit the next child
                st.push({children[child_idx], depth + 1, 0});
            }
        }
    }
    // ========== Prepare LCA queries ==========
    INT q = leafList.size() - this->numText;


    // Build the query array
    vector<INT> lca_results(q);  // Store results only
    {

    vector<Query> Q_lca(q);
    vector<Query> Q_rmq(q);
    INT qidx = 0;

    for (Node* x : leafList) {
        INT xid = x->id;
        INT prev = lastleaf_map[xid];
        if (prev == -1) continue;

        Q_lca[qidx].L = xid;
        Q_lca[qidx].R = prev;

        // Convert to an RMQ query
        if (R[xid] < R[prev]) {
            Q_rmq[qidx].L = R[xid];
            Q_rmq[qidx].R = R[prev];
        } else {
            Q_rmq[qidx].L = R[prev];
            Q_rmq[qidx].R = R[xid];
        }
        qidx++;
    }

    // ========== Run the offline RMQ algorithm ==========
    rmq_offline(L.data(), s, Q_rmq.data(), q);


    for (INT i = 0; i < q; i++) {
        lca_results[i] = E[Q_rmq[i].O];
    }

    }

    // Release large arrays immediately
    E.clear(); E.shrink_to_fit();
    L.clear(); L.shrink_to_fit();
    R.clear(); R.shrink_to_fit();


    // ========== Compute CPLcount ==========

    for (INT i = 0; i < q; i++) {
        INT lca_node = lca_results[i];  // Use the stored result
        allNodes[lca_node]->CPLcount++;
    }



    // ========== Compute duplicate ==========
    for (Node* u : allNodes) {
        INT sum = u->CPLcount;
        for (auto &kv : u->child) {
            sum += kv.second->duplicate;
        }
        u->duplicate = sum;
    }

    // ========== Compute css ==========
    for (Node* u : allNodes) {
        u->css = u->leafCount - u->duplicate;
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
            // cout<<"P["<<i<<"]"<<P[i]<<endl;
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

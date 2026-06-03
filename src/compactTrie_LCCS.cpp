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

    // 如果越界，返回255作为边界标记
    if (index_T < 0 || index_T >= text_size) {
        return 255; // sentinel symbol for boundary
    }

    // 直接返回字符（包括separator字符本身）
    return T[index_T];
}

// 计算从位置i开始的zigzag字符串的最大长度（受separator限制）
INT getZigZagMaxLength(INT i, INT text_size, const vector<INT>& IdxSeparators) {
    // 找到位置i所在文本的左右边界
    INT left_bound = -1;
    INT right_bound = text_size;

    if (!IdxSeparators.empty()) {
        // 使用二分查找找到第一个 >= i 的separator
        auto it = std::lower_bound(IdxSeparators.begin(), IdxSeparators.end(), i);

        // 如果i正好是separator位置，则i不能作为起点
        if (it != IdxSeparators.end() && *it == i) {
            return 1;
        }

        // 找到i左边最近的separator (最后一个 < i 的)
        if (it != IdxSeparators.begin()) {
            --it;
            left_bound = *it ;
            ++it;
        }

        // 找到i右边最近的separator (第一个 > i 的)
        if (it != IdxSeparators.end()) {
            right_bound = *it ;
        }
    }

    // 使用数学公式计算zigzag能达到的最大长度
    // zigzag从i开始，向左右交替扩展

    INT left_steps = i - left_bound;     // 能向左走的步数
    INT right_steps = right_bound - i -1;   // 能向右走的步数
    INT steps = std::min(left_steps, right_steps);

    // 长度 = 初始位置1 + 向右向左各steps步
    // 如果一边还有剩余，再加1
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
            new_node->textID = indices[i].second;  // 存储叶子节点的text ID
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

            x->textID = indices[i].second;  // 存储叶子节点的text ID

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
    // 对每个节点，使用DFS找到其子树中满足css >= tau且深度最大的节点
    // 使用后序遍历，从叶子向上处理

    stack<pair<Node*, bool>> st;
    st.push({root, false});

    while (!st.empty()) {
        auto [u, visited] = st.top();
        st.pop();

        if (visited) {
            // 后序位置：处理当前节点
            Node* maxDepthNode = nullptr;
            INT maxDepth = -1;

            // 首先检查当前节点自己是否满足条件
            if (u->css >= tau) {
                maxDepthNode = u;
                maxDepth = u->depth;
            }

            // 检查所有子节点的pointer
            for (auto &kv : u->child) {
                Node* child = kv.second;
                if (child->ptr != nullptr) {
                    // 子节点有pointer，检查其指向的节点
                    if (child->ptr->depth > maxDepth) {
                        maxDepth = child->ptr->depth;
                        maxDepthNode = child->ptr;
                    }
                }
            }

            // 设置当前节点的pointer
            u->ptr = maxDepthNode;

        } else {
            // 前序位置：将节点再次入栈（后续处理），然后压入所有子节点
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
    // ========== 单次DFS：完成所有前置计算 ==========
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
                // 后序位置
                u->id = cur_id++;
                allNodes.push_back(u);

                if (u->child.empty()) {
                    // 叶子节点
                    u->leafCount = 1;
                    leafList.push_back(u);

                    // 同时计算 lastleaf
                    INT xid = u->id;
                    INT color = u->textID;

                    auto it = last_seen.find(color);
                    lastleaf_map[xid] = (it == last_seen.end()) ? -1 : it->second;
                    last_seen[color] = xid;

                } else {
                    // 内部节点
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



    // ========== 构建欧拉游历 (Euler Tour) ==========
    INT s = 2 * n - 1;  // 欧拉游历长度
    vector<INT> E;   // Euler tour: 节点ID序列
    vector<INT> L;   // Levels: 深度序列
    E.reserve(s);
    L.reserve(s);
    vector<INT> R(n, -1);   // Representative: 每个节点首次出现位置

    {
        stack<tuple<Node*, INT, INT>> st;  // (node, depth, child_idx)
        st.push({root, 0, 0});

        while (!st.empty()) {
            auto [u, depth, child_idx] = st.top();
            st.pop();

            INT uid = u->id;

            // 记录欧拉游历
            if (R[uid] == -1) {
                R[uid] = (INT)E.size();  // 首次出现位置
            }
            E.push_back(uid);
            L.push_back(depth);

            // 获取子节点列表
            vector<Node*> children;
            for (auto &kv : u->child) {
                children.push_back(kv.second);
            }

            // 如果还有未访问的子节点
            if (child_idx < (INT)children.size()) {
                // 父节点再次入栈（稍后从子节点返回时会再次访问）
                st.push({u, depth, child_idx + 1});
                // 访问下一个子节点
                st.push({children[child_idx], depth + 1, 0});
            }
        }
    }
    // ========== 准备LCA查询 ==========
    INT q = leafList.size() - this->numText;


    // 构建查询数组
    vector<INT> lca_results(q);  // ← 只保存结果
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

        // 转换为RMQ查询
        if (R[xid] < R[prev]) {
            Q_rmq[qidx].L = R[xid];
            Q_rmq[qidx].R = R[prev];
        } else {
            Q_rmq[qidx].L = R[prev];
            Q_rmq[qidx].R = R[xid];
        }
        qidx++;
    }

    // ========== 调用RMQ离线算法 ==========
    rmq_offline(L.data(), s, Q_rmq.data(), q);


    for (INT i = 0; i < q; i++) {
        lca_results[i] = E[Q_rmq[i].O];
    }

    }

    // 立即释放大数组
    E.clear(); E.shrink_to_fit();
    L.clear(); L.shrink_to_fit();
    R.clear(); R.shrink_to_fit();


    // ========== 计算 CPLcount ==========

    for (INT i = 0; i < q; i++) {
        INT lca_node = lca_results[i];  // ← 使用保存的结果
        allNodes[lca_node]->CPLcount++;
    }



    // ========== 计算 duplicate ==========
    for (Node* u : allNodes) {
        INT sum = u->CPLcount;
        for (auto &kv : u->child) {
            sum += kv.second->duplicate;
        }
        u->duplicate = sum;
    }

    // ========== 计算 css ==========
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

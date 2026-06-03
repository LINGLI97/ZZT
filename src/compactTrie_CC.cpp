#include "compactTrie_CC.h"
#include <fstream>
#include <queue>
#include <stack>

#include "compactTrie.h"
#include "compactTrie.h"

INT zigzag_length(INT i, INT n) {
    // T[1..n]，两端已添加 $
    // 实际字符位置是 1 到 n

    INT left_steps = i + 1;      // 能向左走的步数
    INT right_steps = n- 2 - i;     // 能向右走的步数
    INT steps = min(left_steps, right_steps);

    // 长度 = 初始位置1 + 向右向左各steps步
    // 如果一边还有剩余，再加1

    INT length = 0;
    if (left_steps <= right_steps){
        length = 2* steps + 1;
    }else{
        length = 2* steps +2;

    }

    return length;
}



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

            INT leafDepth = zigzag_length(indices[i], this->text_size+1);
            Node* new_node = new Node(indices[i], leafDepth, new_node_label);
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
            INT leafDepth = zigzag_length(indices[i], this->text_size+1);
            Node* x = new Node(indices[i], leafDepth, x_label);

//            Node * x = new Node( suffix_start, DS.text_size - suffix_start, x_label);
//            x->phi = prefixesStarting[i].second;
            current->addChild(x, x_label);
            current = x;
        }
        // Move current to the new node


    }
}
//
// void compactTrie::buildCC() {
//     std::stack<Node*> st;
//     st.push(root);
//
//     while(!st.empty())
//     {
//         Node* v = st.top();
//
//         if(!v->visited)
//         {
//             v->visited = true;
//
//             for(auto &kv : v->child)
//                 st.push(kv.second);
//         }
//         else
//         {
//             st.pop();
//
//             v->val_even = 0;
//             v->val_odd = 0;
//
//             for(auto &kv : v->child)
//             {
//                 Node* u = kv.second;
//                 INT d = u->depth - v->depth;
//
//                 // 边的贡献
//                 INT edge_even = d / 2;           // floor(d/2)
//                 INT edge_odd = (d - 1) / 2;      // floor((d-1)/2)  ← 修改这里
//
//                 // 计算 val_even
//                 if(d % 2 == 0)
//                     v->val_even += edge_even + u->val_even;
//                 else
//                     v->val_even += edge_even + u->val_odd;
//
//                 // 计算 val_odd
//                 if(d % 2 == 0)
//                     v->val_odd += edge_odd + u->val_odd;
//                 else
//                     v->val_odd += edge_odd + u->val_even;
//             }
//         }
//     }
// }

void compactTrie::buildCC() {
    std::stack<Node*> st;
    st.push(root);

    while(!st.empty())
    {
        Node* v = st.top();

        if(!v->visited)
        {
            // 第一次访问 v，标记并把孩子入栈
            v->visited = true;

            for(auto &kv : v->child)
                st.push(kv.second);
        }
        else
        {
            // 第二次访问 v，后序处理
            st.pop();

            v->val_even = 0;
            v->val_odd = 0;

            for(auto &kv : v->child)
            {
                Node* u = kv.second;
                INT d = u->depth - v->depth;  // 边长

                // 边的贡献
                INT edge_even = d / 2;              // floor(d/2)
                INT edge_odd = (d - 1) / 2;         // ceil(d/2)

                // 计算 val_even
                // d 是偶数：用 val_even(u)
                // d 是奇数：用 val_odd(u)
                if(d % 2 == 0)
                    v->val_even += edge_even + u->val_even;
                else
                    v->val_even += edge_even + u->val_odd + u->child.size();

                // 计算 val_odd
                // d 是偶数：用 val_odd(u)
                // d 是奇数：用 val_even(u)
                if(d % 2 == 0)
                    v->val_odd += edge_odd + u->val_odd + u->child.size();
                else
                    v->val_odd += edge_odd + u->val_even;
            }
        }
    }
}




void compactTrie::visualize(const std::string& filename) {
    std::ofstream dotFile(filename + ".dot");
    if (!dotFile.is_open()) {
        std::cerr << "Error: Cannot open file " << filename << ".dot" << std::endl;
        return;
    }

    dotFile << "digraph CompactTrie {" << std::endl;
    dotFile << "    node [shape=record, fontname=\"Arial\"];" << std::endl;
    dotFile << "    edge [fontname=\"Arial\"];" << std::endl;

    std::queue<Node*> q;
    std::unordered_map<Node*, int> nodeId;
    int idCounter = 0;

    q.push(root);
    nodeId[root] = idCounter++;

    while (!q.empty()) {
        Node* current = q.front();
        q.pop();
        int currentId = nodeId[current];

        // 创建节点标签，包含所需信息
        dotFile << "    node" << currentId << " [label=\"{";
        dotFile << "addr: " << current;
        dotFile << " | depth: " << current->depth;
        dotFile << " | start: " << current->start;
        dotFile << " | val_even: " << current->val_even;
        dotFile << " | val_odd: " << current->val_odd;
        dotFile << "}\"];" << std::endl;

        // 遍历子节点
        for (auto& kv : current->child) {
            Node* child = kv.second;
            unsigned char edgeLabel = kv.first;

            if (nodeId.find(child) == nodeId.end()) {
                nodeId[child] = idCounter++;
                q.push(child);
            }

            int childId = nodeId[child];

            // 添加边，标签显示字符
            dotFile << "    node" << currentId << " -> node" << childId;
            if (edgeLabel == 255) {
                dotFile << " [label=\"$\"];" << std::endl;
            } else if (edgeLabel >= 32 && edgeLabel < 127) {
                dotFile << " [label=\"" << (char)edgeLabel << "\"];" << std::endl;
            } else {
                dotFile << " [label=\"" << (int)edgeLabel << "\"];" << std::endl;
            }
        }
    }

    dotFile << "}" << std::endl;
    dotFile.close();

    // 调用 Graphviz 生成图片
    std::string cmd = "dot -Tpng " + filename + ".dot -o " + filename + ".png";
    int result = system(cmd.c_str());
    if (result == 0) {
        std::cout << "Visualization saved to " << filename << ".png" << std::endl;
    } else {
        std::cerr << "Error: Failed to generate PNG. Make sure Graphviz is installed." << std::endl;
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

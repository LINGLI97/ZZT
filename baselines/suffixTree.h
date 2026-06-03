
#ifndef CPM_SUFFIXTREE_H
#define CPM_SUFFIXTREE_H

#include <string>
#include <map>




#include "stNode.h"
class suffixTree
{




public:

    suffixTree(unsigned char* T,const INT &text_size);
    INT n;
    stNode * root;

    unsigned char* T;

    stNode * createNode(  stNode * u, INT d );
    void createLeaf(INT i, stNode * u, INT d );
    void ComputeSuffixLink( stNode * u );


    stNode * forward_search( unsigned char* P, INT& pattern_size);




    void generateDot(stNode* node, std::ofstream& dotFile, INT tau,
                     std::unordered_set<stNode*>& targets);
    void exportSuffixTreeToDot(const std::string& filename, INT tau);

    //    void getAll_ul(unordered_map<stNode *, stNode *> &result);


    void deleteTreeIteratively();
    ~suffixTree();
};


#endif
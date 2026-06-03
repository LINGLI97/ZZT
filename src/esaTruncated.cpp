
//g++ esa.cpp -std=c++17 -D_USE_64 -I ./libsdsl/include/ -I /usr/include/boost -I ./include/ -L ./libsdsl/lib/ -lsdsl -ldivsufsort -ldivsufsort64  -o esa
#include "esaTruncated.h"
#include "compactTrieTruncated.h"

using namespace std;


// ===== 你的接口：Kasai，原样保留 =====
unsigned int LCParray(unsigned char *text, INT n, std::vector<INT> &SA, INT *ISA, vector<INT> &LCP) {
    INT j = 0;
    LCP[0] = 0;
    for (INT i = 0; i < n; i++) // compute LCP[ISA[i]]
        if (ISA[i] != 0) {
            if (i == 0) j = 0;
            else j = (LCP[ISA[i - 1]] >= 2) ? LCP[ISA[i - 1]] - 1 : 0;
            while (text[i + j] == text[SA[ISA[i] - 1] + j]) j++;
            LCP[ISA[i]] = j;
        }
    return (1);
}





/* Computes the length of lcp of two suffixes of two strings */
INT lcp(unsigned char *x, INT M, vector<unsigned char> y, INT l, INT a_size, INT w_size) {
    INT xx = a_size;
    if (M >= xx) return 0;
    INT yy = w_size;
    if (l >= yy) return 0;

    INT i = 0;
    while ((M + i < xx) && (l + i < yy)) {
        if (x[M + i] != y[l + i]) break;
        i++;
    }
    return i;
}

/* Computes the length of lcp of two suffixes of two strings */
INT lcp ( unsigned char *  x, INT M, unsigned char * y, INT l, INT a_size, INT w_size )
{
    INT xx = a_size;
    if ( M >= xx ) return 0;
    INT yy = w_size;
    if ( l >= yy ) return 0;

    INT i = 0;
    while ( ( M + i < xx ) && ( l + i < yy ) )
    {
        if ( x[M+i] != y[l+i] )	break;
        i++;
    }
    return i;
}









INT zigzag_length(INT i, INT n, INT Bound) {
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

        if (length > Bound + 1){
            length = Bound + 1;
        }

    }else{
        length = 2* steps +2;
        if (length > Bound + 1){
            length = Bound + 1;
        }
    }

    return length;
}


// 特别给zigzag
void construct_tuples(INT n, INT* SA, vector<B>& b, INT *LCP, INT Bound) {
    stack<INT> st;
    INT x = -1;

    // 初始化根节点

    b[0].lcp = 0;
    b[0].l = 0;
    b[0].r = n - 2; //除去 255 哨兵
//    b[0].ch.clear();
    b[0].cursor = 0;
    st.push(0);

    for(INT i = 1; i < n -1; i++) {
        INT l = i - 1;

        while (LCP[i] < LCP[st.top()]) {
            x = st.top();
            b[x].r = i - 1;

            // 出栈时补最后一段尾巴 [cursor..r]
            for(INT pos = b[x].cursor; pos <= b[x].r; pos++) {
                B tmp;
                tmp.l = pos;
                tmp.r = pos;
                tmp.lcp = zigzag_length(SA[pos], n, Bound);
                tmp.leaf_cnt = 1;
                b[x].leaf_cnt += 1;
//                tmp.ch.clear();
                tmp.parent = x;
//                tmp.id = (long long)tmp.l * n + tmp.lcp;  // 立即计算ID
                b.push_back(tmp);

                INT leaf_idx = (INT)b.size() - 1;
                b[leaf_idx].right_node = leaf_idx;
                b[x].right_node = leaf_idx;
//                b[x].ch.push_back(leaf_idx);
            }

            st.pop();
            l = b[x].l;

            INT check_top = st.top();
            if (LCP[i] <= LCP[check_top]) {
                INT parent = check_top;

                // 补 [cursor..x.l-1] 的叶子节点
                for(INT pos = b[parent].cursor; pos < b[x].l; pos++) {
                    B tmp;
                    tmp.l = pos;
                    tmp.r = pos;
                    tmp.lcp = zigzag_length(SA[pos], n, Bound);
                    tmp.leaf_cnt = 1;

                    b[parent].leaf_cnt += 1;
//                    tmp.ch.clear();
                    tmp.parent = parent;
//                    tmp.id = (long long)tmp.l * n + tmp.lcp;
                    b.push_back(tmp);

                    INT leaf_idx = (INT)b.size() - 1;
                    b[leaf_idx].right_node = leaf_idx;
                    b[parent].right_node = leaf_idx;

//                    b[parent].ch.push_back(leaf_idx);
                }

//                b[check_top].ch.push_back(x);
                b[x].parent = check_top;
                b[parent].cursor = b[x].r + 1;
                b[check_top].leaf_cnt += b[x].leaf_cnt;
                b[check_top].right_node = b[x].right_node;

                x = -1;
            }
        }

        if (LCP[i] > LCP[st.top()]) {
            b[i].lcp = LCP[i];
            b[i].l = l;
//            b[i].ch.clear();
            b[i].cursor = l;
            b[i].leaf_cnt = 0;
            st.push(i);

            if(x != -1) {
                // 补空隙 [cursor..x.l-1]
                for(INT pos = b[i].cursor; pos < b[x].l; pos++) {
                    B tmp;
                    tmp.l = pos;
                    tmp.r = pos;
                    tmp.lcp = zigzag_length(SA[pos], n, Bound);
                    tmp.leaf_cnt = 1;
                    b[i].leaf_cnt += 1;
//                    tmp.ch.clear();
                    tmp.parent = i;
//                    tmp.id = (long long)tmp.l * n + tmp.lcp;
                    b.push_back(tmp);


                    INT leaf_idx = (INT)b.size() - 1;          // NEW
                    b[leaf_idx].right_node = leaf_idx;
                    b[i].right_node = leaf_idx;

//                    b[i].ch.push_back(leaf_idx);
                }

//                b[i].ch.push_back(x);
                b[x].parent = i;
                b[i].cursor = b[x].r + 1;
                b[i].leaf_cnt += b[x].leaf_cnt;
                x = -1;
            }
        }
    }


// ===== 关键：处理栈中剩余的所有节点 =====
    while(st.size() > 1) {  // 保留根节点
        x = st.top();
        b[x].r = n - 2;  // 右边界设为最后

        // 补最后一段 [cursor..r]
        for(INT pos = b[x].cursor; pos <= b[x].r; pos++) {
            B tmp;
            tmp.l = pos;
            tmp.r = pos;
            tmp.lcp = zigzag_length(SA[pos], n, Bound);
            tmp.leaf_cnt = 1;
            b[x].leaf_cnt += 1;

//            tmp.ch.clear();
            tmp.parent = x;
//            tmp.id = (long long)tmp.l * n + tmp.lcp;
            b.push_back(tmp);

            INT leaf_idx = (INT)b.size() - 1;          // NEW
            b[leaf_idx].right_node = leaf_idx;         // NEW
            b[x].right_node = leaf_idx;

//            b[x].ch.push_back(leaf_idx);
        }

        st.pop();

        INT parent = st.top();

        // 补父节点的空隙 [cursor..x.l-1]
        for(INT pos = b[parent].cursor; pos < b[x].l; pos++) {
            B tmp;
            tmp.l = pos;
            tmp.r = pos;
            tmp.lcp = zigzag_length(SA[pos], n, Bound);
            tmp.leaf_cnt = 1;
            b[parent].leaf_cnt += 1;

//            tmp.ch.clear();
            tmp.parent = parent;
//            tmp.id = (long long)tmp.l * n + tmp.lcp;
            b.push_back(tmp);

            INT leaf_idx = (INT)b.size() - 1;          // NEW
            b[leaf_idx].right_node = leaf_idx;         // NEW
            b[parent].right_node = leaf_idx;




//            b[parent].ch.push_back(leaf_idx);
        }

        // 将x挂到父节点
//        b[parent].ch.push_back(x);
        b[x].parent = parent;
        b[parent].cursor = b[x].r + 1;
        b[parent].leaf_cnt += b[x].leaf_cnt;


        b[parent].right_node = b[x].right_node;


    }

// 处理根节点的最后部分
    if(!st.empty()) {
        INT root = st.top();
        b[root].r = n - 2; // 排除哨兵

        for(INT pos = b[root].cursor; pos <= b[root].r; pos++) {
            B tmp;
            tmp.l = pos;
            tmp.r = pos;
            tmp.lcp = zigzag_length(SA[pos], n, Bound);
            tmp.leaf_cnt = 1;
            b[root].leaf_cnt += 1;


//            tmp.ch.clear();
            tmp.parent = root;
//            tmp.id = (long long)tmp.l * n + tmp.lcp;
            b.push_back(tmp);

            INT leaf_idx = (INT)b.size() - 1;          // NEW
            b[leaf_idx].right_node = leaf_idx;         // NEW
            b[root].right_node = leaf_idx;
//            b[root].ch.push_back(leaf_idx);
        }
    }    // ===== 最后统一计算所有内部节点的ID（包括根节点） =====

    b[0].id = 0;
    for(INT i = 1; i < b.size(); i++) {
        // 如果 id == -1，说明是内部节点，需要计算ID
//        if(b[i].id == -1) {
        b[i].id = (uint64_t)b[i].l * (uint64_t)n + (uint64_t) b[i].lcp;

//        }
    }

//    cout << "Tuples corresponding to internal nodes and leaves are constructed." << endl;

}

static inline INT extend_lcp_zz(unsigned char* T, INT SA_i,unsigned char* p,INT N, INT m, INT base, INT Bound)
{
    INT j = base;
    for (;; ++j)
    {
        unsigned char cT = getZigZagChar(SA_i, j, T, N, Bound);
        unsigned char cP = p[j]; // 模式从 0 开始

        // 任一侧越界（哨兵 255）或首次不等即停止
        if (cT >= 255 || j > m || cT != cP) break;
    }
    return j; // 返回总匹配长度（= 最后成功匹配位置数）
}




std::pair<INT,INT> pattern_matching_Z(unsigned char *p, unsigned char *T, INT* SA, INT* LCP,
        sdsl::rmq_succinct_sct<> &rmq, INT n, INT p_size, INT B)
{
    INT m = p_size;
    INT N = n;
    INT d = -1, ld = 0;
    INT f = n,  lf = 0;
    std::pair<INT,INT> interval;
// d, i ,f
    while (d + 1 < f)
    {
        INT i = (d + f)/2;

        INT lcpif = (f == n) ? 0 : LCP[rmq(i + 1, f)];
        INT lcpdi = (i == n) ? 0 : LCP[rmq(d + 1, i)];

#ifdef DEBUG
        fprintf(stderr,
            "[Step] d=%d i=%d f=%d | ld=%d lf=%d | lcpdi=%d lcpif=%d\n",
            d, i, f, ld, lf, lcpdi, lcpif);
#endif

        if ((ld <= lcpif) && (lcpif < lf))
        {
#ifdef DEBUG
            fprintf(stderr, " -> case1: move d=i=%d (ld=lcpif=%d)\n", i, lcpif);
#endif
            d = i; ld = lcpif;
        }
        else if ((ld <= lf) && (lf < lcpif))
        {
#ifdef DEBUG
            fprintf(stderr, " -> case2: move f=i=%d\n", i);
#endif
            f = i;
        }
        else if ((lf <= lcpdi) && (lcpdi < ld))
        {
#ifdef DEBUG
            fprintf(stderr, " -> case3: move f=i=%d (lf=lcpdi=%d)\n", i, lcpdi);
#endif
            f = i; lf = lcpdi;
        }
        else if ((lf <= ld) && (ld < lcpdi))
        {
#ifdef DEBUG
            fprintf(stderr, " -> case4: move d=i=%d\n", i);
#endif
            d = i;
        }
        else
        {
            INT l = std::max(ld, lf);
#ifdef DEBUG
            fprintf(stderr, " -> else: explicit compare start at l=%d\n", l);
#endif

            l = extend_lcp_zz(T, SA[i], p, N, m, l, B);
#ifdef DEBUG
            fprintf(stderr, "    extended l=%d (m=%d)\n", l, m);
#endif

            if (l == m)
            {
#ifdef DEBUG
                fprintf(stderr, "    pattern matched fully at i=%d, start expand\n", i);
#endif
                INT e = i;
                while (d + 1 < e)
                {
                    INT j = (d + e)/2;
                    INT lcpje = (e == n) ? 0 : LCP[rmq(j + 1, e)];
#ifdef DEBUG
                    fprintf(stderr, "      expand-left: j=%d e=%d lcpje=%d\n", j, e, lcpje);
#endif
                    if (lcpje < m) d = j; else e = j;
                }

                INT lcpde = (e == n) ? 0 : LCP[rmq(d + 1, e)];
                if (lcpde >= m) d = std::max(d-1, (INT)-1);

                e = i;
                while (e + 1 < f)
                {
                    INT j = (e + f)/2;
                    INT lcpej = (j == n) ? 0 : LCP[rmq(e + 1, j)];
#ifdef DEBUG
                    fprintf(stderr, "      expand-right: e=%d j=%d lcpej=%d\n", e, j, lcpej);
#endif
                    if (lcpej < m) f = j; else e = j;
                }

                INT lcpef = (f == n) ? 0 : LCP[rmq(e + 1, f)];
                if (lcpef >= m) f = std::min(f + 1, n);

                interval.first = d + 1;
                interval.second = f - 1;
#ifdef DEBUG
                fprintf(stderr, " => Match interval: [%d, %d]\n",
                        interval.first, interval.second);
#endif
                return interval;
            }
            else
            {
                unsigned char cT = getZigZagChar(SA[i], l, T, N, B);
                unsigned char cP = p[l];

#ifdef DEBUG
                fprintf(stderr, "    mismatch: l=%d cT=%d cP=%d\n", l, cT, cP);
#endif

                if (l != m && cT < cP)
                {
#ifdef DEBUG
                    fprintf(stderr, " -> move left bound: d=i=%d ld=l=%d\n", i, l);
#endif
                    d = i; ld = l;
                }
                else
                {
#ifdef DEBUG
                    fprintf(stderr, " -> move right bound: f=i=%d lf=l=%d\n", i, l);
#endif
                    f = i; lf = l;
                }
            }
        }
    }

    interval.first = d + 1;
    interval.second = f - 1;

#ifdef DEBUG
    fprintf(stderr, "No full match, final interval [%d, %d]\n",
            interval.first, interval.second);
#endif
    return interval;
}



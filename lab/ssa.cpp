#include "bits/stdc++.h"

auto vec_cmp(std::vector<int>& v) {
    return [&](int i, int j){ return v[i] < v[j]; };
}

struct uf_t 
{
    size_t n;
    std::vector<int> anc;
    std::vector<int> val;
    std::vector<int> ans;

    uf_t(size_t _n)
    : n(_n), anc(n), val(n), ans(n)
    {
        for (int i = 0; i < n; i++) anc[i] = val[i] = i;
    }

    int& operator[](int i){ return val[i]; }

    int eval(int i)
    {
        if (anc[i] == i) printf("Eval(%d) = %d\n", i, i);
        if (anc[i] == i) return i;
        ans[i] = std::min(i, eval(anc[i]), vec_cmp(val));
        anc[i] = anc[anc[i]];
        printf("Eval(%d) = %d\n", i, ans[i]);
        return ans[i];
    }

    void link(int i, int j) {
        printf("Link(%d, %d)\n", i, j);
        if (anc[i] == i) anc[i] = j;
    }
};

using graph_t = std::vector<std::vector<int>>;
graph_t transpose(graph_t& G)
{
    graph_t ans(G.size());
    for (int i = 0; i < G.size(); i++)
        for (auto j : G[i])
            ans[j].push_back(i);
    return ans;
}

struct dm_t 
{
    size_t n;
    std::vector<int> S;
    std::vector<int> D;
    std::vector<int> anc;
    graph_t& succ;
    graph_t pred;
    std::vector<int> pre;
    std::vector<int> id;
    std::vector<int>::iterator ps;
    graph_t todo;

    dm_t(graph_t& G, int r)
    : n(G.size()), S(n), D(n), anc(n), succ(G), pred(transpose(succ)),
      pre(n), id(n), ps(begin(pre)), todo(n)
    {
        dfs(r, r);
        uf_t tree(n);
        tree.val = id;

        std::reverse(begin(pre), end(pre));
        for (auto i : pre) {
            if (i == r) break;

            printf("Processing %d\n", i);
            S[i] = i;
            for (auto j : pred[i])
                S[i] = std::min(S[i], tree.eval(j), vec_cmp(id));
            todo[S[i]].push_back(i);
            tree[i] = id[S[i]];

            {
                int a = anc[i];
                for (auto j : todo[a]) {
                    int u = tree.eval(j);
                    D[j] = id[u] < id[a] ? u : a;
                }
                tree.link(i, a);
            }

        }

        std::reverse(begin(pre), end(pre));
        for (auto i : pre)
            if (D[i] != S[i]) D[i] = D[D[i]];
    }

    void dfs(int i, int a)
    {
        *ps++ = i;
        id[i] = distance(begin(pre), ps);
        anc[i] = a;
        for (int j : succ[i])
            if (!id[j]) dfs(j, i);
    }
};

int main() {
    int n, m;
    scanf(" %d%d", &n, &m);

    graph_t G(n);
    for (int k = 0; k < m; k++) {
        int i, j;
        scanf(" %d%d", &i, &j);
        G[i].push_back(j);
    }

    dm_t T(G, 0);
    for (int i = 0; i < n; i++)
        printf("D[%d] = %d\n", i, T.D[i]);
}

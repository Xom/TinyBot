// based on https://algs4.cs.princeton.edu/15uf/UF.java

#include "unionfind.h"

namespace tinybot {

UnionFind::UnionFind(int n) {
  count = n;
  parent = new int[n];
  rank = new int[n];
  for (int i = 0; i < n; ++i) {
    parent[i] = i;
    rank[i] = 0;
  }
}

UnionFind::~UnionFind() {
  delete[] parent;
  delete[] rank;
}

int UnionFind::find(int p) {
  while (p != parent[p]) {
    parent[p] = parent[parent[p]];    // path compression by halving
    p = parent[p];
  }
  return p;
}

int UnionFind::getCount() const {
  return count;
}

void UnionFind::merge(int p, int q) {
  const int rootP = find(p);
  const int rootQ = find(q);
  if (rootP == rootQ) {
    return;
  }

  // make root of smaller rank point to root of larger rank
  if (rank[rootP] < rank[rootQ]) {
    parent[rootP] = rootQ;
  } else if (rank[rootP] > rank[rootQ]) {
    parent[rootQ] = rootP;
  } else {
    parent[rootQ] = rootP;
    ++rank[rootP];
  }
  --count;
}

}

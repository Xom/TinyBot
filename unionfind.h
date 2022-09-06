// based on https://algs4.cs.princeton.edu/15uf/UF.java

#ifndef TINYBOT_UNIONFIND_H
#define TINYBOT_UNIONFIND_H

namespace tinybot {

struct UnionFind {
  //Constructor and destructor ------------------

  explicit UnionFind(int n);
  ~UnionFind();

  //Functions------------------------------------

  int find(int p);
  int getCount() const;
  void merge(int p, int q);

  private:

  int *parent;
  int *rank;
  int count;
};

}

#endif  // TINYBOT_UNIONFIND_H

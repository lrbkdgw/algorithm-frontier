// blossom_reference.cpp — example program under audit.
//
// Edmonds' unweighted blossom augmentation, O(n^2 * m) worst case. This is a
// plausible "correct but classical" baseline: an audit of this binary should
// report growth close to n^3 on sparse families and should never report a
// correctness failure.
//
// Protocol (see ../README.md):
//   input : "n m" then m lines "u v"      (0-based vertices, loops allowed)
//   output: "k" then k lines "u v"
//
// Build: g++ -O2 -std=c++17 -o blossom_reference blossom_reference.cpp

#include <algorithm>
#include <cstdio>
#include <iostream>
#include <queue>
#include <vector>

int n;
std::vector<std::vector<int>> adj;
std::vector<int> mate, parent_, base_, seen;
std::vector<char> blossom;

// Lowest common ancestor of two outer vertices in the alternating forest.
int lca(int a, int b) {
  std::vector<char> on_path(static_cast<size_t>(n), 0);
  for (;;) {
    a = base_[a];
    on_path[a] = 1;
    if (mate[a] == -1) break;
    a = parent_[mate[a]];
  }
  for (;;) {
    b = base_[b];
    if (on_path[b]) return b;
    b = parent_[mate[b]];
  }
}

// Flip the parent links along one branch of a detected blossom.
void mark_path(int v, int b, int child) {
  while (base_[v] != b) {
    int partner = mate[v];
    blossom[base_[v]] = blossom[base_[partner]] = 1;
    parent_[v] = child;
    child = partner;
    v = parent_[partner];
  }
}

int find_path(int root) {
  std::fill(seen.begin(), seen.end(), 0);
  std::fill(parent_.begin(), parent_.end(), -1);
  for (int i = 0; i < n; ++i) base_[i] = i;
  std::vector<int> q;
  q.push_back(root);
  seen[root] = 1;
  for (size_t head = 0; head < q.size(); ++head) {
    int v = q[head];
    for (int to : adj[v]) {
      if (base_[v] == base_[to] || mate[v] == to) continue;
      if (to == root || (mate[to] != -1 && parent_[mate[to]] != -1)) {
        int b = lca(v, to);
        blossom.assign(static_cast<size_t>(n), 0);
        mark_path(v, b, to);
        mark_path(to, b, v);
        for (int w = 0; w < n; ++w)
          if (blossom[base_[w]]) {
            base_[w] = b;
            if (!seen[w]) {
              seen[w] = 1;
              q.push_back(w);
            }
          }
      } else if (parent_[to] == -1) {
        parent_[to] = v;
        if (mate[to] == -1) return to;
        seen[mate[to]] = 1;
        q.push_back(mate[to]);
      }
    }
  }
  return -1;
}

int main() {
  if (std::scanf("%d", &n) != 1) return 0;
  long long m = 0;
  if (std::scanf("%lld", &m) != 1) return 0;
  adj.assign(n, {});
  std::vector<std::pair<int, int>> edges;
  edges.reserve(static_cast<size_t>(m));
  for (long long i = 0; i < m; ++i) {
    int u = 0, v = 0;
    if (std::scanf("%d %d", &u, &v) != 2) break;
    if (u == v) continue;            // loops are never useful
    edges.emplace_back(u, v);
    adj[u].push_back(v);
    adj[v].push_back(u);
  }
  mate.assign(n, -1);
  parent_.assign(n, -1);
  base_.resize(n);
  seen.assign(n, 1);

  // Greedy warm start, then complete with augmenting searches.
  for (int v = 0; v < n; ++v) {
    if (mate[v] != -1) continue;
    for (int u : adj[v]) {
      if (mate[u] == -1) {
        mate[v] = u;
        mate[u] = v;
        break;
      }
    }
  }
  // Exactly one augmenting search per still-free root: after a successful
  // search the root is matched, so searching from it again would be invalid.
  for (int root = 0; root < n; ++root) {
    if (mate[root] != -1) continue;
    int end = find_path(root);
    if (end == -1) continue;
    for (int v = end; v != -1;) {
      int p = parent_[v];
      if (p == -1) break;
      int next = mate[p];
      mate[v] = p;
      mate[p] = v;
      v = next;
    }
  }

  // Emit one input record per selected vertex pair, in input order.
  std::vector<char> used(n, 0);
  std::vector<std::pair<int, int>> out;
  for (const auto &e : edges) {
    if (used[e.first] || used[e.second]) continue;
    if (mate[e.first] != e.second) continue;
    used[e.first] = used[e.second] = 1;
    out.push_back(e);
  }
  std::printf("%zu\n", out.size());
  for (const auto &e : out) std::printf("%d %d\n", e.first, e.second);
  return 0;
}

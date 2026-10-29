// hopcroft_karp.cpp — example program under audit.
//
// Bipartite maximum matching in O(m * sqrt(n)). The graph is 2-coloured
// first; if a conflict is found the program does NOT solve the instance: it
// emits a greedy maximal matching instead and exits 0. That behaviour is a
// realistic "narrow-scope claim shipped behind a broad interface" and the
// audit should catch it as a correctness failure on non-bipartite families
// such as triangle-union, with an explicit augmenting path as the witness.
//
// Protocol: input "n m" then m lines "u v"; output "k" then k lines "u v".

#include <algorithm>
#include <cstdio>
#include <queue>
#include <vector>

int main() {
  int n = 0;
  long long m = 0;
  if (std::scanf("%d", &n) != 1) return 0;
  if (std::scanf("%lld", &m) != 1) return 0;
  std::vector<std::vector<int>> adj(n);
  std::vector<std::pair<int, int>> edges;
  edges.reserve(static_cast<size_t>(m));
  for (long long i = 0; i < m; ++i) {
    int u = 0, v = 0;
    if (std::scanf("%d %d", &u, &v) != 2) break;
    if (u == v) continue;
    edges.emplace_back(u, v);
    adj[u].push_back(v);
    adj[v].push_back(u);
  }

  // ---- 2-colouring ------------------------------------------------------
  std::vector<int> color(n, -1);
  bool bipartite = true;
  for (int s = 0; s < n && bipartite; ++s) {
    if (color[s] != -1) continue;
    color[s] = 0;
    std::queue<int> q;
    q.push(s);
    while (!q.empty() && bipartite) {
      int v = q.front();
      q.pop();
      for (int to : adj[v]) {
        if (color[to] == -1) {
          color[to] = color[v] ^ 1;
          q.push(to);
        } else if (color[to] == color[v]) {
          bipartite = false;
          break;
        }
      }
    }
  }

  std::vector<int> mate(n, -1);
  if (!bipartite) {
    // Out of scope: emit a greedy maximal matching and exit successfully.
    for (int v = 0; v < n; ++v) {
      if (mate[v] != -1) continue;
      for (int to : adj[v]) {
        if (mate[to] == -1) {
          mate[v] = to;
          mate[to] = v;
          break;
        }
      }
    }
  } else {
    // ---- Hopcroft-Karp: BFS layering plus a blocking-flow DFS ----------
    std::vector<int> dist(n, -1), ptr(n, 0);
    auto bfs = [&]() {
      std::queue<int> q;
      bool reachable_free = false;
      for (int v = 0; v < n; ++v) {
        if (color[v] == 0) {
          if (mate[v] == -1) {
            dist[v] = 0;
            q.push(v);
          } else {
            dist[v] = -1;
          }
        }
      }
      while (!q.empty()) {
        int v = q.front();
        q.pop();
        for (int to : adj[v]) {
          int w = mate[to];
          if (w == -1) {
            reachable_free = true;
          } else if (dist[w] == -1) {
            dist[w] = dist[v] + 1;
            q.push(w);
          }
        }
      }
      return reachable_free;
    };

    // Iterative DFS: `stack` holds left vertices, `chosen[k]` is the right
    // vertex that stack[k] points at on the way to stack[k+1].
    std::vector<int> stack, chosen;
    auto augment = [&](int root) {
      stack.assign(1, root);
      chosen.clear();
      while (!stack.empty()) {
        int v = stack.back();
        bool descended = false;
        while (ptr[v] < static_cast<int>(adj[v].size())) {
          int to = adj[v][ptr[v]];
          int w = mate[to];
          if (w == -1) {
            mate[v] = to;
            mate[to] = v;
            for (int k = static_cast<int>(stack.size()) - 2; k >= 0; --k) {
              int a = stack[static_cast<size_t>(k)];
              int b = chosen[static_cast<size_t>(k)];
              mate[a] = b;
              mate[b] = a;
            }
            return true;
          }
          if (dist[w] == dist[v] + 1) {
            chosen.resize(stack.size());
            chosen[stack.size() - 1] = to;
            stack.push_back(w);
            descended = true;
            break;
          }
          ++ptr[v];
        }
        if (!descended) {
          dist[v] = -1;  // dead end for the rest of this phase
          if (stack.size() == 1) break;
          stack.pop_back();
          ++ptr[stack.back()];
        }
      }
      return false;
    };

    while (bfs()) {
      std::fill(ptr.begin(), ptr.end(), 0);
      for (int v = 0; v < n; ++v)
        if (color[v] == 0 && mate[v] == -1) augment(v);
    }
  }

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

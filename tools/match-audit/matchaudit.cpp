// matchaudit — an adversarial harness for claimed maximum-matching complexity.
//
// The harness cannot prove an asymptotic bound. It can do three things, each
// of which is checkable by an independent party:
//
//   1. Verify correctness of every output with a matcher that shares no code
//      with the program under audit (Berge's augmenting-path criterion).
//   2. Reject a growth claim that the measured ladder contradicts: the
//      program is run on instance families built to stress each term of the
//      claimed bound, and the log-log growth is compared with candidate
//      complexity classes.
//   3. Make the claim itself falsifiable: protocol, instance families, sizes,
//      timing method, noise floor and verdict thresholds are all fixed here
//      and recorded in the report.
//
// A PASS means "not refuted on these families and sizes; outputs were
// correct". It is not a proof. A FAIL is a counterexample.
//
// Build: g++ -O2 -std=c++17 -o matchaudit matchaudit.cpp
// See README.md for the I/O protocol and the report contract.

#include <algorithm>
#include <chrono>
#include <cmath>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <numeric>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

namespace ma {

// ---------------------------------------------------------------- utilities

static const char *kVersion = "matchaudit 1.0";

static std::string trim(const std::string &s) {
  size_t a = 0, b = s.size();
  while (a < b && std::isspace(static_cast<unsigned char>(s[a]))) ++a;
  while (b > a && std::isspace(static_cast<unsigned char>(s[b - 1]))) --b;
  return s.substr(a, b - a);
}

static std::vector<std::string> split(const std::string &s, char sep) {
  std::vector<std::string> out;
  std::string cur;
  std::istringstream in(s);
  while (std::getline(in, cur, sep)) out.push_back(cur);
  return out;
}

static std::string fmt(const char *spec, double v) {
  char buf[128];
  std::snprintf(buf, sizeof buf, spec, v);
  return std::string(buf);
}

static double ms_since(const std::chrono::steady_clock::time_point &t0) {
  return std::chrono::duration<double, std::milli>(
             std::chrono::steady_clock::now() - t0)
      .count();
}

// ------------------------------------------------------------------ instance

struct Instance {
  int n = 0;
  std::vector<std::pair<int, int>> edges;  // endpoints, 0-based
  long long m() const { return static_cast<long long>(edges.size()); }
  long long m_distinct() const {
    std::set<std::pair<int, int>> s(edges.begin(), edges.end());
    return static_cast<long long>(s.size());
  }
  bool skipped = false;  // family would exceed the edge cap at this size
};

struct Rng {
  std::mt19937_64 g;
  explicit Rng(uint64_t seed) : g(seed) {}
  uint64_t next() { return g(); }
  int below(int k) { return k <= 0 ? 0 : static_cast<int>(g() % static_cast<uint64_t>(k)); }
  double unit() { return std::uniform_real_distribution<double>(0.0, 1.0)(g); }
};

static uint64_t mix_seed(uint64_t a, uint64_t b) {
  a += 0x9e3779b97f4a7c15ULL * (b + 1);
  a ^= a >> 30;
  a *= 0xbf58476d1ce4e5b9ULL;
  a ^= a >> 27;
  a *= 0x94d049bb133111ebULL;
  a ^= a >> 31;
  return a;
}

// Instance families. Each one stresses a specific term of a claimed bound:
//
//   empty              the O(n + m) input scan and any adjacency-matrix setup
//   path / cycle       long augmenting paths, deep search trees
//   complete           dense m, quadratic memory risk
//   complete-bipartite dense but bipartite (classical m*sqrt(n) baseline)
//   random-bipartite:P random two-sided graph with density P percent
//   triangle-union     disjoint triangles: nu = n/3, blossom contraction load
//   c5-union           disjoint C5: odd cycles without dense structure
//   petersen-union     non-bipartite, small nu, many blossoms
//   grid               sparse geometric, long distances
//   star-clique        adversarial for cover/kernel compression
//   random-gnp:P       generic density sweep
//   random-regular:D   D random permutations: sparse near-expander
//   expander3          union of three random perfect matchings
//   parallel-heavy:K   m = K * m_s: normalization and multigraph handling
//
// A program that materializes an n-by-n matrix fails `empty` immediately; a
// program that re-scans all edges per augmentation shows up on `path`; one
// that mishandles parallel records shows up on `parallel-heavy`.
static Instance generate(const std::string &spec, int n, long long max_edges,
                         uint64_t seed) {
  Instance inst;
  inst.n = n;
  Rng rng(mix_seed(seed, static_cast<uint64_t>(n) * 1000003ULL + spec.size()));
  std::string family = spec;
  double param = 0;
  size_t colon = spec.find(':');
  if (colon != std::string::npos) {
    family = spec.substr(0, colon);
    param = std::atof(spec.substr(colon + 1).c_str());
  }
  auto add = [&](int u, int v) {
    if (u == v) return;
    inst.edges.emplace_back(u, v);
  };
  auto too_many = [&]() {
    if (static_cast<long long>(inst.edges.size()) > max_edges) {
      inst.edges.clear();
      inst.skipped = true;
      return true;
    }
    return false;
  };

  if (family == "empty") {
    // nothing
  } else if (family == "path") {
    for (int i = 0; i + 1 < n; ++i) add(i, i + 1);
  } else if (family == "cycle") {
    for (int i = 0; i < n; ++i) add(i, (i + 1) % n);
  } else if (family == "complete") {
    for (int i = 0; i < n && !too_many(); ++i)
      for (int j = i + 1; j < n; ++j) {
        add(i, j);
        if (too_many()) break;
      }
  } else if (family == "complete-bipartite") {
    int half = n / 2;
    for (int i = 0; i < half && !too_many(); ++i)
      for (int j = half; j < n; ++j) {
        add(i, j);
        if (too_many()) break;
      }
  } else if (family == "random-bipartite") {
    double p = (param > 0 ? param : 4.0) / 100.0;
    int half = n / 2;
    for (int u = 0; u < half && !too_many(); ++u)
      for (int v = half; v < n; ++v) {
        if (rng.unit() < p) add(u, v);
        if (too_many()) break;
      }
  } else if (family == "triangle-union") {
    for (int i = 0; i + 2 < n; i += 3) {
      add(i, i + 1);
      add(i + 1, i + 2);
      add(i, i + 2);
    }
  } else if (family == "c5-union") {
    for (int i = 0; i + 4 < n; i += 5)
      for (int k = 0; k < 5; ++k) add(i + k, i + (k + 1) % 5);
  } else if (family == "petersen-union") {
    // Two 5-cycles plus a perfect matching; nu = n/2, abundant blossoms.
    for (int b = 0; b + 9 < n; b += 10) {
      for (int k = 0; k < 5; ++k) {
        add(b + k, b + (k + 1) % 5);
        add(b + 5 + k, b + 5 + (k + 1) % 5);
        add(b + k, b + 5 + k);
      }
    }
  } else if (family == "grid") {
    int side = static_cast<int>(std::sqrt(static_cast<double>(n)));
    for (int r = 0; r < side; ++r)
      for (int c = 0; c < side; ++c) {
        int v = r * side + c;
        if (c + 1 < side) add(v, v + 1);
        if (r + 1 < side) add(v, v + side);
      }
  } else if (family == "star-clique") {
    int half = std::max(1, n / 2);
    for (int v = 1; v < half; ++v) add(0, v);         // star at 0
    for (int u = half; u < n; ++u)                    // clique on the rest
      for (int v = u + 1; v < n; ++v) {
        add(u, v);
        if (too_many()) break;
      }
  } else if (family == "random-gnp") {
    double p = (param > 0 ? param : 2.0) / 100.0;
    for (int u = 0; u < n && !too_many(); ++u)
      for (int v = u + 1; v < n; ++v) {
        if (rng.unit() < p) add(u, v);
        if (too_many()) break;
      }
  } else if (family == "random-regular" || family == "expander3" ||
             family == "parallel-heavy") {
    int deg = static_cast<int>(family == "random-regular" ? (param > 0 ? param : 3)
                                                          : 3);
    if (family == "parallel-heavy" && param > 0) deg = 3;
    // Union of deg random permutations: sparse, near-regular, expander-like.
    std::vector<int> perm(static_cast<size_t>(std::max(n, 1)));
    std::iota(perm.begin(), perm.end(), 0);
    for (int d = 0; d < deg; ++d) {
      std::shuffle(perm.begin(), perm.end(), rng.g);
      for (int i = 0; i < n; ++i) add(i, perm[static_cast<size_t>(i)]);
    }
    if (family == "parallel-heavy") {
      long long repeats = static_cast<long long>(param > 0 ? param : 4);
      std::vector<std::pair<int, int>> base = inst.edges;
      for (long long r = 1; r < repeats; ++r) {
        for (const auto &e : base) inst.edges.push_back(e);
        if (static_cast<long long>(inst.edges.size()) > max_edges) {
          inst.edges.clear();
          inst.skipped = true;
          break;
        }
      }
    }
  } else {
    std::fprintf(stderr, "unknown family '%s'\n", family.c_str());
    std::exit(2);
  }
  // Adversarial relabeling for the textbook sparse families: the graph is
  // unchanged, but a greedy vertex-order initialization can no longer stumble
  // onto an optimum, so the measured work reflects the algorithm, not luck.
  if (family == "path" || family == "cycle" || family == "grid") {
    std::vector<int> perm(static_cast<size_t>(std::max(n, 1)));
    std::iota(perm.begin(), perm.end(), 0);
    std::shuffle(perm.begin(), perm.end(), rng.g);
    for (auto &e : inst.edges) {
      e.first = perm[static_cast<size_t>(e.first)];
      e.second = perm[static_cast<size_t>(e.second)];
    }
  }
  // Normalize orientation without deduplicating: parallel records are part of
  // the input model and must survive to the program under audit.
  for (auto &e : inst.edges)
    if (e.first > e.second) std::swap(e.first, e.second);
  return inst;
}

// ------------------------------------------------- independent max matching
//
// Edmonds' blossom search, written for this harness only. It is used to
// (a) certify that a returned matching is maximum, via Berge's criterion, and
// (b) exhibit an explicit augmenting path when it is not.
// Worst case O(n^3) for a full solve; a single failed search is O(m).

class Blossom {
 public:
  explicit Blossom(int n)
      : n_(n), adj_(static_cast<size_t>(n)), mate_(static_cast<size_t>(n), -1),
        base_(static_cast<size_t>(n)), parent_(static_cast<size_t>(n), -1),
        outer_(static_cast<size_t>(n), 0), marked_(static_cast<size_t>(n), 0) {}

  void add_edge(int u, int v) {
    if (u == v) return;
    adj_[u].push_back(v);
    adj_[v].push_back(u);
  }

  // Reads a supplied matching (mate[v], or -1).
  void set_matching(const std::vector<int> &mate) { mate_ = mate; }
  const std::vector<int> &matching() const { return mate_; }

  static int cardinality(const std::vector<int> &mate) {
    int k = 0;
    for (int v : mate)
      if (v >= 0) ++k;
    return k / 2;
  }

  // Completes the current matching to a maximum one (classic blossom loop).
  void augment_to_maximum() {
    for (int root = 0; root < n_; ++root) {
      if (mate_[root] != -1) continue;
      int end = search(root);
      if (end != -1) extend(end);
    }
  }

  // Berge test: does an augmenting path exist w.r.t. the current matching?
  // Returns the vertex sequence (outer/inner alternating) or an empty vector.
  std::vector<int> augmenting_path() {
    for (int root = 0; root < n_; ++root) {
      if (mate_[root] != -1) continue;
      int end = search(root);
      if (end != -1) return trace(end);
    }
    return {};
  }

 private:
  int common_base(int a, int b) {
    std::fill(seen_.begin(), seen_.end(), 0);
    for (;;) {
      a = base_[a];
      seen_[a] = 1;
      if (mate_[a] == -1) break;
      a = parent_[mate_[a]];
      if (a == -1) return -1;
    }
    for (;;) {
      b = base_[b];
      if (seen_[b]) return b;
      if (mate_[b] == -1 || parent_[mate_[b]] == -1) return -1;
      b = parent_[mate_[b]];
    }
  }

  void mark_branch(int v, int b, int toward) {
    while (base_[v] != b) {
      int partner = mate_[v];
      if (partner == -1) return;
      marked_[base_[v]] = 1;
      marked_[base_[partner]] = 1;
      parent_[v] = toward;
      toward = partner;
      v = parent_[partner];
      if (v == -1) return;
    }
  }

  int search(int root) {
    if (seen_.size() != static_cast<size_t>(n_)) seen_.assign(n_, 0);
    std::fill(base_.begin(), base_.end(), 0);
    for (int i = 0; i < n_; ++i) base_[i] = i;
    std::fill(parent_.begin(), parent_.end(), -1);
    std::fill(outer_.begin(), outer_.end(), 0);
    std::vector<int> queue;
    queue.reserve(static_cast<size_t>(n_));
    outer_[root] = 1;
    queue.push_back(root);
    size_t head = 0;
    while (head < queue.size()) {
      int v = queue[head++];
      for (int to : adj_[v]) {
        if (base_[v] == base_[to] || mate_[v] == to) continue;
        if (to == root || (mate_[to] != -1 && parent_[mate_[to]] != -1)) {
          int b = common_base(v, to);
          if (b == -1) continue;
          std::fill(marked_.begin(), marked_.end(), 0);
          mark_branch(v, b, to);
          mark_branch(to, b, v);
          for (int w = 0; w < n_; ++w)
            if (marked_[base_[w]]) {
              base_[w] = b;
              if (!outer_[w]) {
                outer_[w] = 1;
                queue.push_back(w);
              }
            }
        } else if (parent_[to] == -1) {
          parent_[to] = v;
          if (mate_[to] == -1) return to;
          outer_[mate_[to]] = 1;
          queue.push_back(mate_[to]);
        }
      }
    }
    return -1;
  }

  std::vector<int> trace(int end) {
    std::vector<int> path;
    int v = end;
    for (int guard = 0; guard <= 2 * n_ + 2 && v != -1; ++guard) {
      path.push_back(v);
      int p = parent_[v];
      if (p == -1) break;
      path.push_back(p);
      v = mate_[p];
    }
    return path;
  }

  void extend(int end) {
    int v = end;
    for (int guard = 0; guard <= 2 * n_ + 2 && v != -1; ++guard) {
      int p = parent_[v];
      if (p == -1) break;
      int next = mate_[p];
      mate_[v] = p;
      mate_[p] = v;
      v = next;
    }
  }

  int n_;
  std::vector<std::vector<int>> adj_;
  std::vector<int> mate_, base_, parent_;
  std::vector<char> outer_, marked_;
  std::vector<char> seen_ = std::vector<char>(1, 0);
};

// -------------------------------------------------------------- child runner

struct RunOutcome {
  bool launched = true;
  bool timed_out = false;
  int exit_code = 0;
  int term_signal = 0;
  double wall_ms = 0;
  double cpu_ms = 0;
  double max_rss_mb = 0;
};

static RunOutcome run_child(const std::string &command, const std::string &in_path,
                            const std::string &out_path, const std::string &err_path,
                            double time_limit_s, long mem_limit_mb) {
  RunOutcome out;
  pid_t pid = fork();
  if (pid < 0) {
    out.launched = false;
    return out;
  }
  if (pid == 0) {
    setpgid(0, 0);
    int fd_in = open(in_path.c_str(), O_RDONLY);
    int fd_out = open(out_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    int fd_err = open(err_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_in < 0 || fd_out < 0 || fd_err < 0) _exit(127);
    if (dup2(fd_in, 0) < 0 || dup2(fd_out, 1) < 0 || dup2(fd_err, 2) < 0) _exit(127);
    close(fd_in);
    close(fd_out);
    close(fd_err);
    struct rlimit rl;
    rl.rlim_cur = rl.rlim_max = static_cast<rlim_t>(std::ceil(time_limit_s)) + 2;
    setrlimit(RLIMIT_CPU, &rl);
    if (mem_limit_mb > 0) {
      rl.rlim_cur = rl.rlim_max = static_cast<rlim_t>(mem_limit_mb) * 1024 * 1024;
      setrlimit(RLIMIT_AS, &rl);
    }
    setenv("PYTHONHASHSEED", "0", 0);
    execl("/bin/sh", "sh", "-c", command.c_str(), static_cast<char *>(nullptr));
    _exit(127);
  }
  setpgid(pid, pid);
  struct rusage ru;
  std::memset(&ru, 0, sizeof ru);
  auto t0 = std::chrono::steady_clock::now();
  int status = 0;
  bool killed = false;
  for (;;) {
    pid_t r = wait4(pid, &status, WNOHANG, &ru);
    if (r == pid) break;
    if (r < 0) break;
    double elapsed = ms_since(t0);
    if (!killed && elapsed > time_limit_s * 1000.0) {
      kill(-pid, SIGKILL);
      kill(pid, SIGKILL);
      killed = true;
      out.timed_out = true;
    }
  }
  out.wall_ms = ms_since(t0);
  out.cpu_ms = 1000.0 * (static_cast<double>(ru.ru_utime.tv_sec) +
                         static_cast<double>(ru.ru_utime.tv_usec) / 1e6 +
                         static_cast<double>(ru.ru_stime.tv_sec) +
                         static_cast<double>(ru.ru_stime.tv_usec) / 1e6);
  out.max_rss_mb = static_cast<double>(ru.ru_maxrss) / 1024.0;
  if (WIFEXITED(status)) out.exit_code = WEXITSTATUS(status);
  if (WIFSIGNALED(status)) out.term_signal = WTERMSIG(status);
  return out;
}

static bool write_instance(const Instance &inst, const std::string &path,
                           int id_kind, uint64_t seed) {
  std::ofstream f(path, std::ios::binary);
  if (!f) return false;
  std::string buf;
  buf.reserve(static_cast<size_t>(inst.edges.size()) * 24 + 64);
  buf += std::to_string(inst.n) + " " + std::to_string(inst.m()) + "\n";
  Rng rng(mix_seed(seed, 97));
  for (size_t i = 0; i < inst.edges.size(); ++i) {
    buf += std::to_string(inst.edges[i].first) + " " +
           std::to_string(inst.edges[i].second);
    if (id_kind == 1) {  // sparse 64-bit identifiers
      unsigned long long id = (1ULL << 40) + 7919ULL * (i + 1) + rng.next() % 7;
      buf += " " + std::to_string(id);
    }
    buf += "\n";
  }
  f << buf;
  return true;
}

// ------------------------------------------------------------ output parsing

struct ParsedOutput {
  bool ok = false;
  std::string why;
  std::vector<int> edge_index;  // one per selected edge, in input order space
};

static ParsedOutput parse_output(const std::string &path, const Instance &inst,
                                 int protocol) {
  ParsedOutput out;
  std::ifstream f(path);
  if (!f) {
    out.why = "no output file";
    return out;
  }
  std::string token;
  if (!(f >> token)) {
    out.why = "empty output (expected a cardinality on the first line)";
    return out;
  }
  long long k = -1;
  try {
    k = std::stoll(token);
  } catch (...) {
    out.why = "first token '" + token + "' is not an integer cardinality";
    return out;
  }
  if (k < 0) {
    out.why = "negative cardinality";
    return out;
  }
  long long cap = inst.m() + 1;
  if (k > cap) {
    out.why = "cardinality " + std::to_string(k) + " exceeds the edge count";
    return out;
  }
  std::vector<char> used_edge(static_cast<size_t>(inst.m()), 0);
  for (long long i = 0; i < k; ++i) {
    long long a = 0, b = 0;
    if (!(f >> a)) {
      out.why = "output ended after " + std::to_string(i) + " of " +
                std::to_string(k) + " matching edges";
      return out;
    }
    if (protocol == 1) {
      if (a < 0 || a >= inst.m()) {
        out.why = "edge index " + std::to_string(a) + " out of range";
        return out;
      }
      if (used_edge[static_cast<size_t>(a)]) {
        out.why = "edge index " + std::to_string(a) + " repeated";
        return out;
      }
      used_edge[static_cast<size_t>(a)] = 1;
      out.edge_index.push_back(static_cast<int>(a));
      continue;
    }
    if (!(f >> b)) {
      out.why = "truncated edge line at entry " + std::to_string(i + 1);
      return out;
    }
    if (a < 0 || a >= inst.n || b < 0 || b >= inst.n) {
      out.why = "endpoint out of range at entry " + std::to_string(i + 1);
      return out;
    }
    int lo = static_cast<int>(std::min(a, b)), hi = static_cast<int>(std::max(a, b));
    int found = -1;
    for (size_t e = 0; e < inst.edges.size(); ++e) {
      if (inst.edges[e].first == lo && inst.edges[e].second == hi) {
        if (!used_edge[e]) {
          found = static_cast<int>(e);
          used_edge[e] = 1;
          break;
        }
        found = -2;  // present but already consumed
        break;
      }
    }
    if (found == -1) {
      out.why = "edge (" + std::to_string(lo) + "," + std::to_string(hi) +
                ") is not an input record";
      return out;
    }
    if (found == -2) {
      out.why = "edge (" + std::to_string(lo) + "," + std::to_string(hi) +
                ") used more often than it appears in the input";
      return out;
    }
    out.edge_index.push_back(found);
  }
  out.ok = true;
  return out;
}

// --------------------------------------------------------------- verification

struct VerifyOutcome {
  bool parsed = false;
  bool feasible = false;
  bool checked = false;      // optimality was actually decided
  bool optimal = false;      // no augmenting path found
  bool budget_skipped = false;
  int k = -1;
  int nu = -1;
  std::string detail;
  std::vector<int> witness;
};

static VerifyOutcome verify(const Instance &inst, const ParsedOutput &parsed,
                            int verify_max_n, long long verify_max_m,
                            double budget, bool want_nu) {
  VerifyOutcome vo;
  if (!parsed.ok) {
    vo.detail = parsed.why;
    return vo;
  }
  vo.parsed = true;
  vo.k = static_cast<int>(parsed.edge_index.size());

  Blossom b(inst.n);
  for (const auto &e : inst.edges) b.add_edge(e.first, e.second);

  std::vector<int> mate(static_cast<size_t>(inst.n), -1);
  std::vector<char> seen(static_cast<size_t>(inst.n), 0);
  for (int idx : parsed.edge_index) {
    int u = inst.edges[static_cast<size_t>(idx)].first;
    int v = inst.edges[static_cast<size_t>(idx)].second;
    if (seen[static_cast<size_t>(u)] || seen[static_cast<size_t>(v)]) {
      vo.detail = "matched edges share a vertex (matching is not feasible)";
      return vo;
    }
    seen[static_cast<size_t>(u)] = seen[static_cast<size_t>(v)] = 1;
    mate[static_cast<size_t>(u)] = v;
    mate[static_cast<size_t>(v)] = u;
  }
  vo.feasible = true;

  // No edges: the only maximum matching is the empty one, decided in O(m).
  if (inst.m() == 0) {
    vo.checked = true;
    vo.optimal = (vo.k == 0);
    vo.nu = 0;
    if (!vo.optimal) vo.detail = "the graph has no edges, so the optimum is 0";
    return vo;
  }

  double free_roots = static_cast<double>(inst.n) - 2.0 * static_cast<double>(vo.k);
  double work = std::max(1.0, free_roots) *
                (static_cast<double>(inst.n) + static_cast<double>(inst.m()));
  if (inst.n > verify_max_n || inst.m() > verify_max_m || work > budget) {
    vo.budget_skipped = true;
    vo.detail = "instance exceeds the verification budget; feasibility only";
    return vo;
  }

  b.set_matching(mate);
  std::vector<int> path = b.augmenting_path();
  vo.checked = true;
  if (path.empty()) {
    vo.optimal = true;
  } else {
    vo.optimal = false;
    vo.witness = path;
    vo.detail = "an augmenting path exists, so the returned matching is not maximum";
  }
  if (want_nu) {
    Blossom full(inst.n);
    for (const auto &e : inst.edges) full.add_edge(e.first, e.second);
    full.augment_to_maximum();
    vo.nu = Blossom::cardinality(full.matching());
  }
  return vo;
}

// ------------------------------------------------------------------ statistics

struct Point {
  int n = 0;
  long long m = 0, ms = 0;
  bool ran = false, skipped = false, timed_out = false;
  double wall_best = 0, wall_med = 0, spread = 0;  // ms
  double t = 0;                                    // baseline-corrected
  double rss_mb = 0;
  bool noise_dominated = false;
  int k = -1;
  bool verified = false, correctness_failed = false;
  std::string note;
};

enum ShapeId { S_CLAIM = 0, S_POLYLOG, S_MSQRTN, S_POLYM, S_CUBIC, S_NM, S_COUNT };
static const char *kShapeNames[S_COUNT] = {
    "claim (m_s * exp(C*phi))", "m * log^3 m", "m * sqrt(n)",
    "m^a (free exponent)",      "n^3",         "n * m"};

// phi(t) = (log2(t+2))^(4/5) * (log2 log2(t+4))^(1/5), the exponent shape of
// the repository's claimed deterministic matching bound.
static double phi_claim(double ms) {
  double a = std::log2(ms + 2.0);
  double inner = std::log2(std::log2(ms + 4.0));
  if (!(inner > 0)) inner = 1e-6;
  return std::pow(a, 0.8) * std::pow(inner, 0.2);
}

// Total predicted cost, up to a single multiplicative constant K.
static double predicted(int shape, double n, double m, double ms, double param) {
  double base = n + m;
  switch (shape) {
    case S_CLAIM: {
      double c = param;
      return base + (ms + 1.0) * std::exp(c * phi_claim(ms));
    }
    case S_POLYLOG:
      return base + (ms + 1.0) * std::pow(std::max(1.0, std::log2(ms + 2.0)), 3.0);
    case S_MSQRTN:
      return base + (ms + 1.0) * std::sqrt(n + 1.0);
    case S_POLYM:
      return base + std::pow(ms + 1.0, param);
    case S_CUBIC:
      return base + std::pow(n + 1.0, 3.0);
    case S_NM:
      return base + (n + 1.0) * (ms + 1.0);
    default:
      return base;
  }
}

struct Fit {
  int shape = -1;
  double param = 0;
  double k = 0;      // multiplicative constant
  double rmse = 1e30;  // in natural-log units
  int points = 0;
  bool valid = false;
};

static Fit fit_shape(int shape, const std::vector<Point> &pts) {
  Fit best;
  best.shape = shape;
  auto evaluate = [&](double param) {
    double sum = 0;
    int cnt = 0;
    for (const auto &p : pts) {
      if (!p.ran || p.noise_dominated || p.skipped || p.t <= 0) continue;
      double f = predicted(shape, p.n, p.m, static_cast<double>(p.ms), param);
      if (!(f > 0)) continue;
      sum += std::log(p.t) - std::log(f);
      ++cnt;
    }
    if (cnt < 2) return Fit{shape, param, 0, 1e30, cnt, false};
    double lk = sum / cnt;
    double sse = 0;
    for (const auto &p : pts) {
      if (!p.ran || p.noise_dominated || p.skipped || p.t <= 0) continue;
      double f = predicted(shape, p.n, p.m, static_cast<double>(p.ms), param);
      if (!(f > 0)) continue;
      double r = std::log(p.t) - std::log(f) - lk;
      sse += r * r;
    }
    Fit f;
    f.shape = shape;
    f.param = param;
    f.k = std::exp(lk);
    f.rmse = std::sqrt(sse / cnt);
    f.points = cnt;
    f.valid = true;
    return f;
  };

  bool free_param = (shape == S_CLAIM || shape == S_POLYM);
  if (!free_param) {
    best = evaluate(0.0);
    return best;
  }
  double lo = (shape == S_CLAIM) ? 0.0 : 1.0;
  double hi = (shape == S_CLAIM) ? 40.0 : 3.2;
  for (int i = 0; i <= 400; ++i) {
    double p = lo + (hi - lo) * i / 400.0;
    Fit f = evaluate(p);
    if (f.valid && f.rmse < best.rmse) best = f;
  }
  if (!best.valid) return best;
  // Golden-section refinement around the grid minimum.
  double a = std::max(lo, best.param - (hi - lo) / 400.0);
  double c = std::min(hi, best.param + (hi - lo) / 400.0);
  for (int it = 0; it < 60; ++it) {
    double x1 = c - (c - a) * 0.6180339887;
    double x2 = a + (c - a) * 0.6180339887;
    Fit f1 = evaluate(x1), f2 = evaluate(x2);
    if (f1.valid && f1.rmse < best.rmse) best = f1;
    if (f2.valid && f2.rmse < best.rmse) best = f2;
    if (!f1.valid || !f2.valid) break;
    if (f1.rmse < f2.rmse) c = x2;
    else a = x1;
  }
  return best;
}

// Least-squares slope of log(t) against log(x) across usable points.
static bool loglog_slope(const std::vector<Point> &pts, bool use_n,
                         double *slope, double *se) {
  double sx = 0, sy = 0, sxx = 0, sxy = 0;
  int cnt = 0;
  std::set<double> distinct;
  for (const auto &p : pts) {
    if (!p.ran || p.noise_dominated || p.skipped || p.t <= 0) continue;
    double raw = static_cast<double>(use_n ? p.n : p.m);
    if (raw < 1.0) continue;  // a zero coordinate carries no rate information
    double x = std::log(raw);
    distinct.insert(raw);
    double y = std::log(p.t);
    sx += x;
    sy += y;
    sxx += x * x;
    sxy += x * y;
    ++cnt;
  }
  if (cnt < 3 || distinct.size() < 3) return false;
  double denom = cnt * sxx - sx * sx;
  if (std::fabs(denom) < 1e-12) return false;
  double b = (cnt * sxy - sx * sy) / denom;
  double a = (sy - b * sx) / cnt;
  double sse = 0;
  for (const auto &p : pts) {
    if (!p.ran || p.noise_dominated || p.skipped || p.t <= 0) continue;
    double raw = static_cast<double>(use_n ? p.n : p.m);
    if (raw < 1.0) continue;
    double x = std::log(raw);
    double r = std::log(p.t) - (a + b * x);
    sse += r * r;
  }
  *slope = b;
  *se = std::sqrt(sse / std::max(1, cnt - 2) * cnt / denom);
  return true;
}

// ------------------------------------------------------------- configuration

struct Config {
  std::string solver;
  std::vector<std::string> families;
  std::vector<int> sizes;
  int repeats = 5;
  double time_limit_s = 20.0;
  long mem_limit_mb = 0;
  long long max_edges = 400000;
  int protocol = 0;        // 0 = vertex endpoints, 1 = edge indices
  int id_kind = 0;         // 0 = plain, 1 = sparse 64-bit identifiers
  int verify_max_n = 20000;
  long long verify_max_m = 400000;
  double verify_work_budget = 1.0e9;  // free roots x (n + m) before giving up
  bool verify = true;
  bool claim_is_near_linear = true;
  std::string ops_prefix;
  double noise_floor_ms = 0.2;
  uint64_t seed = 20261029ULL;
  std::string workdir;
  std::string json_path, md_path;
  double baseline_ms = 0;
  double baseline_spread_ms = 0;
  bool keep_temp = false;
  bool quiet = false;
  int crosscheck = 0;
  bool calibrate = true;
  int max_n = 100000;
};

struct FamilyResult {
  std::string family;
  std::vector<int> sizes_used;
  std::vector<Point> points;
  Fit fits[S_COUNT];
  Fit best;
  double exponent_m = 0, exponent_m_se = 0;
  double exponent_n = 0, exponent_n_se = 0;
  double rss_exponent = 0;
  double span_m = 0, span_n = 0;
  bool has_exponent_m = false, has_exponent_n = false, has_rss = false;
  bool correctness_failure = false;
  bool claim_refuted = false;
  bool claim_consistent = false;
  std::string verdict;
  std::string evidence;
};

// ------------------------------------------------------------------- reporting

static std::string now_iso() {
  std::time_t t = std::time(nullptr);
  char buf[64];
  std::tm tmv;
  gmtime_r(&t, &tmv);
  std::strftime(buf, sizeof buf, "%Y-%m-%dT%H:%M:%SZ", &tmv);
  return std::string(buf);
}

static void human_point_row(const Point &p) {
  if (p.skipped) {
    std::printf("  %8d %10s %10s   (skipped: exceeds edge cap)\n", p.n, "-", "-");
    return;
  }
  if (!p.ran) {
    std::printf("  %8d %10lld %10lld   %s\n", p.n, p.m, p.ms, p.note.c_str());
    return;
  }
  double per_edge_ns = p.t > 0 && p.m > 0 ? p.t * 1e6 / static_cast<double>(p.m) : 0;
  std::printf("  %8d %10lld %10lld %10.1f %12.1f %9.1f %7d  %s%s\n", p.n, p.m,
              p.ms, p.t, per_edge_ns, p.rss_mb, p.k,
              (p.noise_dominated ? "[noise] " : (p.spread > 0.25 ? "[noisy] " : "")),
              (p.correctness_failed ? "[WRONG] " : (p.verified ? "ok" : "unverified")));
}

}  // namespace ma

int main(int argc, char **argv) {
  using namespace ma;
  Config cfg;
  bool self_test = false;

  for (int i = 1; i < argc; ++i) {
    std::string a = argv[i];
    auto need = [&](const char *) -> std::string {
      if (i + 1 >= argc) {
        std::fprintf(stderr, "missing value for %s\n", a.c_str());
        std::exit(2);
      }
      return argv[++i];
    };
    if (a == "--solver") cfg.solver = need("solver");
    else if (a == "--families") {
      cfg.families.clear();
      for (auto &s : split(need("families"), ',')) cfg.families.push_back(trim(s));
    } else if (a == "--sizes") {
      cfg.sizes.clear();
      for (auto &s : split(need("sizes"), ','))
        if (!trim(s).empty()) cfg.sizes.push_back(std::atoi(trim(s).c_str()));
    } else if (a == "--repeats") cfg.repeats = std::max(1, std::atoi(need("repeats").c_str()));
    else if (a == "--time-limit-s") cfg.time_limit_s = std::atof(need("t").c_str());
    else if (a == "--mem-limit-mb") cfg.mem_limit_mb = std::atol(need("m").c_str());
    else if (a == "--max-edges") cfg.max_edges = std::atoll(need("e").c_str());
    else if (a == "--protocol") {
      std::string v = need("p");
      if (v == "ids") cfg.protocol = 1;
      else if (v == "endpoints") cfg.protocol = 0;
      else { std::fprintf(stderr, "--protocol expects endpoints|ids\n"); return 2; }
    } else if (a == "--id-kind") {
      std::string v = need("k");
      if (v == "sparse64") cfg.id_kind = 1;
      else if (v == "index") cfg.id_kind = 0;
      else { std::fprintf(stderr, "--id-kind expects index|sparse64\n"); return 2; }
    } else if (a == "--verify") {
      std::string v = need("v");
      cfg.verify = (v != "off" && v != "none");
      if (v == "off" || v == "none") cfg.verify = false;
    } else if (a == "--verify-max-n") cfg.verify_max_n = std::atoi(need("n").c_str());
    else if (a == "--verify-max-m") cfg.verify_max_m = std::atoll(need("m").c_str());
    else if (a == "--claim") {
      std::string v = need("c");
      if (v == "near-linear" || v == "nearlinear") cfg.claim_is_near_linear = true;
      else if (v == "none") cfg.claim_is_near_linear = false;
      else { std::fprintf(stderr, "--claim expects near-linear|none\n"); return 2; }
    } else if (a == "--ops-prefix") cfg.ops_prefix = need("p");
    else if (a == "--seed") cfg.seed = std::strtoull(need("s").c_str(), nullptr, 10);
    else if (a == "--json") cfg.json_path = need("j");
    else if (a == "--md") cfg.md_path = need("m");
    else if (a == "--workdir") cfg.workdir = need("w");
    else if (a == "--keep-temp") cfg.keep_temp = true;
    else if (a == "--quiet") cfg.quiet = true;
    else if (a == "--crosscheck") cfg.crosscheck = std::atoi(need("k").c_str());
    else if (a == "--no-calibrate") cfg.calibrate = false;
    else if (a == "--max-n") cfg.max_n = std::atoi(need("n").c_str());
    else if (a == "--self-test") self_test = true;
    else if (a == "--version") { std::printf("%s\n", kVersion); return 0; }
    else if (a == "--help" || a == "-h") {
      std::printf(
          "%s\n\n"
          "audit a claimed maximum-matching complexity\n\n"
          "  --solver CMD        program under audit; reads the instance on stdin\n"
          "                      and writes its matching on stdout (see README)\n"
          "  --families LIST     comma-separated instance families\n"
          "  --sizes LIST        comma-separated starting vertex counts\n"
          "                      (default 500,1000,2000,4000; rescaled unless\n"
          "                       --no-calibrate is given)\n"
          "  --repeats K         measured runs per point, best wall time kept (5)\n"
          "  --time-limit-s S    per-run wall and CPU limit (default 20)\n"
          "  --protocol P        endpoints (default) or ids\n"
          "  --id-kind K         index (default) or sparse64\n"
          "  --verify MODE       auto (default) or off\n"
          "  --verify-max-n N    skip optimality checks above this size (default 20000)\n"
          "  --claim C           near-linear (default) or none\n"
          "  --families LIST     empty path cycle complete complete-bipartite\n"
          "                      random-bipartite:P triangle-union c5-union\n"
          "                      petersen-union grid star-clique random-gnp:P\n"
          "                      random-regular:D expander3 parallel-heavy:K\n"
          "  --ops-prefix STR    scrape 'STR<integer>' from stderr as an op counter\n"
          "  --json PATH         write machine-readable results\n"
          "  --md PATH           write a Markdown audit report\n"
          "  --crosscheck K      run K random small instances and verify only\n"
          "  --no-calibrate      do not rescale the ladder to beat process noise\n"
          "  --max-n N           largest vertex count the calibration may reach\n"
          "  --self-test         check this harness against brute force\n"
          "\nA PASS is evidence, not a proof. A FAIL is a counterexample.\n",
          kVersion);
      return 0;
    } else {
      std::fprintf(stderr, "unknown option %s (try --help)\n", a.c_str());
      return 2;
    }
  }

  // ------------------------------------------------------------ self test
  if (self_test) {
    std::printf("self-test: blossom vs brute force, augmenting-path extraction\n");
    Rng rng(12345);
    long long checked = 0, mismatches = 0, path_failures = 0;
    for (int n = 1; n <= 9; ++n) {
      for (int trial = 0; trial < 400; ++trial) {
        Blossom b(n);
        std::vector<std::pair<int, int>> es;
        for (int u = 0; u < n; ++u)
          for (int v = u + 1; v < n; ++v)
            if (rng.unit() < 0.4) {
              es.emplace_back(u, v);
              b.add_edge(u, v);
            }
        // brute force: enumerate all matchings via DP over vertex subsets
        std::vector<int> best(static_cast<size_t>(1) << n, -1);
        best[0] = 0;
        int brute = 0;
        for (int mask = 0; mask < (1 << n); ++mask) {
          if (best[static_cast<size_t>(mask)] < 0) continue;
          brute = std::max(brute, best[static_cast<size_t>(mask)]);
          int u = 0;
          while (u < n && (mask >> u & 1)) ++u;
          if (u == n) continue;
          // Either leave u unmatched, or match it with a free neighbour.
          {
            int skip = mask | (1 << u);
            best[static_cast<size_t>(skip)] =
                std::max(best[static_cast<size_t>(skip)],
                         best[static_cast<size_t>(mask)]);
          }
          for (int v = u + 1; v < n; ++v) {
            if (mask >> v & 1) continue;
            bool edge = false;
            for (auto &e : es)
              if ((e.first == u && e.second == v) || (e.first == v && e.second == u)) edge = true;
            if (!edge) continue;
            int nm = mask | (1 << u) | (1 << v);
            best[static_cast<size_t>(nm)] =
                std::max(best[static_cast<size_t>(nm)],
                         best[static_cast<size_t>(mask)] + 1);
          }
        }
        b.augment_to_maximum();
        int got = Blossom::cardinality(b.matching());
        ++checked;
        if (got != brute) {
          ++mismatches;
          if (mismatches <= 5)
            std::printf("  MISMATCH n=%d brute=%d blossom=%d\n", n, brute, got);
        }
        // A deliberately sub-maximal matching must produce a valid
        // augmenting path: removing one matched pair must be detected.
        if (got > 0) {
          std::vector<int> mate = b.matching();
          int victim = -1;
          for (int v = 0; v < n; ++v)
            if (mate[static_cast<size_t>(v)] >= 0) { victim = v; break; }
          if (victim >= 0) {
            int partner = mate[static_cast<size_t>(victim)];
            mate[static_cast<size_t>(victim)] = -1;
            mate[static_cast<size_t>(partner)] = -1;
            Blossom c(n);
            for (auto &e : es) c.add_edge(e.first, e.second);
            c.set_matching(mate);
            std::vector<int> path = c.augmenting_path();
            if (path.size() < 2 || path.size() % 2 != 0) {
              ++path_failures;
              std::printf("  BAD AUGMENTING PATH n=%d len=%zu\n", n, path.size());
            } else {
              // endpoints must be free and alternate in the reduced matching
              for (size_t idx = 0; idx < path.size(); idx += 2) {
                bool uf = mate[static_cast<size_t>(path[idx])] < 0;
                if (idx + 1 >= path.size()) { if (!uf) ++path_failures; }
              }
            }
          }
        }
      }
    }
    std::printf("  graphs checked: %lld, cardinality mismatches: %lld, "
                "augmenting-path failures: %lld\n", checked, mismatches, path_failures);
    // fitting sanity: an exactly linear synthetic series must return slope 1
    std::vector<Point> pts;
    for (int n : {100, 200, 400, 800}) {
      Point p;
      p.n = n;
      p.m = 3LL * n;
      p.ms = 3LL * n;
      p.ran = true;
      // exactly the modelled shape with exponent 1: K*((n+m) + m^1)
      p.t = 1e-3 * (static_cast<double>(p.n) + static_cast<double>(p.m) +
                    static_cast<double>(p.m + 1));
      pts.push_back(p);
    }
    double s = 0, se = 0;
    bool ok = loglog_slope(pts, false, &s, &se);
    std::printf("  fitted slope on an exactly linear series: %.4f (expected 1)\n", s);
    Fit linear = fit_shape(S_POLYM, pts);
    std::printf("  free-exponent fit: a=%.4f rmse=%.3g\n", linear.param, linear.rmse);
    bool pass = (mismatches == 0 && path_failures == 0 && ok && std::fabs(s - 1) < 1e-2 &&
                 std::fabs(linear.param - 1.0) < 5e-3);
    std::printf("%s\n", pass ? "self-test PASS" : "self-test FAIL");
    return pass ? 0 : 1;
  }

  if (cfg.solver.empty()) {
    std::fprintf(stderr, "--solver is required (or use --self-test)\n");
    return 2;
  }
  if (cfg.families.empty()) {
    cfg.families = {"empty",     "path",              "triangle-union",
                    "random-gnp:2", "random-regular:3", "parallel-heavy:4"};
  }
  if (cfg.sizes.empty()) cfg.sizes = {500, 1000, 2000, 4000};

  std::string workdir = cfg.workdir;
  if (workdir.empty()) {
    char tmpl[] = "/tmp/matchauditXXXXXX";
    char *d = mkdtemp(tmpl);
    if (!d) {
      std::fprintf(stderr, "cannot create a temporary directory\n");
      return 2;
    }
    workdir = d;
  }
  const std::string in_path = workdir + "/instance.txt";
  const std::string out_path = workdir + "/answer.txt";
  const std::string err_path = workdir + "/stderr.txt";

  auto command_for = [&](const std::string &cmd) {
    std::string c = cmd;
    size_t pos;
    while ((pos = c.find("{input}")) != std::string::npos)
      c.replace(pos, 7, in_path);
    while ((pos = c.find("{output}")) != std::string::npos)
      c.replace(pos, 8, out_path);
    return c;
  };
  const bool solver_writes_file = cfg.solver.find("{output}") != std::string::npos;
  // If the program writes {output} itself, its stdout is a separate log.
  const std::string stdout_target =
      solver_writes_file ? (workdir + "/stdout.log") : out_path;

  // Startup baseline: the same process on a one-vertex, zero-edge instance.
  auto measure_baseline = [&]() {
    Instance tiny;
    tiny.n = 1;
    write_instance(tiny, in_path, cfg.id_kind, cfg.seed);
    std::vector<double> walls;
    for (int r = 0; r < std::max(3, cfg.repeats); ++r) {
      RunOutcome o = run_child(command_for(cfg.solver), in_path, stdout_target, err_path,
                               cfg.time_limit_s, cfg.mem_limit_mb);
      if (o.timed_out || o.term_signal != 0) continue;
      walls.push_back(o.wall_ms);
    }
    if (walls.empty()) return;
    std::sort(walls.begin(), walls.end());
    cfg.baseline_ms = walls.front();
    cfg.baseline_spread_ms = walls.back() - walls.front();
  };
  measure_baseline();
  if (!cfg.quiet)
    std::printf("%s\nsolver: %s\nstartup baseline: %.2f ms (+- %.2f ms); "
                "measurement resolution is the larger of that spread and the "
                "per-instance spread\n\n",
                kVersion, cfg.solver.c_str(), cfg.baseline_ms, cfg.baseline_spread_ms);

  int total_runs = 0, total_verified = 0, total_instances = 0;

  // Runs the program once on an instance and returns the best wall time, or a
  // negative value when the run did not complete.
  auto timed_run = [&](const Instance &inst, int repeats) {
    if (!write_instance(inst, in_path, cfg.id_kind, cfg.seed)) return -1.0;
    double best = -1.0;
    for (int r = 0; r < repeats; ++r) {
      RunOutcome o = run_child(command_for(cfg.solver), in_path, stdout_target, err_path,
                               cfg.time_limit_s, cfg.mem_limit_mb);
      ++total_runs;
      if (o.timed_out || o.term_signal != 0 || !o.launched) return -1.0;
      best = (best < 0) ? o.wall_ms : std::min(best, o.wall_ms);
    }
    return best;
  };

  std::vector<FamilyResult> results;

  // ---- optional correctness sweep, independent of any timing ------------
  if (cfg.crosscheck > 0) {
    std::printf("correctness sweep: %d random small instances\n", cfg.crosscheck);
    const std::vector<std::string> mix = {
        "triangle-union", "c5-union",  "petersen-union", "path",
        "cycle",          "star-clique", "random-gnp:25",  "parallel-heavy:3"};
    Rng rng(mix_seed(cfg.seed, 4242));
    int failures = 0, checked = 0;
    for (int t = 0; t < cfg.crosscheck; ++t) {
      const std::string &fam = mix[static_cast<size_t>(t) % mix.size()];
      int n = 3 + rng.below(58);
      Instance inst = generate(fam, n, cfg.max_edges, mix_seed(cfg.seed, t + 1));
      if (inst.skipped) continue;
      if (timed_run(inst, 1) < 0) {
        std::printf("  [%d] %s n=%d: did not complete\n", t, fam.c_str(), n);
        ++failures;
        continue;
      }
      ParsedOutput parsed = parse_output(out_path, inst, cfg.protocol);
      VerifyOutcome vo = verify(inst, parsed, cfg.verify_max_n, cfg.verify_max_m,
                                cfg.verify_work_budget, true);
      ++checked;
      bool ok = parsed.ok && vo.feasible && vo.checked && vo.optimal;
      if (!ok) {
        ++failures;
        std::printf("  [%d] %s n=%d m=%lld k=%d: %s\n", t, fam.c_str(), n, inst.m(),
                    vo.k, parsed.ok ? vo.detail.c_str() : parsed.why.c_str());
      }
    }
    std::printf("  %d checked, %d rejected\n\n", checked, failures);
    if (failures > 0) {
      std::printf("CORRECTNESS FAILURE: the program is not a correct maximum-matching\n"
                  "solver on these instances; its complexity claim is moot until fixed.\n");
      return 1;
    }
  }

  for (const auto &family : cfg.families) {
    FamilyResult fr;
    fr.family = family;
    // Calibration: grow the ladder until the largest instance is measurably
    // above process startup, otherwise every point is noise.
    std::vector<int> sizes = cfg.sizes;
    if (cfg.calibrate) {
      for (int round = 0; round < 6; ++round) {
        int probe_n = sizes.front();
        Instance probe = generate(family, probe_n, cfg.max_edges, cfg.seed);
        if (probe.skipped) break;
        double t = timed_run(probe, 1);
        if (t < 0) break;
        // Every ladder point must be above the noise floor, so calibrate the
        // smallest one; the rest are larger and therefore measurable too.
        if (t - cfg.baseline_ms >= 8.0) break;
        bool can_grow = true;
        for (int v : sizes)
          if (v > cfg.max_n / 2) can_grow = false;
        if (!can_grow) break;
        for (int &v : sizes) v *= 2;
      }
    }
    fr.sizes_used = sizes;
    if (!cfg.quiet) {
      std::printf("== family %s ==\n", family.c_str());
      if (sizes != cfg.sizes) {
        std::printf("  ladder rescaled to");
        for (int v : sizes) std::printf(" %d", v);
        std::printf("\n");
      }
    }
    if (!cfg.quiet)
      std::printf("  %8s %10s %10s %10s %12s %9s %7s\n", "n", "m", "m_s",
                  "time_ms", "ns/edge", "rss_MB", "|M|");

    for (int n : sizes) {
      Instance inst = generate(family, n, cfg.max_edges, cfg.seed);
      Point p;
      p.n = n;
      p.m = inst.m();
      p.ms = inst.m_distinct();
      if (inst.skipped) {
        p.skipped = true;
        fr.points.push_back(p);
        if (!cfg.quiet) human_point_row(p);
        continue;
      }
      if (!write_instance(inst, in_path, cfg.id_kind, cfg.seed)) {
        p.note = "cannot write instance";
        fr.points.push_back(p);
        continue;
      }
      // One discarded warm-up run: the first execution of a large instance pays
      // page faults and cold caches, which would inflate the spread and hide
      // every point behind the noise floor.
      {
        RunOutcome warm = run_child(command_for(cfg.solver), in_path, stdout_target,
                                    err_path, cfg.time_limit_s, cfg.mem_limit_mb);
        ++total_runs;
        if (warm.timed_out) {
          p.ran = false;
          p.timed_out = true;
          p.note = "TIMEOUT after " + fmt("%.0f", cfg.time_limit_s) + " s";
          fr.points.push_back(p);
          if (!cfg.quiet) human_point_row(p);
          continue;
        }
      }
      std::vector<double> walls;
      RunOutcome last;
      bool finished = false;
      for (int r = 0; r < cfg.repeats; ++r) {
        RunOutcome o = run_child(command_for(cfg.solver), in_path, stdout_target, err_path,
                                 cfg.time_limit_s, cfg.mem_limit_mb);
        ++total_runs;
        last = o;
        if (o.timed_out) break;
        if (!o.launched) break;
        if (o.term_signal != 0) break;
        walls.push_back(o.wall_ms);
        finished = true;
      }
      if (last.timed_out) {
        p.ran = false;
        p.timed_out = true;
        p.note = "TIMEOUT after " + fmt("%.0f", cfg.time_limit_s) + " s";
        fr.points.push_back(p);
        if (!cfg.quiet) human_point_row(p);
        continue;
      }
      if (!finished || walls.empty()) {
        p.ran = false;
        p.note = "did not complete (signal " + std::to_string(last.term_signal) +
                 ", exit " + std::to_string(last.exit_code) + ")";
        fr.points.push_back(p);
        if (!cfg.quiet) human_point_row(p);
        continue;
      }
      std::sort(walls.begin(), walls.end());
      p.ran = true;
      p.wall_best = walls.front();
      p.wall_med = walls[walls.size() / 2];
      double spread = walls.back() - walls.front();
      p.spread = p.wall_med > 0 ? spread / p.wall_med : 0.0;
      p.rss_mb = last.max_rss_mb;
      p.t = std::max(0.0, p.wall_best - cfg.baseline_ms);
      // The best of several runs is a lower-biased but consistent estimator, so
      // scheduling noise inflates scatter rather than bias. Only a time that is
      // not resolvable above the startup spread is discarded outright; a large
      // per-instance spread is reported instead of hidden.
      double floor_ms = std::max(cfg.noise_floor_ms, 2.0 * cfg.baseline_spread_ms);
      p.noise_dominated = p.t < floor_ms;
      if (p.noise_dominated)
        p.note = "below measurement resolution; increase --sizes";
      ++total_instances;

      if (cfg.verify) {
        ParsedOutput parsed = parse_output(out_path, inst, cfg.protocol);
        VerifyOutcome vo = verify(inst, parsed, cfg.verify_max_n, cfg.verify_max_m,
                                  cfg.verify_work_budget, inst.n <= 600);
        if (!parsed.ok) {
          p.correctness_failed = true;
          p.note = "OUTPUT REJECTED: " + parsed.why;
        } else if (!vo.feasible) {
          p.correctness_failed = true;
          p.note = "INFEASIBLE: " + vo.detail;
        } else if (vo.checked && !vo.optimal) {
          p.correctness_failed = true;
          std::ostringstream w;
          w << "NOT MAXIMUM (augmenting path of " << vo.witness.size() << " vertices:";
          for (size_t i = 0; i < vo.witness.size() && i < 12; ++i)
            w << " " << vo.witness[i];
          if (vo.witness.size() > 12) w << " ...";
          w << ")";
          p.note = w.str();
        } else if (vo.checked) {
          p.verified = true;
          ++total_verified;
          p.k = vo.k;
          if (vo.nu >= 0 && vo.nu != vo.k) {
            p.correctness_failed = true;
            p.note = "cardinality " + std::to_string(vo.k) + " != computed optimum " +
                     std::to_string(vo.nu);
          }
        } else {
          p.note = "feasible only (verification budget skipped)";
          p.k = vo.k;
        }
      } else {
        p.note = "verification disabled";
      }
      if (p.correctness_failed) fr.correctness_failure = true;
      fr.points.push_back(p);
      if (!cfg.quiet && p.correctness_failed)
        std::printf("  %8d %10lld %10lld %10.1f %12s %9.1f %7d  [WRONG] %s\n", p.n,
                    p.m, p.ms, p.t, "-", p.rss_mb, p.k, p.note.c_str());
      else if (!cfg.quiet)
        human_point_row(p);
    }

    // ---- fits and verdict ------------------------------------------------
    int usable = 0;
    for (const auto &p : fr.points)
      if (p.ran && !p.noise_dominated && p.t > 0) ++usable;
    for (int s = 0; s < S_COUNT; ++s) fr.fits[s] = fit_shape(s, fr.points);
    fr.best = fr.fits[S_CLAIM];
    for (int s = 0; s < S_COUNT; ++s)
      if (fr.fits[s].valid && fr.fits[s].rmse < fr.best.rmse) fr.best = fr.fits[s];
    fr.has_exponent_m = loglog_slope(fr.points, false, &fr.exponent_m, &fr.exponent_m_se);
    fr.has_exponent_n = loglog_slope(fr.points, true, &fr.exponent_n, &fr.exponent_n_se);
    {
      double lo_m = 1e30, hi_m = 0, lo_n = 1e30, hi_n = 0;
      for (const auto &p : fr.points) {
        if (!p.ran || p.noise_dominated || p.skipped || p.t <= 0) continue;
        if (p.m >= 1) { lo_m = std::min(lo_m, static_cast<double>(p.m)); hi_m = std::max(hi_m, static_cast<double>(p.m)); }
        lo_n = std::min(lo_n, static_cast<double>(p.n)); hi_n = std::max(hi_n, static_cast<double>(p.n));
      }
      fr.span_m = hi_m > 0 ? hi_m / lo_m : 0;
      fr.span_n = hi_n > 0 ? hi_n / lo_n : 0;
    }

    // Noise floor from the observed repeat spread, then the verdict threshold.
    double sigma = 0;
    int sigma_n = 0;
    for (const auto &p : fr.points)
      if (p.ran && !p.noise_dominated) {
        sigma += p.spread;
        ++sigma_n;
      }
    if (sigma_n) sigma /= sigma_n;
    double thr = std::max(0.08, 3.0 * sigma);

    if (fr.correctness_failure) {
      fr.verdict = "CORRECTNESS FAILURE";
      fr.evidence = "at least one returned matching was rejected by the independent verifier";
    } else if (usable < 3) {
      fr.verdict = "INCONCLUSIVE";
      fr.evidence = "fewer than three usable (non-noise-dominated) points";
    } else {
      const Fit &claim = fr.fits[S_CLAIM];
      bool claim_ok = claim.valid && claim.rmse <= thr;
      // A size-independent constant cannot turn a slope of 1.5 into 1.0, so a
      // significantly superlinear fitted slope rejects the class even when the
      // whole-shape residual is small on a short ladder.
      bool slope_ok = true;
      bool use_n_driver = false;
      double a_hat = 0, a_se = 0, span = 0;
      if (fr.has_exponent_m && fr.span_m >= 4.0) {
        a_hat = fr.exponent_m;
        a_se = fr.exponent_m_se;
        span = fr.span_m;
      } else if (fr.has_exponent_n && fr.span_n >= 4.0) {
        use_n_driver = true;
        a_hat = fr.exponent_n;
        a_se = fr.exponent_n_se;
        span = fr.span_n;
      } else {
        slope_ok = false;  // not enough span to say anything
      }
      // The claimed first term is linear in the driver, so a slope significantly
      // above one already contradicts the claim.
      bool slope_rejects = slope_ok && (a_hat - 2.0 * a_se > 1.15);
      fr.claim_consistent = claim_ok && !slope_rejects;
      fr.claim_refuted = !fr.claim_consistent;
      std::ostringstream ev;
      ev << std::fixed << std::setprecision(3);
      ev << "claim fit rmse=" << claim.rmse << " (threshold " << thr << ")";
      if (claim.valid) ev << ", C=" << claim.param;
      ev << "; best model: " << kShapeNames[fr.best.shape] << " rmse=" << fr.best.rmse;
      if (slope_ok)
        ev << "; growth " << (use_n_driver ? "n^" : "m^") << a_hat << " +- " << a_se
           << " over a " << span << "x span";
      fr.evidence = ev.str();
      if (!claim_ok || slope_rejects) {
        fr.verdict = "REFUTED on this family";
        if (slope_rejects && claim_ok)
          fr.evidence += " | the fitted slope is significantly above the claimed class";
      } else if (fr.best.shape != S_CLAIM && fr.best.rmse + 1e-9 < claim.rmse) {
        fr.verdict = "consistent (not the best-fitting model)";
      } else {
        fr.verdict = "consistent";
      }
    }
    if (!cfg.quiet) {
      std::printf("  -> %s\n", fr.verdict.c_str());
      std::printf("     %s\n\n", fr.evidence.c_str());
    }
    results.push_back(fr);
  }

  // --------------------------------------------------------------- summary
  int refuted = 0, wrong = 0, inconclusive = 0, consistent = 0;
  for (const auto &r : results) {
    if (r.correctness_failure) ++wrong;
    else if (r.claim_refuted) ++refuted;
    else if (r.verdict == "INCONCLUSIVE") ++inconclusive;
    else ++consistent;
  }

  std::printf("----------------------------------------------------------------\n");
  std::printf("AUDIT SUMMARY  (%d families, %d instances, %d runs, %d verified)\n",
              static_cast<int>(results.size()), total_instances, total_runs,
              total_verified);
  std::printf("  %-24s %-34s %s\n", "family", "verdict", "measured growth");
  for (const auto &r : results) {
    std::string growth = "-";
    if (r.has_exponent_m) growth = "m^" + fmt("%.2f", r.exponent_m);
    if (r.has_exponent_n) growth += " / n^" + fmt("%.2f", r.exponent_n);
    std::printf("  %-24s %-34s %s\n", r.family.c_str(), r.verdict.c_str(),
                growth.c_str());
  }
  std::printf("correctness certified by the independent verifier on %d of %d "
              "instances;\n  the rest are feasibility-only and cannot support a "
              "correctness claim.\n", total_verified, total_instances);
  std::printf("----------------------------------------------------------------\n");
  if (wrong > 0)
    std::printf("VERDICT: FAIL — %d family/families returned an incorrect matching.\n"
                "         A rejected output is a definite counterexample and does not\n"
                "         depend on any timing argument.\n", wrong);
  else if (refuted > 0)
    std::printf("VERDICT: CLAIM NOT SUPPORTED — %d family/families grow faster than the\n"
                "         claimed class on the tested ladder (see per-family rmse).\n",
                refuted);
  else if (inconclusive > 0)
    std::printf("VERDICT: INCONCLUSIVE — %d family/families had too few usable points;\n"
                "         raise --sizes or --repeats.\n", inconclusive);
  else
    std::printf("VERDICT: NOT REFUTED on the tested families and sizes.\n");
  std::printf(
      "Scope: measurement can falsify a growth claim and can certify each returned\n"
      "matching; it cannot prove an asymptotic bound. Recorded protocol, families,\n"
      "sizes, repeats, noise floor and thresholds are part of the evidence.\n");

  // ------------------------------------------------------------------ files
  if (!cfg.json_path.empty()) {
    std::ofstream j(cfg.json_path);
    j << "{\n  \"tool\": \"" << kVersion << "\",\n  \"timestamp\": \"" << now_iso()
      << "\",\n  \"solver\": \"" << cfg.solver << "\",\n  \"protocol\": \""
      << (cfg.protocol ? "ids" : "endpoints") << "\",\n  \"repeats\": " << cfg.repeats
      << ",\n  \"baseline_ms\": " << cfg.baseline_ms << ",\n  \"claim\": \""
      << (cfg.claim_is_near_linear ? "near-linear" : "none") << "\",\n  \"families\": [\n";
    for (size_t f = 0; f < results.size(); ++f) {
      const auto &r = results[f];
      j << "    {\n      \"family\": \"" << r.family << "\",\n      \"verdict\": \""
        << r.verdict << "\",\n      \"evidence\": \"" << r.evidence << "\",\n";
      j << "      \"exponent_m\": " << (r.has_exponent_m ? r.exponent_m : 0)
        << ",\n      \"exponent_m_se\": " << (r.has_exponent_m ? r.exponent_m_se : 0)
        << ",\n      \"exponent_n\": " << (r.has_exponent_n ? r.exponent_n : 0)
        << ",\n      \"fits\": [";
      bool first = true;
      for (int s = 0; s < S_COUNT; ++s) {
        if (!r.fits[s].valid) continue;
        if (!first) j << ",";
        first = false;
        j << "\n        {\"model\": \"" << kShapeNames[s] << "\", \"rmse\": "
          << r.fits[s].rmse << ", \"param\": " << r.fits[s].param << ", \"k\": "
          << r.fits[s].k << "}";
      }
      j << "\n      ],\n      \"points\": [";
      first = true;
      for (const auto &p : r.points) {
        if (!p.ran) continue;
        if (!first) j << ",";
        first = false;
        j << "\n        {\"n\": " << p.n << ", \"m\": " << p.m << ", \"m_s\": " << p.ms
          << ", \"t_ms\": " << p.t << ", \"rss_mb\": " << p.rss_mb
          << ", \"k\": " << p.k << ", \"verified\": " << (p.verified ? "true" : "false")
          << ", \"correctness_failed\": " << (p.correctness_failed ? "true" : "false")
          << ", \"spread\": " << p.spread
          << ", \"noise_dominated\": " << (p.noise_dominated ? "true" : "false") << "}";
      }
      j << "\n      ]\n    }" << (f + 1 < results.size() ? "," : "") << "\n";
    }
    j << "  ],\n  \"summary\": {\"refuted\": " << refuted << ", \"wrong\": " << wrong
      << ", \"inconclusive\": " << inconclusive << ", \"consistent\": " << consistent
      << "}\n}\n";
  }

  if (!cfg.md_path.empty()) {
    std::ofstream m(cfg.md_path);
    m << "# Matching complexity audit\n\n";
    m << "- Tool: `" << kVersion << "`\n- Date: " << now_iso() << "\n";
    m << "- Program under audit: `" << cfg.solver << "`\n";
    m << "- Protocol: " << (cfg.protocol ? "edge indices" : "vertex endpoints")
      << ", starting ladder: ";
    for (size_t i = 0; i < cfg.sizes.size(); ++i)
      m << (i ? ", " : "") << cfg.sizes[i];
    m << " (each family below lists the ladder it actually used)";
    m << "\n- Repeats per point: " << cfg.repeats << "; timing: best wall time minus a "
      << fmt("%.2f", cfg.baseline_ms) << " ms startup baseline\n";
    m << "- Optimality checks: independent blossom search up to n = "
      << cfg.verify_max_n << "\n\n";
    m << "## What this report establishes\n\n";
    m << "Every accepted matching was checked by an independent matcher "
         "(Berge's augmenting-path criterion). Growth is compared with candidate "
         "complexity classes by log-log least squares. Measurement can refute a "
         "growth claim; it cannot prove one. A `consistent` verdict means the "
         "measured points are compatible with the class, not that the class is true.\n\n";
    m << "## Results\n\n| family | verdict | m^slope | n^slope | claim rmse | best model |\n";
    m << "|---|---|---|---|---|---|\n";
    for (const auto &r : results) {
      m << "| `" << r.family << "` | " << r.verdict << " | ";
      if (r.has_exponent_m) m << fmt("%.2f", r.exponent_m) << " +- " << fmt("%.2f", r.exponent_m_se);
      else m << "-";
      m << " | ";
      if (r.has_exponent_n) m << fmt("%.2f", r.exponent_n);
      else m << "-";
      m << " | " << (r.fits[S_CLAIM].valid ? fmt("%.3f", r.fits[S_CLAIM].rmse) : "-");
      m << " | " << (r.best.valid ? kShapeNames[r.best.shape] : "-") << " |\n";
    }
    m << "\n## Points\n\n";
    for (const auto &r : results) {
      m << "### `" << r.family << "`\n\nladder:";
      for (int v : r.sizes_used) m << " " << v;
      m << "\n\n| n | m | m_s | time ms | ns/edge | RSS MB "
           "| \\|M\\| | status |\n|---|---|---|---|---|---|---|---|\n";
      for (const auto &p : r.points) {
        if (p.skipped || !p.ran) {
          m << "| " << p.n << " | - | - | - | - | - | - | "
            << (p.skipped ? "skipped (edge cap)" : p.note) << " |\n";
          continue;
        }
        double per_edge = p.m > 0 ? p.t * 1e6 / static_cast<double>(p.m) : 0;
        m << "| " << p.n << " | " << p.m << " | " << p.ms << " | " << fmt("%.1f", p.t)
          << " | " << fmt("%.1f", per_edge) << " | " << fmt("%.1f", p.rss_mb) << " | "
          << p.k << " | " << (p.correctness_failed ? "**WRONG**"
                              : p.verified ? "verified" : "feasible only")
          << (p.noise_dominated ? " (noise)" : "") << " |\n";
      }
      m << "\n" << r.evidence << "\n\n";
    }
    m << "## Limits\n\n";
    m << "1. A correct implementation of an asymptotically better algorithm can be "
         "slower than a simple one on every size that fits in memory; a flat ladder "
         "does not refute the claim.\n";
    m << "2. The fitted exponent has a standard error; conclusions at the 0.05 level "
         "need several ladder points above the noise floor.\n";
    m << "3. Timing includes process startup, I/O and the interpreter or runtime of "
         "the audited program.\n";
    m << "4. Optimality is decided only below the verification budget; larger "
         "instances contribute feasibility evidence only and are marked as such.\n";
    m << "5. Passing this audit does not establish a worst-case bound, a constant "
         "independent of input, or determinism.\n";
  }

  if (!cfg.keep_temp && cfg.workdir.empty()) {
    std::remove(in_path.c_str());
    std::remove(out_path.c_str());
    std::remove(err_path.c_str());
    rmdir(workdir.c_str());
  }

  if (wrong > 0) return 1;
  return 0;
}

// trivial_scan.cpp — control program for the audit harness.
//
// It touches every vertex and every edge record, then returns an empty
// matching. Its true cost is Theta(n + m), so on the `empty` family the audit
// must NOT refute it: a harness that rejects every program is useless. On any
// family with edges it is wrong, and the audit must report a correctness
// failure instead of a complexity verdict.
//
// Use:  matchaudit --solver ./trivial_scan --families empty --max-n 2000000

#include <cstdio>
#include <vector>

int main() {
  int n = 0;
  long long m = 0;
  if (std::scanf("%d %lld", &n, &m) != 2) return 0;
  // Theta(n): touch every vertex so the measured cost exposes the O(n) term.
  std::vector<int> mark(static_cast<size_t>(n < 0 ? 0 : n), 0);
  long long touched = 0;
  for (int i = 0; i < n; ++i) mark[static_cast<size_t>(i)] = i & 1;
  for (int i = 0; i < n; ++i) touched += mark[static_cast<size_t>(i)];
  for (long long i = 0; i < m; ++i) {
    int u = 0, v = 0;
    if (std::scanf("%d %d", &u, &v) != 2) break;
    touched += u + v;
  }
  std::printf("0\n");
  // Never true; keeps the loops above from being optimised away.
  return touched == 123456789012345LL ? 1 : 0;
}

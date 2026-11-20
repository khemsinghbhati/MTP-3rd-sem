"""Brute-force checks of every identity and reduction used in the report.

Checks (all on random small instances, compared against networkx ground truth):
  C1  sum_x |K_x| <= n + m                               (clique-size bound)
  C2  W(x,y) = |intersection of separators on path x..y|  (path-intersection identity)
  C3  sum_v |T_v|^2 = sum_{x,y} W(x,y)  (ordered, incl. diagonal)
  C4  Ibarra Insert-Query criterion == is_chordal(G + uv)  (Theorems 2.4/2.5 of the thesis)
  C5  when the answer is "yes", W(x,y) == min edge weight on the path (so Insert never needs W)
  C6  every pair of clique-tree nodes at distance >= 2 is the closest pair of some non-edge uv
  C7  BMM reduction: Insert-Query(p_i, q_j) == "yes"  iff  (A*B)[i][j] == 0
  C8  common-neighbourhood criterion: G+uv chordal iff N(u)&N(v) separates u,v
"""
import itertools
import random
import sys

import networkx as nx


def random_chordal(n, host_size, rng, forest=False):
    """Intersection graph of random subtrees of a random host tree (Gavril 1974)."""
    host = nx.random_labeled_tree(host_size, seed=rng.randrange(10**9)) if host_size > 1 else nx.empty_graph(1)
    if forest and host_size > 3:
        # delete a random edge or two to obtain a host forest -> disconnected graphs
        for _ in range(rng.randint(1, 2)):
            e = rng.choice(list(host.edges()))
            host.remove_edge(*e)
    subtrees = []
    for _ in range(n):
        start = rng.randrange(host_size)
        size = rng.randint(1, max(1, host_size // 2))
        comp = {start}
        frontier = list(host.neighbors(start))
        while len(comp) < size and frontier:
            nxt = frontier.pop(rng.randrange(len(frontier)))
            if nxt in comp:
                continue
            comp.add(nxt)
            frontier.extend(w for w in host.neighbors(nxt) if w not in comp)
        subtrees.append(comp)
    G = nx.Graph()
    G.add_nodes_from(range(n))
    for a, b in itertools.combinations(range(n), 2):
        if subtrees[a] & subtrees[b]:
            G.add_edge(a, b)
    assert nx.is_chordal(G)
    return G


def clique_tree(G):
    """Maximal cliques + a clique tree = max-weight spanning tree of the complete
    clique graph with weights |Ki & Kj| (weight-0 'dummy' edges join components,
    exactly as in Ibarra's thesis Sec. 2.2)."""
    cliques = [frozenset(c) for c in nx.find_cliques(G)]
    c = len(cliques)
    H = nx.Graph()
    H.add_nodes_from(range(c))
    for i, j in itertools.combinations(range(c), 2):
        H.add_edge(i, j, weight=len(cliques[i] & cliques[j]))
    T = nx.maximum_spanning_tree(H) if c > 1 else H
    # check induced-subtree property
    for v in G.nodes():
        nodes = [i for i in range(c) if v in cliques[i]]
        assert nx.is_connected(T.subgraph(nodes)), "not a clique tree"
    return cliques, T


def insert_query(G, cliques, T, u, v):
    """Ibarra's Insert-Query (thesis Sec. 2.3.3).  Returns (answer, x, y, W(x,y), minweight)."""
    Tu = {i for i, K in enumerate(cliques) if u in K}
    Tv = {i for i, K in enumerate(cliques) if v in K}
    assert not (Tu & Tv)
    # closest pair: multi-source BFS from Tu
    best = None
    for x in Tu:
        dist = nx.single_source_shortest_path(T, x)
        for y in Tv:
            p = dist[y]
            if best is None or len(p) < len(best):
                best = p
    path = best
    x, y = path[0], path[-1]
    Wxy = len(cliques[x] & cliques[y])
    if len(path) == 2:
        return True, x, y, Wxy, Wxy, path
    mw = min(T[a][b]["weight"] for a, b in zip(path, path[1:]))
    assert mw >= Wxy  # clique-intersection property
    return mw == Wxy, x, y, Wxy, mw, path


def check_graph(G, stats):
    n, m = G.number_of_nodes(), G.number_of_edges()
    cliques, T = clique_tree(G)
    c = len(cliques)
    # C1
    assert sum(len(K) for K in cliques) <= n + m
    # C2, C3
    W = [[len(cliques[i] & cliques[j]) for j in range(c)] for i in range(c)]
    for x, y in itertools.combinations(range(c), 2):
        path = nx.shortest_path(T, x, y)
        inter = None
        for a, b in zip(path, path[1:]):
            S = cliques[a] & cliques[b]
            inter = S if inter is None else inter & S
        assert len(inter) == W[x][y]
    Tsz = {v: sum(1 for K in cliques if v in K) for v in G.nodes()}
    assert sum(t * t for t in Tsz.values()) == sum(sum(r) for r in W)
    # C4, C5, C8
    for u, v in itertools.combinations(G.nodes(), 2):
        if G.has_edge(u, v):
            continue
        ans, x, y, Wxy, mw, path = insert_query(G, cliques, T, u, v)
        H = G.copy()
        H.add_edge(u, v)
        truth = nx.is_chordal(H)
        assert ans == truth, (u, v)
        if truth:
            assert Wxy == mw  # C5
        common = set(G[u]) & set(G[v])
        R = G.copy()
        R.remove_nodes_from(common)
        separates = not nx.has_path(R, u, v)
        assert separates == truth  # C8
        stats["queries"] += 1
    # C6: every node pair at tree distance >= 2 is realisable as a closest pair
    for x, y in itertools.combinations(range(c), 2):
        path = nx.shortest_path(T, x, y)
        if len(path) < 3:
            continue
        xn, yn = path[1], path[-2]
        U = cliques[x] - cliques[xn]
        V = cliques[y] - cliques[yn]
        assert U and V
        u, v = min(U), min(V)
        assert not G.has_edge(u, v)
        _, x2, y2, *_ = insert_query(G, cliques, T, u, v)
        assert {x2, y2} == {x, y}
        stats["c6"] += 1
    stats["graphs"] += 1


def bmm_instance(A, B):
    """Reduction of Section 4.4: n = 5u vertices, m = Theta(u^2)."""
    u = len(A)
    U = [("e", k) for k in range(u)]
    D = [("d", k) for k in range(u)]
    E = [("f", k) for k in range(u)]
    universe = U + D + E
    G = nx.Graph()
    G.add_nodes_from(universe)
    G.add_edges_from(itertools.combinations(universe, 2))  # K_l = universe
    P, Q = [], []
    for i in range(u):
        Ai = [("e", k) for k in range(u) if A[i][k]]
        Ri = Ai + D[: u - len(Ai)]  # |R_i| = u
        p = ("p", i)
        G.add_edges_from((p, w) for w in Ri)
        P.append(p)
    for j in range(u):
        Xj = [("e", k) for k in range(u) if not B[k][j]]
        Qj = Xj + D + E[: u - len(Xj)]  # |Q_j| = 2u
        q = ("q", j)
        G.add_edges_from((q, w) for w in Qj)
        Q.append(q)
    return G, P, Q


def check_bmm(rng, stats):
    u = rng.randint(1, 5)
    A = [[rng.random() < 0.4 for _ in range(u)] for _ in range(u)]
    B = [[rng.random() < 0.4 for _ in range(u)] for _ in range(u)]
    G, P, Q = bmm_instance(A, B)
    assert nx.is_chordal(G)
    n, m = G.number_of_nodes(), G.number_of_edges()
    assert n == 5 * u
    cliques, T = clique_tree(G)
    for i in range(u):
        for j in range(u):
            Cij = any(A[i][k] and B[k][j] for k in range(u))
            ans, *_ = insert_query(G, cliques, T, P[i], Q[j])
            H = G.copy()
            H.add_edge(P[i], Q[j])
            assert ans == nx.is_chordal(H) == (not Cij)
            stats["bmm_queries"] += 1
    stats["bmm"] += 1
    stats["max_m_over_u2"] = max(stats["max_m_over_u2"], m / (u * u))


def main():
    rng = random.Random(int(sys.argv[1]) if len(sys.argv) > 1 else 1)
    stats = dict(graphs=0, queries=0, c6=0, bmm=0, bmm_queries=0, max_m_over_u2=0.0)
    for trial in range(400):
        n = rng.randint(1, 11)
        host = rng.randint(1, 9)
        G = random_chordal(n, host, rng, forest=(trial % 3 == 0))
        check_graph(G, stats)
    # named families
    fam = {
        "empty": nx.empty_graph(9),
        "path": nx.path_graph(9),
        "star": nx.star_graph(8),
        "complete": nx.complete_graph(7),
        "tree": nx.random_labeled_tree(10, seed=3),
        "user_counterexample": nx.Graph([("a", "b"), ("a", "x"), ("b", "x"), ("a", "c"), ("b", "c"),
                                         ("b", "y"), ("c", "y")]),
    }
    for name, G in fam.items():
        check_graph(nx.convert_node_labels_to_integers(G), stats)
    for _ in range(150):
        check_bmm(rng, stats)
    print("ALL CHECKS PASSED", stats)


if __name__ == "__main__":
    main()
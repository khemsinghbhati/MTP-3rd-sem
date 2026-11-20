"""Check the W-update formulas of Section 6.3 (Ibarra thesis 2.5.1) by simulating Insert/Delete."""
import itertools, random, sys
import networkx as nx
# The companion verification module is located alongside this script.
from verify_chordal_identities import random_chordal, clique_tree, insert_query

def check_insert(G, rng):
    cliques, T = clique_tree(G)
    nonedges = [(u, v) for u, v in itertools.combinations(G.nodes(), 2) if not G.has_edge(u, v)]
    rng.shuffle(nonedges)
    done = 0
    for u, v in nonedges:
        ans, x, y, Wxy, mw, path = insert_query(G, cliques, T, u, v)
        if not ans:
            continue
        T2 = T.copy()
        if len(path) > 2:  # exchange e* for {x,y}
            for a, b in zip(path, path[1:]):
                if T[a][b]["weight"] == mw:
                    T2.remove_edge(a, b); break
            T2.add_edge(x, y, weight=Wxy)
        I = cliques[x] & cliques[y]
        Kz = I | {u, v}
        xmax = len(cliques[x]) != len(I) + 1
        ymax = len(cliques[y]) != len(I) + 1
        H = T2.copy(); H.remove_edge(x, y)
        xside = nx.node_connected_component(H, x)
        W = lambda a, b: len(cliques[a] & cliques[b])
        for w in range(len(cliques)):
            Kw = cliques[w]
            if not xmax:   pred = W(w, x) + (v in Kw)
            elif not ymax: pred = W(w, y) + (u in Kw)
            elif w in xside: pred = W(w, y) + (u in Kw)
            else:          pred = W(w, x) + (v in Kw)
            assert pred == len(Kw & Kz), (w, pred, len(Kw & Kz))
        done += 1
        if done >= 3: break
    return done

def check_delete(G):
    cliques, _ = clique_tree(G)
    cnt = 0
    for u, v in G.edges():
        holders = [i for i, K in enumerate(cliques) if u in K and v in K]
        if len(holders) != 1: continue
        x = holders[0]
        K1, K2 = cliques[x] - {v}, cliques[x] - {u}
        for w, Kw in enumerate(cliques):
            if w == x: continue
            assert len(Kw & K1) == len(Kw & cliques[x]) - (v in Kw)
            assert len(Kw & K2) == len(Kw & cliques[x]) - (u in Kw)
        assert len(K1 & K2) == len(cliques[x]) - 2
        cnt += 1
    return cnt

rng = random.Random(3)
ins = dele = 0
for t in range(500):
    G = random_chordal(rng.randint(2, 12), rng.randint(1, 9), rng, forest=(t % 3 == 0))
    ins += check_insert(G, rng); dele += check_delete(G)
print("W update formulas OK: inserts checked", ins, "deletes checked", dele)
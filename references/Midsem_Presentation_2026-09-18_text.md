# Midsemester presentation — extracted text

**Source:** `Midsem_Presentation_.pdf` (September 18, 2026, 21 pages).

This is the text extracted from the uploaded original PDF, preserved for search and version control. It is **not** a visually faithful substitute for the source PDF (figures, graphs, mathematical formatting, and slide layout may be missing or distorted). The detailed working report is `../docs/MTP_REPORT.md`.

---



## PDF page 1 of 21

                                     MTP Thesis Progress: Chordal Graphs
                                      Theoretical Foundations, MCS, and Maximal Cliques



                                                  Khem Singh (25cs06004)

                                                   MidSemester Thesis Presentation


                                                     September 18, 2026




Khem Singh (MidSemester Thesis Presentation)              MCS & Chordal Graphs            September 18, 2026   1 / 21

## PDF page 2 of 21

Outline

1   Overall Objectives

2   Theoretical Foundations

3   Maximum Cardinality Search (MCS)

4   Formal Proof of Correctness

5   Implementation & Complexity

6   Dry Run

7   Maximal Clique Extraction

8   Project Progress & Roadmap


    Khem Singh (MidSemester Thesis Presentation)   MCS & Chordal Graphs   September 18, 2026   2 / 21

## PDF page 3 of 21

Overall Objectives of the paper

Reference: Louis Ibarra (DOI: 10.1145/1383369.1383371)

The primary objective of the target literature is to develop the first dynamic algorithm that maintains
a clique tree representation of a chordal graph.
      Core Supported Operations:
          Query whether deleting or inserting an arbitrary edge preserves chordality.
          Delete or insert an arbitrary edge, provided it preserves chordality.
     Implementation Complexities (n = vertices, m = edges):
         First Implementation: Each operation runs in O(n) time.
         Second Implementation: Insertion query runs in O(log2 n) time, insertion in O(n) time,
         deletion query in O(n) time, and deletion in O(n log n) time.
     Advanced
       √      Deletion Query: A specialized data structure allowing a deletion query to run in
     O( m) time. Updating this data structure after a deletion or insertion requires O(m) time.


  Khem Singh (MidSemester Thesis Presentation)   MCS & Chordal Graphs           September 18, 2026    3 / 21

## PDF page 4 of 21

Theoretical Foundations

Definition of a Chordal Graph
     Let G = (V , E ) be an undirected graph.
     G is defined as chordal if every cycle of length 4 or more has a chord (an edge joining two
     nonconsecutive vertices of the cycle).
     Often referred to as triangulated graphs because no chordless cycle exceeds length 3.

Perfect Elimination Ordering (PEO)
     A specific ordering of vertices such that, for every vertex v , the set of neighbors of v that appear
     later in the ordering induces a complete subgraph.


Fundamental Theorem
A graph is chordal if and only if it has a Perfect Elimination Ordering. Chordality can be tested by
computing an ordering and verifying the PEO condition.


  Khem Singh (MidSemester Thesis Presentation)   MCS & Chordal Graphs               September 18, 2026       4 / 21

## PDF page 5 of 21

Maximum Cardinality Search (MCS)


Objective: For a chordal graph, compute a Perfect Elimination Ordering efficiently.

The MCS algorithm utilizes a greedy, backward-building approach:
     Initialization: Every vertex starts unnumbered with an initial weight w (v ) = 0.
     Backward Numbering: Iterates from index V − 1 down to 0.
     Selection: Extracts an unnumbered vertex with the highest weight (maximum already-numbered
     neighbors).
     Placement: Places the selected vertex at the current back-to-front index of the PEO.
     Weight Update: Immediately increments the weight of all its remaining unnumbered neighbors by
     1.




  Khem Singh (MidSemester Thesis Presentation)   MCS & Chordal Graphs             September 18, 2026   5 / 21

## PDF page 6 of 21

Formal Proof: Setup and Maximization

Theorem: If G = (V , E ) is chordal, the ordering α produced by MCS is a PEO.
Proof by Contradiction (Vandenberghe & Andersen):
     Let α(v ) denote the position of v (larger α(v ) = numbered earlier).
     Assume α is not a PEO. There exists a chordless path P = (v0 , . . . , vk ), k ≥ 2, where every
     interior vertex vi satisfies α(vi ) < α(v0 ) and α(vi ) < α(vk ).
     Maximization Step: Among all such paths, choose P such that min{α(v0 ), α(vk )} is as large as
     possible.
     Relabel endpoints such that α(v0 ) ≥ α(vk ). We obtain:

                                                 α(v0 ) > α(vk ) > α(v1 )

     Note: v0 is adjacent to v1 , but not vk (endpoints of a chordless path).



  Khem Singh (MidSemester Thesis Presentation)     MCS & Chordal Graphs            September 18, 2026   6 / 21

## PDF page 7 of 21

Formal Proof: MCS Weight Comparison

Weight Evaluation at Selection:
     Because α(vk ) > α(v1 ), vk was selected before v1 by MCS; at the moment vk was selected,
     weight(vk ) ≥ weight(v1 ).
     Vertex v0 is already numbered (α(v0 ) > α(vk )) and contributes +1 to weight(v1 ), but contributes
     0 to weight(vk ).
     To maintain weight(vk ) ≥ weight(v1 ), vk must possess an already-numbered neighbor z that v1
     lacks:
                                α(z) > α(vk ), {z, vk } ∈ E , {z, v1 } ∈
                                                                       /E

Iterative Path Construction:
     Let i be the smallest index in {2, . . . , k} where {vi , z} ∈ E .
     By minimality of i, z is adjacent to none of v1 , . . . , vi−1 .
     Therefore, (v0 , . . . , vi ) is a chordless sub-path, and (v0 , v1 , . . . , vi , z) has no chords except
     potentially {v0 , z}.

  Khem Singh (MidSemester Thesis Presentation)       MCS & Chordal Graphs                      September 18, 2026   7 / 21

## PDF page 8 of 21

Proof Visualization: The Mathematical Contradiction

                                                                              Dichotomy of {v0 , z}:
                                                                              Case 1: {v0 , z} ∈ E
                  Case 1: Edge closes
                  chordless cycle ≥ 4
                                                                                     Closes the path into a chordless cycle
                                                       z
                                                                                     v0 − v1 . . . vi − z − v0 of length i + 2 ≥ 4.
                                                           α(z) > α(vk )             Contradicts G being chordal.
                                         Min index i
                   Case 2: No edge creates
  v0                strictly superior path
                                                             vk               Case 2: {v0 , z} ∈
                                                                                               /E
                                                   vi                             Forms a new chordless path (v0 , . . . , vi , z) of the
                                                                                  forbidden type.
α(v0 ) > α(v1 )
                                          Chordless Path P                           min{α(v0 ), α(z)} > α(vk ) = min{α(v0 ), α(vk )}.
                            v1                                                       Contradicts the initial choice of P as the maximal
                                                                                     path.
                                                                              Conclusion: No such path exists; α is a PEO. ■


    Khem Singh (MidSemester Thesis Presentation)                           MCS & Chordal Graphs                     September 18, 2026      8 / 21

## PDF page 9 of 21

Data Structure Utilization


To strictly bound the time complexity to O(V + E ) and avoid O(V 2 ) scanning, the following data
structures are utilized:

     Bucket Array (buckets): An array of doubly linked lists. buckets[w] contains all unnumbered
     vertices that currently possess a weight of w .
     Max Weight Tracker (max_weight): A dynamically updated variable tracking the highest active
     weight, enabling O(1) extraction of the heaviest vertex.
     Iterator Array (node_it): Stores std::list iterators pointing to each vertex’s exact location in
     the bucket array. Permits O(1) erasure when upgrading a neighbor from buckets[w] to
     buckets[w+1].




  Khem Singh (MidSemester Thesis Presentation)   MCS & Chordal Graphs          September 18, 2026   9 / 21

## PDF page 10 of 21

Optimized MCS Algorithm
Algorithm 1 Optimized Maximum Cardinality Search (MCS)
 1: Initialize peo, weight array (all 0), numbered array (all false)
 2: Initialize buckets[0 . . . V − 1] and node_it iterators
 3: for i = 0 to V − 1 do Push i to front of buckets[0], Store iterator in node_it[i]
 4: end for
 5: max _weight ← 0
 6: for i = V − 1 down to 0 do
 7:     while buckets[max _weight] is empty do max _weight ← max _weight − 1
 8:     end while
 9:     v ← pop front of buckets[max _weight]
10:      peo[i] ← v ; numbered[v ] ← true
11:      for each u ∈ adjacency _list[v ] do
12:           if not numbered[u] then
13:               w ← weight[u]
14:               Erase u from buckets[w ] using node_it[u]                                                  ▷ O(1) removal
15:               weight[u] ← w + 1
16:               Push u to front of buckets[w + 1]; Update node_it[u]
17:               if weight[u] > max _weight then max _weight ← weight[u]
18:               end if
19:           end if
20:      end for
21: end for
22: return peo


   Khem Singh (MidSemester Thesis Presentation)               MCS & Chordal Graphs      September 18, 2026              10 / 21

## PDF page 11 of 21

Time Complexity Analysis: O(V + E )

Worst-Case Complexity Justification:
     Initialization: Executes exactly V times =⇒ O(V ).
     Main Loop: Extracts each vertex exactly once =⇒ O(V ).
     Inner Loop (Adjacency Scan): Processes exactly 2E entries (for a simple undirected graph
     represented by symmetric adjacency lists) =⇒ O(E ).
     List Operations: Insertions and deletions inside the inner loop execute in strict O(1) time via the
     node_it array.
Bounding the max_weight Decrements:
     The nested while loop contributes a total cost of O(E ) over the entire execution.
     max_weight only increments when an edge traversal increases a neighbor’s weight (maximum E
     global increments).
     Because max_weight starts at 0 and never drops below 0, every decrement must be charged to a
     preceding increment. Total decrements ≤ E .

  Khem Singh (MidSemester Thesis Presentation)   MCS & Chordal Graphs            September 18, 2026   11 / 21

## PDF page 12 of 21

Complete Dry Run of MCS (Part 1)


                                                    Initialization: buckets[0] = [6, 5, 4, 3, 2, 1, 0]. max_weight = 0.

              1                       3         6        Iteration i = 6: Pop 6. peo[6]=6.
                                                         Neighbors: {3}. Weight of 3 becomes 1.
                                                         buckets[1] = [3]. max_weight = 1.
   0
                                                         Iteration i = 5: max_weight is 1. Pop 3. peo[5]=3.
                                                         Neighbors: {1, 2, 6, 0}. Node 6 is numbered.
              2                       4                  Weights of 1, 2, 0 become 1.
                                                         buckets[1] = [0, 2, 1]. max_weight = 1.

                                                         Iteration i = 4: max_weight is 1. Pop 0. peo[4]=0.
                                                         Neighbors: {1, 2, 3}. Node 3 is numbered.
                                      5
                                                         Weights of 1, 2 become 2.
                                                         buckets[2] = [2, 1]. max_weight = 2.




 Khem Singh (MidSemester Thesis Presentation)          MCS & Chordal Graphs                      September 18, 2026       12 / 21

## PDF page 13 of 21

Complete Dry Run of MCS (Part 2)

                                                         Iteration i = 3: Pop 2. peo[3]=2.
                                                         Neighbors: {0, 3, 4, 5, 1}.
                                                         Weights of 4, 5 become 1. Weight of 1 becomes 3.
                                                         buckets[3] = [1]. max_weight = 3.
              1                       3         6
                                                         Iteration i = 2: max_weight is 3. Pop 1. peo[2]=1.
                                                         All neighbors numbered. buckets[3] is empty.
   0
                                                         Iteration i = 1: while decrements max_weight to 1. Pop 5.
                                                         peo[1]=5.
              2                       4                  Neighbors: {2, 4}. Node 2 is numbered.
                                                         Weight of 4 becomes 2. buckets[2] = [4]. max_weight = 2.

                                                         Iteration i = 0: max_weight is 2. Pop 4. peo[0]=4.
                                                         Neighbors: {2, 5} (All numbered).
                                      5
                                                    Final Array: peo = [4, 5, 1, 2, 0, 3, 6]
                                                    Validates as a Perfect Elimination Ordering.
                                                    MCS selection order: 6, 3, 0, 2, 1, 5, 4; the PEO is its reverse.

 Khem Singh (MidSemester Thesis Presentation)          MCS & Chordal Graphs                    September 18, 2026    13 / 21

## PDF page 14 of 21

Maximal Clique Extraction: Theoretical Basis


Cliques in Chordal Graphs:
     Given a valid Perfect Elimination Ordering (PEO), every maximal clique takes a specific,
     predictable structural form.

The Candidate Clique Formulation:
     For any arbitrary vertex u, let Madj(u) be the set of neighbors of u that appear later in the
     computed PEO.
     By the foundational property of the PEO, the set {u} ∪ Madj(u) strictly forms a completely
     connected subgraph (a candidate clique).
     The Core Challenge: While every maximal clique takes the form {u} ∪ Madj(u), not every such
     set is maximal. Certain candidate cliques are strictly subsumed by others.




  Khem Singh (MidSemester Thesis Presentation)   MCS & Chordal Graphs             September 18, 2026   14 / 21

## PDF page 15 of 21

The Subsumption Check (Maximality Formulation)


To algorithmically filter non-maximal cliques in strict O(V + E ) time, we establish a deterministic
parent-child subsumption rule.

     Parent Definition: For a vertex u with Madj(u) ̸= ∅, define its parent p(u) as the
     earliest-appearing vertex in Madj(u). If Madj(u) = ∅, set p(u) = −1.
     The Subsumption Theorem: The candidate clique of parent p is strictly subsumed by the
     candidate clique of u if and only if the exact size condition is met:

                                                 |Madj(u)| − 1 = |Madj(p)|

     Algorithmic Application: We initialize a boolean verification array is_maximal to true for all
     vertices. During iteration, if the subsumption condition holds for u and p, we update
     is_maximal[p] = false, explicitly eliminating it from the final extraction.



  Khem Singh (MidSemester Thesis Presentation)       MCS & Chordal Graphs         September 18, 2026   15 / 21

## PDF page 16 of 21

Formal Proof: Candidate Clique Structure


Let α be a valid PEO of a chordal graph G = (V , E ). For v ∈ V , let Madj(v ) be the neighbors of v
appearing after v in α. The candidate clique is Cv = {v } ∪ Madj(v ).
Lemma 1: Every maximal clique in G is of the form Cv for some v ∈ V .
     Let K be an arbitrary maximal clique in G.
     Since K is finite, exactly one vertex v ∈ K appears earliest in α.
     Since K is a clique, every other vertex in K is adjacent to v and appears after v in α. Thus,
     K \ {v } ⊆ Madj(v ).
     Adding v yields K ⊆ {v } ∪ Madj(v ) = Cv .
     By the definition of a PEO, Madj(v ) induces a complete subgraph, making Cv a clique.
     Because K ⊆ Cv and K is maximal, it cannot be a proper subset of any other clique. Therefore,
     K = Cv . ■



  Khem Singh (MidSemester Thesis Presentation)   MCS & Chordal Graphs            September 18, 2026    16 / 21

## PDF page 17 of 21

Formal Proof: Subsumption Condition (Part 1)

Theorem: Let p be the earliest-appearing vertex in Madj(u). The candidate clique Cp is strictly
subsumed by Cu (Cp ⊂ Cu ) if and only if |Madj(u)| − 1 = |Madj(p)|.
Proof (Subset Boundary & Forward Direction):
     Subset Boundary: p ∈ Madj(u). Since Madj(u) is a clique, every vertex in Madj(u) \ {p} is
     adjacent to p. Since p is the earliest, they all appear after p. Thus, they belong to Madj(p).

                                                 Madj(u) \ {p} ⊆ Madj(p)

     Forward Direction (⇒): Assume Cp ⊆ Cu .
     {p} ∪ Madj(p) ⊆ {u} ∪ Madj(u). Since u appears before p, u ∈
                                                                / Madj(p).
     Removing u and p reveals Madj(p) ⊆ Madj(u) \ {p}.
     Combined with our subset boundary, Madj(p) = Madj(u) \ {p}.
     Taking the cardinality yields |Madj(p)| = |Madj(u)| − 1.


  Khem Singh (MidSemester Thesis Presentation)      MCS & Chordal Graphs          September 18, 2026   17 / 21

## PDF page 18 of 21

Formal Proof: Subsumption Condition (Part 2)


Proof (Backward Direction & Conclusion):
     Backward Direction (⇐): Assume |Madj(u)| − 1 = |Madj(p)|.
     We established Madj(u) \ {p} ⊆ Madj(p). Since both sets have the same cardinality, they are
     identical:
                                      Madj(p) = Madj(u) \ {p}.
     Therefore,
                                         Cp = {p} ∪ Madj(p) = Madj(u) ⊊ {u} ∪ Madj(u) = Cu .
     Thus, Cp is not maximal whenever this condition holds for the child u.
     If the condition fails for this particular u, then Cp is not subsumed by Cu ; it may still be subsumed
     by another child of p, which is checked separately by the algorithm. ■




  Khem Singh (MidSemester Thesis Presentation)            MCS & Chordal Graphs            September 18, 2026   18 / 21

## PDF page 19 of 21

Linear-Time Extraction Algorithm

Algorithm 2 Optimized O(V + E ) Maximal Clique Extraction
 1: Compute PEO using Optimized MCS
 2: Initialize madj, madj_size, parent, and is_maximal structures
 3: for each u ∈ V do
 4:     for each neighbor ∈ adjacency _list[u] do
 5:          if position[neighbor ] > position[u] then
 6:              Append neighbor to madj[u]
 7:              Update parent[u] if neighbor appears earlier than the current tracked parent
 8:          end if
 9:     end for
10:      madj_size[u] ← |madj[u]|
11: end for
12: for each u ∈ V do                                                                                   ▷ Execute Subsumption Check
13:      p ← parent[u]
14:      if p ̸= −1 and madj_size[u] − 1 == madj_size[p] then
15:           is_maximal[p] ← false                                                             ▷ p’s candidate clique is absorbed by u
16:      end if
17: end for
18: Extract and yield all {u} ∪ madj[u] where is_maximal[u] evaluates to true.




   Khem Singh (MidSemester Thesis Presentation)               MCS & Chordal Graphs                      September 18, 2026          19 / 21

## PDF page 20 of 21

Thesis Roadmap and Current Progress



               Clique Graph WG : A graph whose vertices represent the maximal cliques of G.
                                                                  Currently here



                                                                        Clique     Clique
                                    MCS &       Maximal                                                    Dynamic
         Start                                                          Graph       Tree
                                     PEO        Cliques                                                   Algorithms
                                                                        (WG )       (T )



                                 Completed




 Khem Singh (MidSemester Thesis Presentation)        MCS & Chordal Graphs                   September 18, 2026         20 / 21

## PDF page 21 of 21

References

     Ibarra, L. (2008). Fully Dynamic Algorithms for Chordal Graphs and Split Graphs. ACM
     Transactions on Algorithms, 4(4), 40:1–40:20. DOI: 10.1145/1383369.1383371.
     Tarjan, R. E., & Yannakakis, M. (1984). Simple Linear-Time Algorithms to Test Chordality of
     Graphs, Test Acyclicity of Hypergraphs, and Selectively Reduce Acyclic Hypergraphs. SIAM Journal
     on Computing, 13(3), 566–579.
     Golumbic, M. C. (1980). Algorithmic Graph Theory and Perfect Graphs. Academic Press.
     Vandenberghe, L., & Andersen, M. S. (2015). Chordal Graphs and Semidefinite Optimization.
     Foundations and Trends in Optimization, 1(4), 241–433.
     Gavril, F. (1972). Algorithms for minimum coloring, maximum clique, minimum covering by cliques,
     and maximum independent set of a chordal graph. SIAM Journal on Computing, 1(2), 180–187.
     Blair, J. R. S., & Peyton, B. (1993). An introduction to chordal graphs and clique trees. In Graph
     Theory and Sparse Matrix Computation (pp. 1–29). Springer, New York.

  Khem Singh (MidSemester Thesis Presentation)   MCS & Chordal Graphs            September 18, 2026   21 / 21
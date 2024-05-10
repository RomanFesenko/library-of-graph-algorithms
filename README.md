1/17/2024

A library of the most widely used graph algorithms.

Algorithms: 

1.Linear-time algorithms:

    1.1 Breadth first search.    
    1.2 Depth first search.
    1.3 Search cycles.
    1.4 Is graph bipartite.
    1.5 Euler tours and cycles.
    1.6 Search articulation points and bridges.
    1.7 Topological sort.
    1.8 Tarjan algoritm for search strong connectivity components in digraph.
    1.9 Kosaraju algoritm for search strong connectivity components in digraph.	
	
2.Shortest paths algorithms:

    2.1 Variants of Dijkstra algoritm for sparse and dense graphs 
    	with O(E*ln(V) ) and O(V*V) complexity.
    2.2 Bellman-Ford algoritms for searching shortest paths or detection negative cycles.
    2.3 Floyd-Warshall algoritms for searching shortest paths or detection negative cycles.
    2.4 A* algorithm.
    2.5 Generalized versions of the priority-search for the algorithms above. 

3.Minimal spanning tree algorithms:

    3.1 Variants of Prim algoritm for sparse and dense graphs with O(E*ln(V) ) and O(V*V) complexity.
    3.2 Kruskal algorithm.
    
4.Maximal flow algorithms:

	4.1 Flow augmentation with edges shortest path with O(E*E*V) complexity.  
	4.2 Flow augmentation with maximal capacity for sparse and dense graphs with O(E*E*ln(V)*ln(C) ) and O(V*V*E*ln(C) ) complexity.   
	4.3 Dinic algoritm  with O(E*V*V) complexity. 
	4.4 FIFO preflow-push algorithm with O(V*V*V) complexity.   
	4.5 Highest label preflow-push algorithm with O(V*V*V) complexity.  
	4.6 Relabel-to-front preflow-push algorithm with O(V*V*V) complexity.
   
5.Minimal cost flow algorithms:

	5.1 Succesive shortest path algorithm.
	5.2 Cycle-canceling algorithm.	   

6.Matching algorithms:

	6.1 Maximal cardinality matching for arbitrary graph.
	6.2 Matching with maximal weights for arbitrary graph.
	6.3 Perfect matching with maximal weights for arbitrary graph.
	6.4 Edge covering with minimal weights for bipartite graph.
	6.5 Brutforce matching algorithms (for debug only).

Features:

1. The implementation of most algorithms is independent of graph types or weight/distance/cost types.
Any structure that has a suitable adjacency list interface can be used as a graph:
```C++
template<class graph_t>
concept graph_cpt = requires(graph_t g,typename graph_t::node_handler_t n,
                                       typename graph_t::edge_handler_t e)
{
    // node_handler_t- A lightweight reference object 
    // that represents a node in a graph, such as a pointer 
    // to a node or an index/iterator in a list of nodes
    typename graph_t::node_handler_t;

    // edge_handler_t- A lightweight reference object,
    // that represents edge of the graph,, such as a
    // pointer to edge or index/iterator in a list of edges.
    typename graph_t::edge_handler_t;

    // edge_begin(node_handler_t node)-return initial edge handler in
    // adjacency list of node.
    {g.edge_begin(n)}->std::same_as<typename graph_t::edge_handler_t>;

    // graph_t::edge_inc(node_handler_t n,edge_handler_t&e) -
    // incrementing edge e in adjacency list of n.
    {g.edge_inc(n,e)};

    // graph_t::edge_end(node_handler_t n,edge_handler_t e) -
    // return true if e is "end iterator" in adjacency list of n.
    {g.edge_end(n,e)}->std::same_as<bool>;

    // graph_t::target(node_handler_t n,edge_handler_t edge) -
    // return adjacent node with n by edge.
    {g.target(n,e)}->std::same_as<typename graph_t::node_handler_t>;
};
```

2.Breadth-search, depth-search, and priority search algorithms (e.g., Dijkstra's algorithm or Prim's algorithm) can be extended with "search adapters" that allow to perform some actions at key points in the algorithms:
```C++
template<class search_adapter_t,class node_t,class edge_t>
concept depth_search_adapter_cpt = requires(search_adapter_t sad,node_t n,edge_t e)
{
    // search_adapter_t::node_preprocess(node_t n)-
    // call, when node n is first time visited.
    // If return false, algorithm preterm terminate;
    {sad.node_preprocess(n)}->std::same_as<bool>;

    // search_adapter_t::edge_filter(node_t n,edge_t edge)-
    // if return false, this edge ignored in the search.
    {sad.edge_filter(n,e)}->std::same_as<bool>;

    // search_adapter_t::edge_process(node_t n,edge_t edge)-
    // if return false, algorithm preterm terminate;
    {sad.edge_process(n,e)}->std::same_as<bool>;

    // search_adapter_t::node_postprocess(node_t n)-
    // call, when node n is last time visited.
    // if return false, algorithm preterm terminate;
    {sad.node_postprocess(n)}->std::same_as<bool>;
};
```
3. Shortest path algorithms can be extended by using "weight updaters" 
that allow to search for a "generalized shortest path":
```C++
template<class upd_t,class weight_t>
concept weight_updater_cpt= requires(upd_t upd,weight_t w)
{
    // upd_t::priority(weight_t w1,weight_t w2)-
    // return true if w1 have highest priority then w2,
    // in edge relaxation process.
    // ==std::less<weight_t> for minimization problem or
    // ==std::greater<weight_t> for maximization problem.
    {upd.priority(w,w)}->std::same_as<bool>;

    // upd_t::weight_update(weight_t node_weight,weight_t edge_weight)-
    // return weight of adjacent node if it include in
    // optimal path.
    // ==std::plus<weight_t> for shortest path problem or
    // ==std::min()  for bottleneck problem or
    // ==edge_weight for minimal spanning tree problem.
    {upd.weight_update(w,w)}->std::same_as<weight_t>;

    // upd_t::init_weight - initial weight of start node.
    {upd.init_weight()}->std::same_as<weight_t>;
};
```

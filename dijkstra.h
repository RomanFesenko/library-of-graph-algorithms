
/** @file dijkstra.h
 *  Content:
 *  dense_dijkstra
 *  sparse_dijkstra
*/

#ifndef _liga_dijkstra_
#define _liga_dijkstra_

#include <vector>
#include <optional>
#include <algorithm>
#include <type_traits>

#include "search_settings.h"
#include "priority_search.h"

namespace liga{

/*
    Call list  for all __dijkstra functions:
    call_list:=graph_t node_t (search_adapter_t|node_t)? tree_search_t
*/

/**
 *  @brief  Find shortest paths from one node in dense graph, using Dijkstra algorithm.
 *  @param  graph       Graph,that satisfy w_graph_cpt concept.
 *  @param  source      First node of all shortest paths.
 *  @param  search      Search adapter,that satisfy priority_search_adapter_cpt concept.
 *  @param  tree        Tree of shortest paths from source node,that build algorithm.
 *  @param  weight_map  Weight map,which mapped edge to it weight.
 *  @return             Search termination cause.
 *
 * Complexity: O(V*V) in worst case.
*/

template<class graph_t,class search_t,class weight_t,class index_map_t,class weight_map_t>
search_termination_t dense_dijkstra(graph_t graph,
                                    typename graph_t::node_handler_t sourse,
                                    search_t&&search,
                                    tree_search_t<typename graph_t::node_handler_t,
                                                  typename graph_t::edge_handler_t,
                                                  weight_t,
                                                  index_map_t>&tree,
                                    weight_map_t weight_map)
{
    return dense_priority_search(graph,sourse,std::forward<search_t>(search),tree,weight_map,minimize_distance_t<weight_t>{});
}

/**
 *  @brief  Find one shortest path in dense graph, using Dijkstra algorithm.
 *  @param  g           Graph,that satisfy w_graph_cpt concept.
 *  @param  source      First node of  shortest path.
 *  @param  dest        Last node of  shortest path.
 *  @param  tree        Tree of shortest paths from source node,that build algorithm.
 *  @param  weight_map  Weight map,which mapped edge to it weight.
 *  @return             True if dest is reachable from source.
 *
 * Complexity: O(V*V) in worst case.
*/

template<class graph_t,class weight_t,class index_map_t,class weight_map_t>
bool dense_dijkstra(graph_t graph,
                    typename graph_t::node_handler_t sourse,
                    typename graph_t::node_handler_t dest,
                    tree_search_t<typename graph_t::node_handler_t,
                                  typename graph_t::edge_handler_t,
                                  weight_t,
                                  index_map_t>&tree,
                    weight_map_t weight_map)
{
    using node_t=graph_t::node_handler_t;
    index_map_t imap=tree.index_map();
    auto comp=[&imap](node_t n1,node_t n2)
    {
        return imap(n1)==imap(n2);
    };
    search_node_t<node_t,decltype(comp)> search_node(dest,comp);
    return dense_priority_search(graph,sourse,search_node,tree,weight_map,minimize_distance_t<weight_t>{})
           ==search_termination_t::node_preprocess_id;
}

/**
 *  @brief  Find all shortest paths from one node in dense graph, using Dijkstra algorithm.
 *  @param  g           Graph,that satisfy w_graph_cpt concept.
 *  @param  source      First node of all shortest paths.
 *  @param  tree        Tree of shortest paths from source node,that build algorithm.
 *  @param  weight_map  Weight map,which mapped edge to it weight.
 *  @return             Nothing.
 *
 * Complexity: O(V*V) in worst case.
*/

template<class graph_t,class weight_t,class index_map_t,class weight_map_t>
void dense_dijkstra(graph_t graph,
                    typename graph_t::node_handler_t sourse,
                    tree_search_t<typename graph_t::node_handler_t,
                                  typename graph_t::edge_handler_t,
                                  weight_t,
                                  index_map_t>&tree,
                    weight_map_t weight_map)
{
    dense_priority_search(graph,sourse,full_search_t{},tree,weight_map,minimize_distance_t<weight_t>{});
}



//Sparse cases



/**
 *  @brief  Find shortest paths from one node in sparse graph, using Dijkstra algorithm.
 *  @param  g           Graph,that satisfy w_graph_cpt concept.
 *  @param  source      First node of all shortest paths.
 *  @param  search      Search adapter,that satisfy priority_search_adapter_cpt concept.
 *  @param  tree        Tree of shortest paths from source node,that build algorithm.
 *  @param  weight_map  Weight map,which mapped edge to it weight.
 *  @return             Search termination cause.
 *
 * Complexity: O(E*ln(V)) in worst case.
*/

template<class graph_t,class search_t,class weight_t,class index_map_t,class weight_map_t>
search_termination_t sparse_dijkstra(graph_t graph,
                                     typename graph_t::node_handler_t sourse,
                                     search_t&&search,
                                     tree_search_t<typename graph_t::node_handler_t,
                                                   typename graph_t::edge_handler_t,
                                                   weight_t,
                                                   index_map_t>&tree,
                                     weight_map_t weight_map)
{
    return sparse_priority_search(graph,sourse,std::forward<search_t>(search),tree,weight_map,minimize_distance_t<weight_t>{});
}

/**
 *  @brief  Find one shortest path in sparse graph, using Dijkstra algorithm.
 *  @param  g           Graph,that satisfy w_graph_cpt concept.
 *  @param  source      First node of  shortest path.
 *  @param  dest        Last node of  shortest path.
 *  @param  tree        Tree of shortest paths from source node,that build algorithm.
 *  @param  weight_map  Weight map,which mapped edge to it weight.
 *  @return             True if dest is reachable from source.
 *
 * Complexity: O(E*ln(V)) in worst case.
*/

template<class graph_t,class weight_t,class index_map_t,class weight_map_t>
bool sparse_dijkstra(graph_t graph,
                     typename graph_t::node_handler_t sourse,
                     typename graph_t::node_handler_t dest,
                     tree_search_t<typename graph_t::node_handler_t,
                                   typename graph_t::edge_handler_t,
                                   weight_t,
                                   index_map_t>&tree,
                     weight_map_t weight_map)
{
    using node_t=graph_t::node_handler_t;
    index_map_t imap=tree.index_map();
    auto comp=[&imap](node_t n1,node_t n2)
    {
        return imap(n1)==imap(n2);
    };
    search_node_t<node_t,decltype(comp)> search_node(dest,comp);
    return sparse_priority_search(graph,sourse,search_node,tree,weight_map,minimize_distance_t<weight_t>{})
    ==search_termination_t::node_preprocess_id;
}

/**
 *  @brief  Find all shortest paths from one node in sparse graph, using Dijkstra algorithm.
 *  @param  g           Graph,that satisfy w_graph_cpt concept.
 *  @param  source      First node of all shortest paths.
 *  @param  tree        Tree of shortest paths from source node,that build algorithm.
 *  @param  weight_map  Weight map,which mapped edge to it weight.
 *  @return             Nothing.
 *
 * Complexity: O(E*ln(V)) in worst case.
*/

template<class graph_t,class weight_t,class index_map_t,class weight_map_t>
void sparse_dijkstra(graph_t graph,
                     typename graph_t::node_handler_t sourse,
                     tree_search_t<typename graph_t::node_handler_t,
                                   typename graph_t::edge_handler_t,
                                   weight_t,
                                   index_map_t>&tree,
                     weight_map_t weight_map)
{
    sparse_priority_search(graph,sourse,full_search_t{},tree,weight_map,minimize_distance_t<weight_t>{});
}

}// liga

#endif























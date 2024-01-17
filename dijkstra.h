
/** @file dijkstra.h
 *  Content:
 *  dense_dijkstra
 *  sparse_dijkstra
*/

#ifndef _dijkstra_
#define _dijkstra_

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
 *  @param  g           Graph,that satisfy w_graph_cpt concept.
 *  @param  source      First node of all shortest paths.
 *  @param  search      Search adapter,that satisfy priority_search_adapter_cpt concept.
 *  @param  tree        Tree of shortest paths from source node,that build algorithm.
 *  @return             Search termination cause.
 *
 * Complexity: O(V*V) in worst case.
*/

template<class graph_t,class search_t>
search_termination_t dense_dijkstra(const graph_t&g,
                     typename graph_traits<graph_t>::node_handler_t sourse,
                     search_t search,
                     tree_search_t<graph_t>&tree)
{
    return dense_priority_search(g,sourse,search,tree,minimize_distance_t<graph_t>{});
}

/**
 *  @brief  Find one shortest path in dense graph, using Dijkstra algorithm.
 *  @param  g           Graph,that satisfy w_graph_cpt concept.
 *  @param  source      First node of  shortest path.
 *  @param  dest        Last node of  shortest path.
 *  @param  tree        Tree of shortest paths from source node,that build algorithm.
 *  @return             True if dest is reachable from source.
 *
 * Complexity: O(V*V) in worst case.
*/

template<class graph_t>
bool dense_dijkstra(const graph_t&g,
                    typename graph_traits<graph_t>::node_handler_t sourse,
                    typename graph_traits<graph_t>::node_handler_t dest,
                    tree_search_t<graph_t>&tree)
{
    return dense_priority_search(g,sourse,search_node_t{g,dest},tree,minimize_distance_t<graph_t>{})
           ==search_termination_t::node_preprocess_id;
}

/**
 *  @brief  Find all shortest paths from one node in dense graph, using Dijkstra algorithm.
 *  @param  g           Graph,that satisfy w_graph_cpt concept.
 *  @param  source      First node of all shortest paths.
 *  @param  tree        Tree of shortest paths from source node,that build algorithm.
 *  @return             Nothing.
 *
 * Complexity: O(V*V) in worst case.
*/

template<class graph_t>
void dense_dijkstra(const graph_t&g,
                    typename graph_traits<graph_t>::node_handler_t sourse,
                    tree_search_t<graph_t>&tree)
{
    dense_priority_search(g,sourse,full_search_t<graph_t>{},tree,minimize_distance_t<graph_t>{});
}



//Sparse cases



/**
 *  @brief  Find shortest paths from one node in sparse graph, using Dijkstra algorithm.
 *  @param  g           Graph,that satisfy w_graph_cpt concept.
 *  @param  source      First node of all shortest paths.
 *  @param  search      Search adapter,that satisfy priority_search_adapter_cpt concept.
 *  @param  tree        Tree of shortest paths from source node,that build algorithm.
 *  @return             Search termination cause.
 *
 * Complexity: O(E*ln(V)) in worst case.
*/

template<class graph_t,class search_t>
search_termination_t sparse_dijkstra(const graph_t&g,
                     typename graph_traits<graph_t>::node_handler_t sourse,
                     search_t search,
                     tree_search_t<graph_t>&tree)
{
    return sparse_priority_search(g,sourse,search,tree,minimize_distance_t<graph_t>{});
}

/**
 *  @brief  Find one shortest path in sparse graph, using Dijkstra algorithm.
 *  @param  g           Graph,that satisfy w_graph_cpt concept.
 *  @param  source      First node of  shortest path.
 *  @param  dest        Last node of  shortest path.
 *  @param  tree        Tree of shortest paths from source node,that build algorithm.
 *  @return             True if dest is reachable from source.
 *
 * Complexity: O(E*ln(V)) in worst case.
*/

template<class graph_t>
bool sparse_dijkstra(const graph_t&g,
                    typename graph_traits<graph_t>::node_handler_t sourse,
                    typename graph_traits<graph_t>::node_handler_t dest,
                    tree_search_t<graph_t>&tree)
{
    return sparse_priority_search(g,sourse,search_node_t{g,dest},tree,minimize_distance_t<graph_t>{})
    ==search_termination_t::node_preprocess_id;
}

/**
 *  @brief  Find all shortest paths from one node in sparse graph, using Dijkstra algorithm.
 *  @param  g           Graph,that satisfy w_graph_cpt concept.
 *  @param  source      First node of all shortest paths.
 *  @param  tree        Tree of shortest paths from source node,that build algorithm.
 *  @return             Nothing.
 *
 * Complexity: O(E*ln(V)) in worst case.
*/

template<class graph_t>
void sparse_dijkstra(const graph_t&g,
                    typename graph_traits<graph_t>::node_handler_t sourse,
                    tree_search_t<graph_t>&tree)
{
    sparse_priority_search(g,sourse,full_search_t<graph_t>{},tree,minimize_distance_t<graph_t>{});
}

}// liga

#endif























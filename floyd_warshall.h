
/** @file floyd_warshall.h
 *  Content:
 *  floyd_warshall_shortest_path
 *  floyd_warshall_negative_cycle
*/

#ifndef _floyd_warshall_
#define _floyd_warshall_

#include <vector>
#include <optional>
#include <algorithm>
#include <type_traits>

#include "search_settings.h"
#include "tree_search.h"
#include "node_property.h"

namespace liga{

template<class graph_t,class efilter_t,class w_upd_t>
std::optional<typename graph_traits<graph_t>::node_handler_t>
__priority_floyd_warshall(const graph_t&gph,
                          const std::vector<typename graph_traits<graph_t>::node_handler_t>&nodes,
                          node_property_t<graph_t,tree_search_t<graph_t>>&trees,
                          efilter_t efilter,
                          w_upd_t upd)
{
    using gts=graph_traits<graph_t>;
    using weight_t=gts::weight_t;
    using node_t=gts::node_handler_t;
    using edge_t=gts::edge_handler_t;
    using tree_t=tree_search_t<graph_t>;

    trees.init(gph,{});
    for(node_t from:nodes)
    {
        tree_t&tree=trees.ref(from);
        tree.m_init(gph);
        for(node_t to:nodes)
        {
            tree.m_resize_for_node(to);
        }
    }

    for(node_t from:nodes)
    {
        tree_t&tree=trees.ref(from);
        tree.m_prev_node(from)=from;
        tree.m_dist(from)=upd.init_weight();
        tree.m_state(from)=node_state_t::closed_id;
        for(edge_t e=gts::edge_begin(gph,from);!gts::edge_end(gph,from,e);gts::edge_inc(gph,from,e))
        {
            if(!efilter(from,e)) continue;
            node_t to=gts::target(gph,from,e);
            tree.m_resize_for_node(to);
            tree.m_prev_node(to)=from;
            tree.m_prev_edge(to)=e;
            tree.m_dist(to)=gts::weight(gph,from,e);
            tree.m_state(to)=node_state_t::closed_id;
        }
    }

    // from -> median -> to
    for(node_t median:nodes)
    {
        tree_t&med_tree=trees.ref(median);
        for(node_t from:nodes)
        {
            tree_t&from_tree=trees.ref(from);
            if(!from_tree.in_tree(median)) continue;
            for(node_t to:nodes)
            {
                if(!med_tree.in_tree(to)) continue;
                if(!from_tree.in_tree(to))
                {
                    weight_t new_weight=upd.weight_update(from_tree.m_dist(median),med_tree.m_dist(to));
                    from_tree.m_prev_node(to)=med_tree.m_prev_node(to);
                    from_tree.m_prev_edge(to)=med_tree.m_prev_edge(to);
                    from_tree.m_dist(to)=new_weight;
                    from_tree.m_state(to)=node_state_t::closed_id;
                }
                else
                {
                    weight_t new_weight=upd.weight_update(from_tree.m_dist(median),med_tree.m_dist(to));
                    if(upd.priority(new_weight,from_tree.m_dist(to)))
                    {
                        from_tree.m_prev_node(to)=med_tree.m_prev_node(to);
                        from_tree.m_prev_edge(to)=med_tree.m_prev_edge(to);
                        from_tree.m_dist(to)=new_weight;
                    }
                }
            }
            if(!is_equal(gph,from,from_tree.m_prev_node(from))) return from;

        }
    }
    return {};
}

/**
 *  @brief  Find all-pairs generic shortest paths between using Floyd Warshall algorithm.
 *  @param  graph       Weighted graph,that satisfy w_graph_cpt concept.
 *  @param  nodes       All nodes of the graph.
 *  @param  trees       Trees, rooted in graph nodes.
 *  @param  efilter     Predicate- filter of edges.
 *  @param  updater     Weight updater,that satisfy weight_updater_cpt concept.
 *  @return             True if graph not content negative cycle.
*/

template<class graph_t,class efilter_t,class w_upd_t>
bool floyd_warshall_shortest_path(const graph_t&graph,
                                  const std::vector<typename graph_traits<graph_t>::node_handler_t>&nodes,
                                  node_property_t<graph_t,tree_search_t<graph_t>>&trees,
                                  efilter_t efilter,
                                  w_upd_t updater)
{
    return !__priority_floyd_warshall(graph,nodes,trees,efilter,updater);
}


template<class graph_t,class efilter_t=always_bool_t<true>>
bool floyd_warshall_shortest_path(const graph_t&graph,
                                  const std::vector<typename graph_traits<graph_t>::node_handler_t>&nodes,
                                  node_property_t<graph_t,tree_search_t<graph_t>>&trees,
                                  efilter_t efilter=always_bool_t<true>{})
{
    return floyd_warshall_shortest_path(graph,nodes,trees,efilter,minimize_distance_t<graph_t>{});
}

/**
 *  @brief  Find generic negative cycle using Floyd-Warshall algorithm.
 *  @param  graph        Weighted graph,that satisfy w_graph_cpt concept.
 *  @param  nodes        All nodes of the graph.
 *  @param  efilter      Predictor- filter of edges.
 *  @param  updater      Weight updater,that satisfy weight_updater_cpt concept.
 *  @return              Sequence of cycle edges if it exist.
*/


template<class graph_t,class efilter_t,class w_upd_t>
tree_search_t<graph_t>::path_t
floyd_warshall_negative_cycle(const graph_t&graph,
                              const std::vector<typename graph_traits<graph_t>::node_handler_t>&nodes,
                              node_property_t<graph_t,tree_search_t<graph_t>>&trees,
                              efilter_t efilter,
                              w_upd_t updater)
{
    using tree_t=tree_search_t<graph_t>;
    using path_t=tree_t::path_t;
    auto provoke_node=__priority_floyd_warshall(graph,nodes,trees,efilter,updater);
    return provoke_node? trees.ref(*provoke_node).find_cycle(*provoke_node):path_t{};
}

template<class graph_t,class efilter_t=always_bool_t<true>>
tree_search_t<graph_t>::path_t
floyd_warshall_negative_cycle(const graph_t&graph,
                              const std::vector<typename graph_traits<graph_t>::node_handler_t>&nodes,
                              node_property_t<graph_t,tree_search_t<graph_t>>&trees,
                              efilter_t efilter=always_bool_t<true>{})
{
    return floyd_warshall_negative_cycle(graph,nodes,trees,efilter,minimize_distance_t<graph_t>{});
}

}// liga

#endif
























/** @file floyd_warshall.h
 *  Content:
 *  floyd_warshall_shortest_path
 *  floyd_warshall_negative_cycle
*/

#ifndef _liga_floyd_warshall_
#define _liga_floyd_warshall_

#include <vector>
#include <optional>
#include <algorithm>
#include <type_traits>

#include "search_settings.h"
#include "tree_search.h"
#include "property_map.h"

namespace liga{

template<class graph_t,class cont_t,class weight_t,class index_map_t,class weight_map_t,class efilter_t,class w_upd_t>
std::optional<typename graph_t::node_handler_t>
__priority_floyd_warshall(graph_t graph,
                          const cont_t&nodes,
                          property_map_t<typename graph_t::node_handler_t,
                                         tree_search_t<typename graph_t::node_handler_t,
                                                       typename graph_t::edge_handler_t,
                                                       weight_t,
                                                       index_map_t>,
                                         index_map_t>&trees,
                          weight_map_t  weight_map,
                          efilter_t e_filter,
                          w_upd_t upd)
{
    using node_t=graph_t::node_handler_t;
    using edge_t=graph_t::edge_handler_t;
    using tree_t=tree_search_t<typename graph_t::node_handler_t,
                               typename graph_t::edge_handler_t,
                               weight_t,
                               index_map_t>;

    trees.clear();
    trees.default_value()=tree_t(trees.index_map());
    for(node_t from:nodes)
    {
        tree_t&tree=trees.ref(from);
        tree.drop();
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
        for(edge_t e=graph.edge_begin(from);!graph.edge_end(from,e);graph.edge_inc(from,e))
        {
            if(!e_filter(from,e)) continue;
            node_t to=graph.target(from,e);
            tree.m_resize_for_node(to);
            tree.m_prev_node(to)=from;
            tree.m_prev_edge(to)=e;
            tree.m_dist(to)=weight_map(from,e);
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
            if(!trees.is_equal_key(from,from_tree.m_prev_node(from))) return from;

        }
    }
    return {};
}

/**
 *  @brief  Find all-pairs generic shortest paths between using Floyd Warshall algorithm.
 *  @param  graph       Weighted graph,that satisfy w_graph_cpt concept.
 *  @param  nodes       All nodes of the graph.
 *  @param  trees       Trees, rooted in graph nodes.
 *  @param  weight_map  Weight map,which mapped edge to it weight.
 *  @param  e_filter    Predicate- filter of edges.
 *  @param  updater     Weight updater,that satisfy weight_updater_cpt concept.
 *  @return             True if graph not content negative cycle.
*/

template<class graph_t,class cont_t,class weight_t,class index_map_t,class weight_map_t,class efilter_t,class w_upd_t>
bool floyd_warshall_priority_search(graph_t graph,
                                    const cont_t&nodes,
                                    property_map_t<typename graph_t::node_handler_t,
                                                   tree_search_t<typename graph_t::node_handler_t,
                                                                 typename graph_t::edge_handler_t,
                                                                 weight_t,
                                                                 index_map_t>,
                                                   index_map_t>&trees,
                                    weight_map_t  weight_map,
                                    efilter_t e_filter,
                                    w_upd_t updater)
{
    return !__priority_floyd_warshall(graph,nodes,trees,weight_map,e_filter,updater);
}


template<class graph_t,class cont_t,class weight_t,class index_map_t,class weight_map_t,class efilter_t=always_bool_t<true>>
bool floyd_warshall_shortest_path(graph_t graph,
                                  const cont_t&nodes,
                                  property_map_t<typename graph_t::node_handler_t,
                                                 tree_search_t<typename graph_t::node_handler_t,
                                                               typename graph_t::edge_handler_t,
                                                               weight_t,
                                                               index_map_t>,
                                                 index_map_t>&trees,
                                  weight_map_t  weight_map,
                                  efilter_t e_filter=always_bool_t<true>{})
{
    return floyd_warshall_priority_search(graph,nodes,trees,weight_map,e_filter,minimize_distance_t<weight_t>{});
}

/**
 *  @brief  Find generic negative cycle using Floyd-Warshall algorithm.
 *  @param  graph        Weighted graph,that satisfy w_graph_cpt concept.
 *  @param  nodes        All nodes of the graph.
 *  @param  weight_map   Weight map,which mapped edge to it weight.
 *  @param  efilter      Predictor- filter of edges.
 *  @param  updater      Weight updater,that satisfy weight_updater_cpt concept.
 *  @return              Sequence of cycle edges if it exist.
*/

template<class graph_t,class cont_t,class weight_t,class index_map_t,class weight_map_t,class efilter_t,class upd_t>
std::vector<std::pair<typename graph_t::node_handler_t,
                      typename graph_t::edge_handler_t>>
floyd_warshall_negative_cycle(graph_t graph,
                              const cont_t&nodes,
                              property_map_t<typename graph_t::node_handler_t,
                                             tree_search_t<typename graph_t::node_handler_t,
                                                           typename graph_t::edge_handler_t,
                                                           weight_t,
                                                           index_map_t>,
                                             index_map_t>&trees,
                              weight_map_t  weight_map,
                              efilter_t e_filter,
                              upd_t upd)
{
    using path_t=std::vector<std::pair<typename graph_t::node_handler_t,
                                       typename graph_t::edge_handler_t>>;
    auto provoke_node=__priority_floyd_warshall(graph,nodes,trees,weight_map,e_filter,upd);
    return provoke_node? trees.ref(*provoke_node).find_cycle(graph,*provoke_node):path_t{};
}

template<class graph_t,class cont_t,class weight_t,class index_map_t,class weight_map_t,class efilter_t=always_bool_t<true>>
std::vector<std::pair<typename graph_t::node_handler_t,
                      typename graph_t::edge_handler_t>>
floyd_warshall_negative_cycle(graph_t graph,
                              const cont_t&nodes,
                              property_map_t<typename graph_t::node_handler_t,
                                             tree_search_t<typename graph_t::node_handler_t,
                                                           typename graph_t::edge_handler_t,
                                                           weight_t,
                                                           index_map_t>,
                                             index_map_t>&trees,
                              weight_map_t  weight_map,
                              efilter_t e_filter=always_bool_t<true>{})
{
    return floyd_warshall_negative_cycle(graph,nodes,trees,weight_map,e_filter,minimize_distance_t<weight_t>{});
}

}// liga

#endif






















